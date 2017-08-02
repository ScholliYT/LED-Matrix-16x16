#define SData 23
#define SLatch 22
#define SClock 21

void setup() {
  pinMode(SData, OUTPUT);
  pinMode(SLatch, OUTPUT);
  pinMode(SClock, OUTPUT);
  digitalWrite(SData, LOW);
  digitalWrite(SLatch, LOW);
  digitalWrite(SClock, LOW);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {
  for (uint8_t row = 0; row < 16; ++row)
  {

    multiplex(row);
    delay(1);
}

	
}

void multiplex(uint8_t row)
{
  digitalWrite(SLatch, LOW); // Latch Low
  uint16_t value = 1 << row;
  ShiftOut(value);
  ShiftOut(value >> 8);
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
    delayMicroseconds(1);
    // register shifts bits on upstroke of clock pin
    digitalWrite(SClock, HIGH); // Clock an
    delayMicroseconds(1);
    digitalWrite(SClock, LOW); // Clock aus
    // zero the data pin after shift to prevent bleed through
    digitalWrite(SData, LOW); // Data aus
  }
}