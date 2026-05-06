Readme - Custom NIOS II CPU architecture core 1

This core handles all user interactable inputs/outputs, except for the
buttons. This core also makes use of the on board accelerometer to
detect tilt, and assert emergency state to the whole device and the other
2 cores. Main features of this core includes the HEX displays which will
be controlled through the switches. 

- SW[0] acts as the master switch which determines if the HEX display is on or off. 
- SW[1] enables scrolling text and has the ability to pause the scrolling text.
- SW[2] displays core 1 CPU usage when flipped on.
- SW[3] turns the buzzer on according to the frequency of the scrolling text.
        It also enables a custom lighting effect on the traffic led.

When the accelerometer detects the board being flipped over, it will raise an emergency
flag across all 3 cores and halt all operations. It will also display the text "stop"
onto the HEX displays. The on board red leds will mirror the switches to allow for better
visualisation of which switches are turned on.