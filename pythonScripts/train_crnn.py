"""
train_crnn_ctc_ocr.py

Train a CRNN/CTC OCR model from the synthetic scrolling-text dataset, then export
an inference-only TensorFlow Lite model.

Expected dataset format:
    CRNN_CTC_Scrolling_Text_192/
      train/
        img_000000.jpg
        labels.csv
      valid/
        img_000001.jpg
        labels.csv
      test/
        img_000002.jpg
        labels.csv

Each labels.csv:
    filename,text
    img_000000.jpg,ABC123 DEF456

Install:
    pip install tensorflow opencv-python numpy pandas

Run:
    python train_crnn_ctc_ocr.py

Then compile with Vela, for example:
    vela crnn_ctc_ocr_int8.tflite

Notes:
    - Training model uses CTC loss.
    - Exported TFLite model outputs per-timestep character probabilities.
    - Decode the output with greedy CTC decoding in your host/firmware code.
"""

from __future__ import annotations

import random
from pathlib import Path

import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import layers


# ============================================================
# CONFIG
# ============================================================

DATASET_DIR = Path("CRNN_CTC_Scrolling_Text_192")
OUTPUT_DIR = Path("crnn_ctc_outputs")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# Full camera scene input, matching your generator.
IMG_W = 192
IMG_H = 192
IMG_C = 1

# Uppercase alphanumeric plus space.
CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 "
BLANK_INDEX = len(CHARS)
NUM_CLASSES = len(CHARS) + 1

BATCH_SIZE = 32
EPOCHS = 80
LEARNING_RATE = 1e-3
SEED = 1234

# If your RAM/GPU is tiny, lower this.
AUTOTUNE = tf.data.AUTOTUNE

# Quantization. Use representative data for int8 export.
EXPORT_FLOAT_TFLITE = True
EXPORT_INT8_TFLITE = True
REPRESENTATIVE_SAMPLES = 200


# ============================================================
# REPRODUCIBILITY
# ============================================================

random.seed(SEED)
np.random.seed(SEED)
tf.random.set_seed(SEED)


# ============================================================
# CHARACTER ENCODING / DECODING
# ============================================================

char_to_num = {ch: i for i, ch in enumerate(CHARS)}
num_to_char = {i: ch for ch, i in char_to_num.items()}


def encode_text(text: str) -> list[int]:
    encoded: list[int] = []
    for ch in text:
        if ch not in char_to_num:
            raise ValueError(f"Unsupported character {ch!r} in label {text!r}")
        encoded.append(char_to_num[ch])
    return encoded


def greedy_ctc_decode_single(probabilities: np.ndarray) -> str:
    """Decode one prediction array shaped [time_steps, num_classes]."""
    ids = probabilities.argmax(axis=-1)
    result: list[str] = []
    prev = None

    for idx in ids:
        idx = int(idx)
        if idx != BLANK_INDEX and idx != prev:
            result.append(num_to_char[idx])
        prev = idx

    return "".join(result)


# ============================================================
# DATA LOADING
# ============================================================


def load_split(split: str) -> pd.DataFrame:
    csv_path = DATASET_DIR / split / "labels.csv"
    if not csv_path.exists():
        raise FileNotFoundError(f"Missing labels file: {csv_path}")

    df = pd.read_csv(csv_path)
    if "filename" not in df.columns or "text" not in df.columns:
        raise ValueError(f"{csv_path} must contain columns: filename,text")

    df["text"] = df["text"].astype(str)
    df["path"] = df["filename"].apply(lambda name: str(DATASET_DIR / split / name))

    # Remove empty labels. CTC can technically train with them, but they are not useful here.
    df = df[df["text"].str.len() > 0].reset_index(drop=True)

    # Validate characters.
    bad_rows: list[tuple[int, str]] = []
    for i, text in enumerate(df["text"].tolist()):
        for ch in text:
            if ch not in char_to_num:
                bad_rows.append((i, text))
                break
    if bad_rows:
        preview = bad_rows[:5]
        raise ValueError(
            f"Found unsupported characters in {split} labels. Preview: {preview}. "
            f"Allowed CHARS={CHARS!r}"
        )

    return df


train_df = load_split("train")
valid_df = load_split("valid")

MAX_LABEL_LEN = max(
    train_df["text"].str.len().max(),
    valid_df["text"].str.len().max(),
)

print(f"Train samples: {len(train_df)}")
print(f"Valid samples: {len(valid_df)}")
print(f"Max label length: {MAX_LABEL_LEN}")
print(f"Character classes including CTC blank: {NUM_CLASSES}")


# ============================================================
# TF.DATA PIPELINE
# ============================================================


def read_image(path: tf.Tensor) -> tf.Tensor:
    data = tf.io.read_file(path)
    image = tf.image.decode_jpeg(data, channels=1)
    image = tf.image.resize(image, [IMG_H, IMG_W])
    image = tf.cast(image, tf.float32) / 255.0
    return image


def encode_label_py(text: bytes) -> np.ndarray:
    decoded = text.decode("utf-8")
    ids = encode_text(decoded)
    padded = np.full((MAX_LABEL_LEN,), fill_value=-1, dtype=np.int32)
    padded[: len(ids)] = ids
    return padded


def make_example(path: tf.Tensor, text: tf.Tensor) -> dict[str, tf.Tensor]:
    image = read_image(path)

    label = tf.numpy_function(encode_label_py, [text], tf.int32)
    label.set_shape([MAX_LABEL_LEN])

    label_length = tf.strings.length(text)
    label_length = tf.cast(label_length, tf.int32)
    label_length = tf.expand_dims(label_length, axis=0)

    return {
        "image": image,
        "label": label,
        "label_length": label_length,
    }


def make_dataset(df: pd.DataFrame, training: bool) -> tf.data.Dataset:
    paths = df["path"].tolist()
    texts = df["text"].tolist()

    ds = tf.data.Dataset.from_tensor_slices((paths, texts))
    if training:
        ds = ds.shuffle(buffer_size=min(len(df), 4096), seed=SEED, reshuffle_each_iteration=True)

    ds = ds.map(make_example, num_parallel_calls=AUTOTUNE)
    ds = ds.batch(BATCH_SIZE, drop_remainder=False)
    ds = ds.prefetch(AUTOTUNE)
    return ds


train_ds = make_dataset(train_df, training=True)
valid_ds = make_dataset(valid_df, training=False)


# ============================================================
# MODEL
# ============================================================


def build_crnn_backbone() -> tf.keras.Model:
    """Inference model: image -> per-timestep character probabilities."""
    image = layers.Input(shape=(IMG_H, IMG_W, IMG_C), name="image")

    x = layers.Conv2D(32, 3, padding="same", activation="relu", name="conv1")(image)
    x = layers.MaxPooling2D(pool_size=(2, 2), name="pool1")(x)  # 96x96

    x = layers.Conv2D(64, 3, padding="same", activation="relu", name="conv2")(x)
    x = layers.MaxPooling2D(pool_size=(2, 2), name="pool2")(x)  # 48x48

    x = layers.Conv2D(96, 3, padding="same", activation="relu", name="conv3")(x)
    x = layers.MaxPooling2D(pool_size=(2, 1), name="pool3")(x)  # 24x48

    x = layers.Conv2D(128, 3, padding="same", activation="relu", name="conv4")(x)
    x = layers.MaxPooling2D(pool_size=(2, 1), name="pool4")(x)  # 12x48

    # Convert feature map into sequence along width.
    # Current shape: [batch, height=12, width=48, channels=128]
    x = layers.Permute((2, 1, 3), name="width_as_time")(x)
    x = layers.Reshape((IMG_W // 4, (IMG_H // 16) * 128), name="sequence_features")(x)

    # GRU is usually lighter than LSTM.
    x = layers.Bidirectional(layers.GRU(96, return_sequences=True), name="bigru1")(x)
    x = layers.Bidirectional(layers.GRU(96, return_sequences=True), name="bigru2")(x)

    probs = layers.Dense(NUM_CLASSES, activation="softmax", name="char_probs")(x)

    return tf.keras.Model(inputs=image, outputs=probs, name="crnn_ctc_inference")


class CTCLossLayer(layers.Layer):
    def call(self, inputs):
        y_pred, labels, label_length = inputs

        batch_size = tf.shape(y_pred)[0]
        time_steps = tf.shape(y_pred)[1]

        input_length = tf.fill([batch_size, 1], time_steps)
        loss = tf.keras.backend.ctc_batch_cost(labels, y_pred, input_length, label_length)
        self.add_loss(tf.reduce_mean(loss))
        return y_pred


def build_training_model(backbone: tf.keras.Model) -> tf.keras.Model:
    image = layers.Input(shape=(IMG_H, IMG_W, IMG_C), name="image")
    label = layers.Input(shape=(MAX_LABEL_LEN,), dtype="int32", name="label")
    label_length = layers.Input(shape=(1,), dtype="int32", name="label_length")

    y_pred = backbone(image)
    output = CTCLossLayer(name="ctc_loss")([y_pred, label, label_length])

    return tf.keras.Model(
        inputs={"image": image, "label": label, "label_length": label_length},
        outputs=output,
        name="crnn_ctc_training",
    )


backbone = build_crnn_backbone()
training_model = build_training_model(backbone)

training_model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE)
)

backbone.summary()


# ============================================================
# CALLBACKS
# ============================================================

checkpoint_path = OUTPUT_DIR / "best_crnn_ctc.weights.h5"
callbacks = [
    tf.keras.callbacks.ModelCheckpoint(
        filepath=str(checkpoint_path),
        monitor="val_loss",
        save_best_only=True,
        save_weights_only=True,
        verbose=1,
    ),
    tf.keras.callbacks.ReduceLROnPlateau(
        monitor="val_loss",
        factor=0.5,
        patience=5,
        min_lr=1e-5,
        verbose=1,
    ),
    tf.keras.callbacks.EarlyStopping(
        monitor="val_loss",
        patience=12,
        restore_best_weights=True,
        verbose=1,
    ),
]


# ============================================================
# TRAIN
# ============================================================

history = training_model.fit(
    train_ds,
    validation_data=valid_ds,
    epochs=EPOCHS,
    callbacks=callbacks,
)

if checkpoint_path.exists():
    training_model.load_weights(str(checkpoint_path))

# Save Keras inference model.
keras_path = OUTPUT_DIR / "crnn_ctc_inference.keras"
backbone.save(keras_path)
print(f"Saved Keras inference model: {keras_path}")


# ============================================================
# QUICK VALIDATION DECODE
# ============================================================

print("\nSample predictions:")
for batch in valid_ds.take(1):
    images = batch["image"]
    labels = batch["label"].numpy()
    preds = backbone.predict(images, verbose=0)

    for i in range(min(8, preds.shape[0])):
        pred_text = greedy_ctc_decode_single(preds[i])
        true_ids = [int(x) for x in labels[i] if int(x) >= 0]
        true_text = "".join(num_to_char[x] for x in true_ids)
        print(f"TRUE: {true_text!r} | PRED: {pred_text!r}")


# ============================================================
# TFLITE EXPORT
# ============================================================


def representative_dataset():
    count = 0
    for batch in train_ds:
        images = batch["image"]
        for i in range(images.shape[0]):
            yield [tf.expand_dims(images[i], axis=0)]
            count += 1
            if count >= REPRESENTATIVE_SAMPLES:
                return


if EXPORT_FLOAT_TFLITE:
    converter = tf.lite.TFLiteConverter.from_keras_model(backbone)
    float_tflite = converter.convert()
    float_path = OUTPUT_DIR / "crnn_ctc_ocr_float32.tflite"
    float_path.write_bytes(float_tflite)
    print(f"Saved float TFLite: {float_path}")

if EXPORT_INT8_TFLITE:
    converter = tf.lite.TFLiteConverter.from_keras_model(backbone)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset

    # Full integer quantization. This is normally what Vela wants.
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    try:
        int8_tflite = converter.convert()
        int8_path = OUTPUT_DIR / "crnn_ctc_ocr_int8.tflite"
        int8_path.write_bytes(int8_tflite)
        print(f"Saved int8 TFLite: {int8_path}")
    except Exception as exc:
        print("\nINT8 export failed.")
        print("This usually means one or more ops, often Bidirectional GRU, cannot be fully int8-converted.")
        print("Try float TFLite first, or replace GRU layers with a CNN-only architecture.")
        print(f"Error: {exc}")


# ============================================================
# SAVE DECODER INFO
# ============================================================

info_path = OUTPUT_DIR / "charset.txt"
info_path.write_text(
    f"CHARS={CHARS}\n"
    f"BLANK_INDEX={BLANK_INDEX}\n"
    f"NUM_CLASSES={NUM_CLASSES}\n"
    f"INPUT_SHAPE=1,{IMG_H},{IMG_W},{IMG_C}\n"
    f"OUTPUT=per-timestep softmax probabilities. Decode using greedy CTC.\n",
    encoding="utf-8",
)
print(f"Saved decoder info: {info_path}")

print("\nVela command example:")
print(f"    vela {OUTPUT_DIR / 'crnn_ctc_ocr_int8.tflite'}")

