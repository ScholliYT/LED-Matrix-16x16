/*
Basic Pin setup:
------------                                  ---u----
Teensy    13|-> Onbard LED    OUT1 |1     28| OUT channel 0
12|                           OUT2 |2     27|-> GND (VPRG)
11|                           OUT3 |3     26|-> SIN (pin 11)
10|                           OUT4 |4     25|-> SCLK (pin 13)
9|-> DMX.B                     .  |5     24|-> XLAT (pin 9)
8|                             .  |6     23|-> BLANK (pin 10)
7|-> SCLK (pin 25)             .  |7     22|-> GND
6|-> SIN (pin 26)              .  |8     21|-> VCC (+5V)
5|-> GSCLK (pin 18)            .  |9     20|-> 2K Resistor -> GND
4|-> BLANK (pin 23)            .  |10    19|-> +5V (DCPRG)
3|-> XLAT (pin 24)             .  |11    18|-> GSCLK (pin 3)
2|                             .  |12    17|-> SOUT
1|                             .  |13    16|-> XERR
0|-> DMX.A                   OUT14|14    15| OUT channel 15
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
// Imports
#include "Tlc5940.h"
#include "TeensyDMX.h"
// Debug
#define DEBUG 0
#define DEBUGVALUES 0
#define DEBUGDMX 0
#define DEBUGMULTIPLEX 0
// Matrix Size
#define MATRIX_ROWS 16
#define MATRIX_COLUMS 16
// Shift register PINS
#define SData 23
#define SLatch 22
#define SClock 21
// DMX Data
uint8_t data[MATRIX_ROWS * MATRIX_COLUMS * 3];
// DMX Connection
namespace teensydmx =::qindesign::teensydmx;
teensydmx::Receiver dmxRxA
{
  Serial1
};

teensydmx::Receiver dmxRxB
{
  Serial2
};

void setup()
{
  Serial.begin(250000);
  Serial.print("\n\n\n\n\n");
  Serial.println("Starting LED Matrix");
  Serial.printf("TLCs: %d\n", NUM_TLCS);
  Serial.printf("Dimension: %dx%d\n", MATRIX_ROWS, MATRIX_COLUMS);
  pinMode(SData, OUTPUT);
  pinMode(SLatch, OUTPUT);
  pinMode(SClock, OUTPUT);
  pinMode(13, OUTPUT);
  dmxRxA.begin();
  dmxRxB.begin();
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
    delay(50);
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

#if DEBUGDMX
  long micGetDMX = micros();
#endif

  getDMX();

#if DEBUGDMX
  Serial.printf("Got new DMX Values. Time µs: %d\n", micros() - micGetDMX);
#endif

  for (uint8_t row = 0; row < MATRIX_ROWS; ++row)
  {

#if DEBUGMULTIPLEX
    long micTlc = micros();
#endif

    for (uint8_t colum = 0; colum < MATRIX_COLUMS; ++colum)
    {

#if DEBUGVALUES
      Serial.printf("[%d %d %d] ", getVlaue(row, colum, 0), getVlaue(row, colum, 1), getVlaue(row, colum, 2));
#endif

      // Red
      Tlc.set(colum, getVlaue(row, colum, 0) * 16);
      // Green
      Tlc.set(colum + MATRIX_COLUMS, getVlaue(row, colum, 1) * 16);
      // Blue
      Tlc.set(colum + MATRIX_COLUMS * 2, getVlaue(row, colum, 2) * 16);
    }

#if DEBUGMULTIPLEX
    Serial.printf("Tlc Values for all colums. Time µs: %d\n", micros() - micTlc);
#endif

#if DEBUGVALUES
    Serial.print("\n");
#endif

#if DEBUGMULTIPLEX
    long micOutput = micros();
#endif

    multiplex(row);

#if DEBUGMULTIPLEX
    Serial.printf("Send Multiplex ShiftRegister. Time µs: %d\n", micros() - micOutput);
    micOutput = micros();
#endif

    Tlc.update();

#if DEBUGMULTIPLEX
    Serial.printf("Update Tlc. Time µs: %d\n", micros() - micOutput);
#endif

    delayMicroseconds(10);
    //delay(100);

    digitalWrite(SLatch, LOW); // Latch Low
    ShiftOut(0);
    ShiftOut(0);
    Tlc.clear();
    Tlc.update();
    delayMicroseconds(1);
    digitalWrite(SLatch, HIGH); // Latch High
  }

#if DEBUGVALUES
  Serial.print("\n\n\n");
#endif

}

// color: 0->RED, 1->GREEN, 2->BLUE
int getVlaue(uint8_t row, uint8_t colum, uint8_t color)
{
  return data[row * 16 * 3 + colum * 3 + color];
}

void getDMX()
{
  uint8_t tempA[510];
  int readA = dmxRxA.readPacket(tempA, 1, 510);
  if (readA == 510)
  {

#if DEBUG
    Serial.printf("DMX.A: %d\n", readA);
#endif

    memcpy(data, tempA, sizeof(tempA));
  }
  else
    if (readA == -1)
    {

#if DEBUG
      Serial.println("There is no DMX package available for port A");
#endif

    }
  else
  {
    Serial.printf("%s: %d\n", "Error reading all 510 required DMX Channels from DMX.A. Read: ", readA);
  }
  uint8_t tempB[258];
  int readB = dmxRxB.readPacket(tempB, 1, 258);
  if (readB == 258)
  {

#if DEBUG
    Serial.printf("DMX.B: %d\n", readB);
#endif

    memcpy(data + 510, tempB, sizeof(tempB));
  }
  else
    if (readB == -1)
    {

#if DEBUG
      Serial.println("There is no DMX package available for port B");
#endif

    }
  else
  {
    Serial.printf("%s: %d\n", "Error reading all 258 required DMX Channels from DMX.A. Read: ", readB);
  }
}

void multiplex(uint8_t row)
{
  digitalWrite(SLatch, LOW); // Latch Low
  uint16_t value = 1 << row;
  ShiftOut(value);
  ShiftOut(value >> 8);
  delayMicroseconds(1);
  digitalWrite(SLatch, HIGH); // Latch High
}

void ShiftOut(uint8_t myDataOut)
{
  // This shifts 8 bits out lngMsLoopstartB first,
  // on the rising edge of the clock,
  // clock idles low
  // internal function setup
  // clear everything out just in case to
  // prepare shift register for bit shifting
  digitalWrite(SData, LOW); // Data aus
  digitalWrite(SClock, LOW); // Clock aus
  // for each bit in the uint8_t myDataOutï
  // NOTICE THAT WE ARE COUNTING DOWN in our for loop
  // This means that %00000001 or "1" will go through such
  // that it will be pin Q0 that lights.
  for (uint8_t i = 0; i <= 7; i++)
  {
    digitalWrite(SClock, LOW); // Clock aus
    // if the value passed to myDataOut and a bitmask result
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000
    // and proceeds to set pinState to 1.
    if (myDataOut & (1 << i))
    {
      digitalWrite(SData, HIGH); // Data an
    }
    else
    {
      digitalWrite(SData, LOW); // Data aus
    }
    // register shifts bits on upstroke of clock pin
    digitalWrite(SClock, HIGH); // Clock an
    digitalWrite(SClock, LOW); // Clock aus
    // zero the data pin after shift to prevent bleed through
    digitalWrite(SData, LOW); // Data aus
  }
}
