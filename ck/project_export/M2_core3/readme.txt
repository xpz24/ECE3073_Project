Readme - Custom NIOS II CPU architecture core 3

This core handles all SPI communication with the external microcontroller
board ESP32. The buttons are also apart of this core as it will be used
to control the external camera and decide when to capture the picture,
and when to display the image. When each key is pressed, it will notify
the other cores and raise a flag across the other 2 cores for their own
functionalities.

- KEY[0] will determine when to capture the external data
- KEY[1] will determine when to display the captured data or an image.