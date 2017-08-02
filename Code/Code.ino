

/*
    Basic Pin setup:
    ------------                                  ---u----
    Teensy    13|                           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|                           OUT3 |3     26|-> SIN (pin 11)
              10|                           OUT4 |4     25|-> SCLK (pin 13)
               9|                             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|-> SCLK (pin 25)             .  |7     22|-> GND
               6|-> SIN (pin 26)              .  |8     21|-> VCC (+5V)
               5|-> GSCLK (pin 18)            .  |9     20|-> 2K Resistor -> GND
               4|-> BLANK (pin 23)            .  |10    19|-> +5V (DCPRG)
               3|-> XLAT (pin 24)             .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.
*/

#include "Tlc5940.h"

#define SData 5
#define SLatch 6
#define SClock 8

void setup()
{

  pinMode(SData, OUTPUT);
  pinMode(SLatch, OUTPUT);
  pinMode(SClock, OUTPUT);

  pinMode(13, OUTPUT);
  for (int i = 0; i < 10; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
}

void loop()
{
  Tlc.clear();
  for (int tlcColum = 0; tlcColum < 16; tlcColum++) {
    Tlc.set(tlcColum, 1);
    Tlc.set(tlcColum + 16, 1);
    Tlc.set(tlcColum + 32, 1);
  }
  Tlc.update();

  for (uint16_t shiftRegisterRow = 1; shiftRegisterRow <= shiftRegisterRow << 16; shiftRegisterRow = shiftRegisterRow << 1) {
    digitalWrite(SLatch, LOW); //Latch Low
    ShiftOut(shiftRegisterRow >> 8);
    ShiftOut(shiftRegisterRow);
    digitalWrite(SLatch, HIGH); //Latch High
    delay(1000);
  }

}

void ShiftOut(uint8_t myDataOut) {
  // This shifts 8 bits out lngMsLoopstartB first,
  //on the rising edge of the clock,
  //clock idles low
  //internal function setup

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(SData, LOW);//Data aus
  digitalWrite(SClock, LOW);//Clock aus
  //for each bit in the uint8_t myDataOutï
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights.
  for (uint8_t i = 0; i <= 7; i++)  {
    digitalWrite(SClock, LOW);//Clock aus
    //if the value passed to myDataOut and a bitmask result
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1 << i) ) {
      digitalWrite(SData, HIGH);//Data an
    } else {
      digitalWrite(SData, LOW);//Data aus
    }
    //register shifts bits on upstroke of clock pin
    digitalWrite(SClock, HIGH);//Clock an
    digitalWrite(SClock, LOW);//Clock aus
    //zero the data pin after shift to prevent bleed through
    digitalWrite(SData, LOW);//Data aus
  }
}

