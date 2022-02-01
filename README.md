# HAGIWO 005 Eurorack Sequencer PCB and Code
<img src="images/005_front.jpg" width="25%" height="25%">

Through hole PCB version of the [HAGIWO 005 Generative Sequencer](https://www.youtube.com/watch?v=0eHllqHmlZQ) Eurorack module.
The module is a very simple Arduino-based CV and gate generator that gives random sequences similar to a music things modular turing machine.
Parameters can be set to change different parameters of the sequence.

## Hardware and PCB
<img src="images/005_side.jpg" width="25%" height="25%">

You can find the schematic and BOM in the root folder. For the PCBs, the module has one circuit PCB and one panel PCB. You can order them on any common PCB manufacturing service, I used JLCPCB. Standard settings should be fine.

I have uploaded the panel kicad and inkscape files if you want to change fonts etc. (I use the great [svg2shenzen](https://github.com/badgeek/svg2shenzhen) inkscape extension to do my panels.

you should probably make the led resistors (the 330 ohm ones) something higher like 1k, the leds on mine are very bright


# TODO:
- code cleanup (it works but could be prettier)

