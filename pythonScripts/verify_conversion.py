from pathlib import Path

label_dir = Path(r".\Character_Detection_Swift-YOLO_192\dataset_yolo\labels\valid")

files = list(label_dir.glob("*.txt"))
non_empty = [p for p in files if p.read_text().strip()]

print("Label files:", len(files))
print("Non-empty label files:", len(non_empty))

for p in non_empty[:5]:
    print("\n", p)
    print(p.read_text()[:500])
    
root = Path(r".\Character_Detection_Swift-YOLO_192\dataset_yolo\labels")

bad = []

for txt in root.rglob("*.txt"):
    for line_no, line in enumerate(txt.read_text().splitlines(), 1):
        if not line.strip():
            continue

        parts = line.split()
        cls = int(parts[0])
        vals = list(map(float, parts[1:]))

        if len(vals) != 4 or any(v < 0 or v > 1 for v in vals):
            bad.append((txt, line_no, line))

print("Bad labels:", len(bad))
for item in bad[:20]:
    print(item)