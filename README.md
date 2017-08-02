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

## The building process


## DMX
Using ArtNet Node running on ESP8266. See [this awesome project](https://github.com/mtongnz/ESP8266_ArtNetNode_v2) by @mtongnz.
