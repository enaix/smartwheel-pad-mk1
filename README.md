# Smartwheel Pad Mk1

3d printed macropad with a tactile knob

![smartwheel-pad mk1](/extra/render1.jpg)

# Assembly

## Components

* Bearing 60012 12x28 mm
* EC12E rotary encoder, no button (d=6mm, l=15mm, f=7mm)
* MX style switch
* Replacement string for the switch (for the higher activation force)
* Aluminium shaft d=8mm (needs to be cut in 4 pieces ~4 cm)
* 4x M3 flat head screws
* 4x M3 d=5mm heat set inserts
* Any microcontroller (I use Wemos D1 mini type c)

### Used 3D models:
* https://github.com/ConstantinoSchillebeeckx/cherry-mx-switch

### CAD links

* Fusion 360 online editor: https://a360.co/3OT2Vcp
* Fusion 360 .f3z and .step models are in `src` folder

# Usage

## Firmware

Install Platformio cli/ide and run `pio run --target upload` in order to flash the firmware

## Software

Install [smartwheel-core](https://smartwheel-core.github.io), go to Input -> Serial and change the device port if needed
