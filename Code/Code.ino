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
#define DEBUG 1
#define DEBUGVALUES 0
#define DEBUGDMX 1
#define DEBUGMULTIPLEX 1
#define DEBUGREFRESHRATE 1
#if DEBUGREFRESHRATE
long lastLoop = 0;
#endif
// Matrix Size
#define MATRIX_ROWS 16
#define MATRIX_COLUMNS 16
// Shift register PINS
#define SData 23
#define SLatch 22
#define SClock 21
// DMX Data
uint8_t data[MATRIX_ROWS * MATRIX_COLUMNS * 3];
uint8_t dmxABuffer[510]; // 510 channels in channel A
uint8_t dmxBBuffer[258]; // remaining 258 channels in port B
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
  delay(250);
  Serial.begin(1000000);
  Serial.print("\n\n\n\n\n");
  Serial.println("Starting LED Matrix");
  Serial.printf("TLCs: %d\n", NUM_TLCS);
  Serial.printf("TLC_PWM_PERIOD: %d\n", TLC_PWM_PERIOD);
  Serial.printf("Dimension: %dx%d\n", MATRIX_ROWS, MATRIX_COLUMNS);

  pinMode(SData, OUTPUT);
  pinMode(SLatch, OUTPUT);
  pinMode(SClock, OUTPUT);
  pinMode(13, OUTPUT);

  dmxRxA.begin();
  dmxRxB.begin();

  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  /* Call Tlc.init() to setup the tlc.
  You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
}

void loop()
{
  #if DEBUGREFRESHRATE
  long time = micros() - lastLoop;
  Serial.printf("Refresh Time µs: %d. %dHz\n", time, 1000000 / time);
  lastLoop = micros();
  #endif

  #if DEBUGDMX
  long micGetDMX = micros();
  #endif

  getDMX();

  #if DEBUGDMX
  Serial.printf("Got new DMX Values. Time µs: %d\n", micros() - micGetDMX);
  #endif
  Tlc.setAll(0);
  for (uint8_t row = 0; row < MATRIX_ROWS; ++row)
  {
    #if DEBUGMULTIPLEX
    Serial.printf("Row: %d\n", row);
    
    long micTlc = micros();
    #endif
    for (uint8_t column = 0; column < MATRIX_COLUMNS; ++column)
    {

      #if DEBUGVALUES
      Serial.printf("[%d %d %d] ", data[row * 16 * 3 + column * 3 + 0], data[row * 16 * 3 + column * 3 + 1], data[row * 16 * 3 + column * 3 + 2]);
      #endif

      // Red
      Tlc.set(column                    , data[row * 48 + column * 3    ] * 16);
      // Green
      Tlc.set(column + MATRIX_COLUMNS   , data[row * 48 + column * 3 + 1] * 16);
       // Blue
      Tlc.set(column + MATRIX_COLUMNS *2, data[row * 48 + column * 3 + 2] * 16);

    }

    #if DEBUGMULTIPLEX
    Serial.printf("Set   Tlcs. Time µs: %d\n", micros() - micTlc);
    #endif

    #if DEBUGVALUES
    Serial.print("\n");
    #endif

    #if DEBUGMULTIPLEX
    long micOutput = micros();
    #endif
    clearMultiplex();    // --- clear Shift register in meanwhile ---- Data COULD already be latched
    Serial.printf("Clear multiplex. Time µs: %d\n", micros() - micOutput);
    while(Tlc.update()); //Wait to shift data
    Serial.printf("Tlc update. Time µs: %d\n", micros() - micOutput);
    //while(tlc_needXLAT); //Wait to latch data
      //Serial.printf("tlc need xlat. Time µs: %d\n", micros() - micOutput);
    multiplex(row);
    Serial.printf("Multiplex row. Time µs: %d\n", micros() - micOutput);
    #if DEBUGMULTIPLEX
    Serial.printf("Send  Data. Time µs: %d\n", micros() - micOutput);
    #endif

    delayMicroseconds(920); //Give LEDs some ontime - Best possible value: 900
  }
  Tlc.setAll(0);

  #if DEBUGVALUES
  Serial.print("\n\n\n");
  #endif

}


/*
 * Checks dmx Port A and for available data. If there is data available it will be copied to
 * the data array. 
 */
void getDMX()
{
  int readA = dmxRxA.readPacket(dmxABuffer, 1, 510);
  if (readA == 510)
  {

    #if DEBUG
    Serial.printf("DMX.A: %d\n", readA);
    #endif

    memcpy(data, dmxABuffer, sizeof(dmxABuffer));
  }
  else if (readA == -1)
  {
    #if DEBUG
      Serial.println("There is no DMX package available for port A");
    #endif
  }
  else
  {
    Serial.printf("Error reading all 510 required DMX Channels from DMX.A. Read: %d\n", readA);
  }


  int readB = dmxRxB.readPacket(dmxBBuffer, 1, 258);
  if (readB == 258)
  {
    #if DEBUG
      Serial.printf("DMX.B: %d\n", readB);
    #endif

    memcpy(data + sizeof(dmxABuffer), dmxBBuffer, sizeof(dmxBBuffer));
  }
  else if (readB == -1)
  {
    #if DEBUG
      Serial.println("There is no DMX package available for port B");
    #endif
  }
  else
  {
    Serial.printf("Error reading all 258 required DMX Channels from DMX.B. Read: %d\n", readB);
  }
}

/*
 * Activates the row by multiplexing to Shift registers, turning only one ouptut on.
 * row is a value between 0 and MATRIX_ROWS-1
 */
void multiplex(uint8_t row)
{
  digitalWriteFast(SLatch, LOW); // Latch Low
  uint16_t shift_data = 1 << row;
  // ============================= SHIFTOUT =============================
  for (uint8_t i = 0; i < MATRIX_ROWS; i++)
  {
    digitalWriteFast(SData, (shift_data & (1 << i))!=0?1:0);
    digitalWriteFast(SClock, HIGH);
    __asm__ __volatile__ ("nop\n\t");
    __asm__ __volatile__ ("nop\n\t");
    digitalWriteFast(SClock, LOW);
    digitalWriteFast(SData, LOW);
  }
  // ============================= SHIFTOUT =============================
  digitalWriteFast(SLatch, HIGH); // Latch High
}

/*
 * Sets all shift register outputs off.
 */
void clearMultiplex()
{
  digitalWriteFast(SLatch, LOW); // Latch Low
  // ============================= SHIFTOUT =============================
  for (uint8_t i = 0; i < MATRIX_ROWS; i++)
  {
    digitalWriteFast(SData, LOW);
    digitalWriteFast(SClock, HIGH);
    __asm__ __volatile__ ("nop\n\t");
    __asm__ __volatile__ ("nop\n\t");
    digitalWriteFast(SClock, LOW);
  }
  // ============================= SHIFTOUT =============================
  digitalWriteFast(SLatch, HIGH); // Latch High
}
