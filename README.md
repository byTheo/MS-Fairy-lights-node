# MS-Fairy-lights-node
A MySensors node for controlling Fairy Light strings. For making the mounting of the node into a light fixture easier, I've created a small PCB which supports NRF24L01+ radios only. I've kept the footprint of the PCB as small as I could, but I used through hole components in those places where the values depend on the type of fairy light you use. I used a 3 * AA battery version which outputs 5v to the fairy light string. In my case there was an 18ohm resistor between the chip that controls the fairy lights and the positive side of the fairy lights string itself.

# Hardware
It is of course not necessary to order a PCB but it makes life much easier than soldering it to a prototype board manually. This version supports up to 3 fairy lights - I have a seasonable version in my mind - and you can connect a tactile switch and rotary encoder to it for manual operation. I think that being able to manually operate smart lightning is a design must. You phone battery can be empty or your HA system might be malfunctioning, so in those cases you need to be able to manually operate it.

![alt text](hardware/PCB%202d%20photo.png)

The mounting point for the NRF24L01+ radio is set inwards, this is so that an angles female header can me mounter. I needed the radio to be mounted angled - but you will see that in a picture.

The Gerber files can be found in the hardware directory and you can order 5 pcb's for around 15 euro's including shipping and tax.
