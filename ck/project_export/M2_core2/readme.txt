Readme - Custom NIOS II CPU architecture core 2

This core handles VGA display with emergency halt function implemented.
This core will clear the screen when KEY[0] is pressed and display an image
stored in shared SDRAM when KEY[1] is pressed. When the device is flipped, the
system will enter an emergency state, and an emergency stop image will be displayed.

All images used in this CPU are saved in the shared SDRAM and will be read from
the SDRAM when needed.