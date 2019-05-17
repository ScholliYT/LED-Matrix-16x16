//############################################################################################################################################################
//Externe Bibliotheken einbinden                                                                                                                             #
//############################################################################################################################################################

//############################################################################################################################################################
//Definitionen und Variablen                                                                                                                                 #
//############################################################################################################################################################


//Only for Debugging
#define DEBUG 1

#define TLC5940_N 1

// Setup pins
// GSCLK
#define GSCLK_DDR DDRB
#define GSCLK_PORT PORTB
#define GSCLK_PIN PB0

#define SIN_DDR DDRB
#define SIN_PORT PORTB
#define SIN_PIN PB3

#define SCLK_DDR DDRB
#define SCLK_PORT PORTB
#define SCLK_PIN PB5

#define BLANK_DDR DDRB
#define BLANK_PORT PORTB
#define BLANK_PIN PB2

#define DCPRG_DDR DDRD
#define DCPRG_PORT PORTD
#define DCPRG_PIN PD4

#define VPRG_DDR DDRD
#define VPRG_PORT PORTD
#define VPRG_PIN PD7

#define XLAT_DDR DDRB
#define XLAT_PORT PORTB
#define XLAT_PIN PB1



// define some BitBang macros
#define setOutput(DDR_Port, Pin) ((DDR_Port) |=   (1 << (Pin)))
#define setInput(DDR_Port, Pin)  ((DDR_Port) &=  ~(1 << (Pin)))
#define setHigh(Port, Pin)       ((Port)     |=   (1 << (Pin)))
#define setLow(Port, Pin)        ((Port)     &=  ~(1 << (Pin)))
#define toggle(Port, Pin)        ((Port)     ^=   (1 << (Pin)))
#define pulse(port, pin) do { \
    setHigh((port), (pin)); \
    setLow((port), (pin)); \
  } while (0)
#define outputState(port, pin) ((port) & (1 << (pin)))


//############################################################################################################################################################
// Daten                                                                                                                                                     #
//############################################################################################################################################################

// Dot correction data
uint8_t dcData[96 * TLC5940_N] = {
  // MSB            LSB
  1, 1, 1, 1, 1, 1,     // Channel 15
  1, 1, 1, 1, 1, 1,     // Channel 14
  1, 1, 1, 1, 1, 1,     // Channel 13
  1, 1, 1, 1, 1, 1,     // Channel 12
  1, 1, 1, 1, 1, 1,     // Channel 11
  1, 1, 1, 1, 1, 1,     // Channel 10
  1, 1, 1, 1, 1, 1,     // Channel 9
  1, 1, 1, 1, 1, 1,     // Channel 8
  1, 1, 1, 1, 1, 1,     // Channel 7
  1, 1, 1, 1, 1, 1,     // Channel 6
  1, 1, 1, 1, 1, 1,     // Channel 5
  1, 1, 1, 1, 1, 1,     // Channel 4
  1, 1, 1, 1, 1, 1,     // Channel 3
  1, 1, 1, 1, 1, 1,     // Channel 2
  1, 1, 1, 1, 1, 1,     // Channel 1
  1, 1, 1, 1, 1, 1,     // Channel 0
};

uint8_t gsData[192 * TLC5940_N] = {
  // MSB                              LSB
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // Channel 15
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // Channel 14
  1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // Channel 13
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,     // Channel 12
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,     // Channel 11
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,     // Channel 10
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,     // Channel 9
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,     // Channel 8
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,     // Channel 7
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,     // Channel 6
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,     // Channel 5
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,     // Channel 4
  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // Channel 3
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // Channel 2
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // Channel 1
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // Channel 0
};

//############################################################################################################################################################
//Setup - Prozedur                                                                                                                                           #
//############################################################################################################################################################

void setup() {
  Serial.begin(250000);
  Serial.println("TLC 5940 Bitbang implementation");
  setOutput(GSCLK_DDR, GSCLK_PIN);
  setOutput(SCLK_DDR, SCLK_PIN);
  setOutput(DCPRG_DDR, DCPRG_PIN);
  setOutput(VPRG_DDR, VPRG_PIN);
  setOutput(XLAT_DDR, XLAT_PIN);
  setOutput(BLANK_DDR, BLANK_PIN);
  setOutput(SIN_DDR, SIN_PIN);

  setLow(GSCLK_PORT, GSCLK_PIN);
  setLow(SCLK_PORT, SCLK_PIN);
  setLow(DCPRG_PORT, DCPRG_PIN);
  setHigh(VPRG_PORT, VPRG_PIN);
  setLow(XLAT_PORT, XLAT_PIN);
  setHigh(BLANK_PORT, BLANK_PIN);

  //Set brightness to initial value (DotCorrection values)
  set_dc_data();
}

//############################################################################################################################################################
//Hauptprogramm                                                                                                                                              #
//############################################################################################################################################################

void loop() {
  long start = millis();
  set_gs_and_gs_pwm();
  Serial.println(millis() - start);
}

//############################################################################################################################################################
// Shift out Data                                                                                                                                            #
//############################################################################################################################################################

void set_gs_and_gs_pwm()
{
  uint8_t firstCycleFlag = 0;

  if (outputState(VPRG_PORT, VPRG_PIN)) { // check if last thing we did was setting DC data
    setLow(VPRG_PORT, VPRG_PIN);
    firstCycleFlag = 1;
  }

  uint16_t GSCLK_Counter = 0;
  uint8_t Data_Counter = 0;

  setLow(BLANK_PORT, BLANK_PIN);
  for (;;) {
    if (GSCLK_Counter > 4095) {
      setHigh(BLANK_PORT, BLANK_PIN);
      pulse(XLAT_PORT, XLAT_PIN);
      if (firstCycleFlag) {
        pulse(SCLK_PORT, SCLK_PIN);
        firstCycleFlag = 0;
      }
      break;
    } else {
      if (!(Data_Counter > TLC5940_N * 192 - 1)) {
        if (gsData[Data_Counter])
          setHigh(SIN_PORT, SIN_PIN);
        else
          setLow(SIN_PORT, SIN_PIN);
        pulse(SCLK_PORT, SCLK_PIN);
        Data_Counter++;
      }
    }
    pulse(GSCLK_PORT, GSCLK_PIN);
    GSCLK_Counter++;
  }
}

//############################################################################################################################################################
// Set Global Brightness (DC/DotCorrection)                                                                                                                                 #
//############################################################################################################################################################

void set_dc_data() {
  setHigh(DCPRG_PORT, DCPRG_PIN);
  setHigh(VPRG_PORT, VPRG_PIN);

  uint8_t Counter = 0;

  for (;;) {
    if (Counter > TLC5940_N * 96 - 1) {
      pulse(XLAT_PORT, XLAT_PIN);
      break;
    } else {
      if (dcData[Counter])
        setHigh(SIN_PORT, SIN_PIN);
      else
        setLow(SIN_PORT, SIN_PIN);
      pulse(SCLK_PORT, SCLK_PIN);
      Counter++;
    }
  }
}

//############################################################################################################################################################
//############################################################################################################################################################
//############################################################################################################################################################

