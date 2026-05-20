import tkinter as tk
from collections import deque

FONT = ("Consolas", 72, "bold")
FRAME_DELAY_MS = 16


fullscreen = True
scroll_speed = 4

controls_hide_job = None
queue = deque()

current_text_id = None
current_text_width = 0
typed_buffer = ""

paused = False


def toggle_fullscreen():
    global fullscreen

    fullscreen = not fullscreen

    root.attributes("-fullscreen", fullscreen)

    if fullscreen:
        window_btn.config(text="Windowed")
        root.resizable(False, False)
    else:
        window_btn.config(text="Fullscreen")
        root.resizable(True, True)
        root.geometry("960x540")
        root.minsize(320, 180)


def hide_controls():
    controls.place_forget()
    root.config(cursor="none")


def update_scroll_speed(value):
    global scroll_speed
    scroll_speed = int(float(value))


def show_controls(event=None):
    global controls_hide_job

    controls.place(x=10, y=10)
    root.config(cursor="")

    if controls_hide_job is not None:
        root.after_cancel(controls_hide_job)

    controls_hide_job = root.after(2000, hide_controls)


def submit_text(text):
    global typed_buffer

    text = text.strip().upper()

    if text:
        queue.append(text)
        start_next_text()

    typed_buffer = ""


def start_next_text():
    global current_text_id

    if current_text_id is not None:
        return

    if not queue:
        return

    text = queue.popleft()

    current_text_id = canvas.create_text(
        canvas.winfo_width(),
        canvas.winfo_height() // 2,
        text=text,
        fill="white",
        font=FONT,
        anchor="w",
    )


def animate():
    global current_text_id

    if not paused:

        if current_text_id is not None:

            canvas.move(
                current_text_id,
                -scroll_speed,
                0,
            )

            bbox = canvas.bbox(current_text_id)

            if bbox and bbox[2] < 0:

                canvas.delete(current_text_id)

                current_text_id = None

                start_next_text()

    root.after(
        FRAME_DELAY_MS,
        animate,
    )


def toggle_pause():
    global paused

    paused = not paused

    pause_btn.config(text="Resume" if paused else "Pause")


def exit_program():
    root.destroy()


def on_key(event):

    global typed_buffer

    if event.keysym == "Escape":
        exit_program()
        return

    if event.keysym == "Return":

        submit_text(typed_buffer)

        return

    if event.keysym == "BackSpace":

        typed_buffer = typed_buffer[:-1]

        return

    if len(event.char) == 1:

        typed_buffer += event.char


root = tk.Tk()

root.title("Scrolling Text")

root.attributes("-fullscreen", fullscreen)
root.resizable(False, False)

root.configure(bg="black")

canvas = tk.Canvas(
    root,
    bg="black",
    highlightthickness=0,
    bd=0,
)

canvas.pack(
    fill="both",
    expand=True,
)

# Controls overlay
controls = tk.Frame(
    root,
    bg="#202020",
)

controls.place(
    x=10,
    y=10,
)

window_btn = tk.Button(
    controls,
    text="Windowed",
    command=toggle_fullscreen,
    font=("Arial", 12),
)

window_btn.pack(
    side="left",
    padx=4,
)

root.bind("<F11>", lambda event: toggle_fullscreen())

pause_btn = tk.Button(
    controls,
    text="Pause",
    command=toggle_pause,
    font=("Arial", 12),
)

pause_btn.pack(
    side="left",
    padx=4,
)

speed_label = tk.Label(
    controls,
    text="Speed",
    bg="#202020",
    fg="white",
    font=("Arial", 12),
)

speed_label.pack(
    side="left",
    padx=(8, 2),
)

speed_slider = tk.Scale(
    controls,
    from_=1,
    to=20,
    orient="horizontal",
    command=update_scroll_speed,
    bg="#202020",
    fg="white",
    highlightthickness=0,
    length=160,
)

speed_slider.set(scroll_speed)

speed_slider.pack(
    side="left",
    padx=4,
)

exit_btn = tk.Button(
    controls,
    text="Exit",
    command=exit_program,
    font=("Arial", 12),
)

exit_btn.pack(
    side="left",
    padx=4,
)

root.bind(
    "<Key>",
    on_key,
)

root.bind("<Motion>", show_controls)

show_controls()

root.after(1000, hide_controls)

# queue.append(
#     "HELLO WORLD."
# )

# start_next_text()

animate()

root.mainloop()
