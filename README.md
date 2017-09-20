# LED-Matrix-16x16
## A 16x16 RGB LED Matrix controlled by DMX over wifi

__Some facts:__
- 256 RGB LEDs
- 768 DMX Channels
- Teensy 3.6 as Main Controller
- ESP8266 as ArtNet Node
- 3 TLC5940s for controlling brightness
- 2 74CH595 Shift Registers for multiplexing

## History
So this is the second LED Matrix we are building. The first one was a bit smaller (10x10) and used shift registers only for controlling the LEDs. We weren't that happy about the limitation we got using shift registers only. We ran into refresh rate problems while trying to increase possible grayscale values. We ended up with 8bit grayscale (0-255).

![old Matrix](docs/images/oldMatrix.png?raw=true "The old 10x10 Matrix")

## The plan
##### So the basic data flow should look like this
![dataflow](docs/images/function.png?raw=true "function - Data flow")

## Schematic
![MainCircuitBoard](Schematics/Hauptplatine.png?raw=true "Circuit Board")

## The building process

![main frame](docs/images/frame.png?raw=true "The main frame")
![Backplate](docs/images/Back.png?raw=true "The main backplate")

##### So after some weekends of soldering 4 wires to every single SMD LED we finally made it.

![First LED](docs/images/firstled.png?raw=true "The first finished LED")
![Soldering LEDs](docs/images/working.png?raw=true "The process of soldering all wires to 256 LEDs")

##### Lets start soldering them on the frame...

![First](docs/images/firstmountedled.png?raw=true "The first LED is mounted")
![Process](docs/images/process.png?raw=true "We're making progress")

##### After dozen weekends spend soldering we made it

![Finished](docs/images/finished.png?raw=true "Finally we made it")

## First testing
##### As you can see. It works! :) But there are some LEDs that need to be fixed

![working](docs/images/working.gif?raw=true "It works")

## DMX
Using ArtNet Node running on ESP8266. See [this awesome project](https://github.com/mtongnz/ESP8266_ArtNetNode_v2).

