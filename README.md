# MS-Fairy-lights-node
A MySensors node for controlling Fairy Light strings. For making the mounting of the node into a light fixture easier, I've created a small PCB which supports NRF24L01+ radios only. I've kept the footprint of the PCB as small as I could, but I used through hole components in those places where the values depend on the type of fairy light you use. I used a 3 * AA battery version which outputs 5v to the fairy light string. In my case there was an 18ohm resistor between the chip that controls the fairy lights and the positive side of the fairy lights string itself.

# Hardware
It is of course not necessary to order a PCB but it makes life much easier than soldering it to a prototype board manually. This version supports up to 3 fairy lights - I have a seasonable version in my mind - and you can connect a tactile switch and rotary encoder to it for manual operation. I think that being able to manually operate smart lightning is a design must. Your phone battery can be empty or your HA system might be malfunctioning, so in those cases you need to be able to manually operate it.

![alt text](PCB%202d%20photo.png)

The mounting point for the NRF24L01+ radio is set inwards, this is so that an anglds female header (2*5p 2.54mm) can be mounted. I needed the radio to be mounted angled - but you will see that in a picture.

The Gerber files can be found in the hardware directory and you can order 5 pcb's for around 15 euro's including shipping and tax.

The parts you need are:
- 1 x th 100uf capacitor for stabalising the NRF24L01+
- 1 x NRF24L01+ radio (for communicating over the MySensors network)
- 1 proMini 3.3v - they can be order cheaply on aliExpress or Ebat. I used the 3.3v version so that I didn't need to mount a separate power regulater for the radio on the PCB.

Depending on your FairyLight string, you need the following parts for each string:
- 1 2N7000 th - this mosfet can drive 200mA. My fairylights consume about 40mA at max brightness so more than sufficient
- 18 ohm th resistor - this is for current limiting the fairy light string
- 1K pull down resistor (TH) - for pulling down the 2n7000's gate

Optional:
- either one tactile switch if you don't want to be able to manually dim the lights
- or a rotary encoder with tactile switch encorperated.

# Software
All the embedded software I created for this board is located in the embedded software directory of this repository. 
