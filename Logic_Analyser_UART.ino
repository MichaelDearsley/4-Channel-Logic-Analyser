const int channel_1 = 2;
const int channel_2 = 3;
const int channel_3 = 4;
const int channel_4 = 5;
const int UART_tx = 8;
const int test_button = 6;

volatile byte buffer[1000]; //Capture buffer stores 1000 samples, volatile as accessed by ISR
volatile int sampleIndex = 0; 
volatile bool data_captured = false;

volatile byte uartData = 0; //Stores UART character currently being transmitted
volatile int uartBit = -1; //Keeps track of which UART bit, -1 = waiting,  0 = start bit, 1-8 = data bits, 9 = stop bit
volatile bool uartSending = false;
volatile int uartTicks = 0; //Counts timer1 interrupt (each interrupt is approximately 10us and each UART bit lasts 104us, hence 10 timer ticks needed)

void setup()
{
  Serial.begin(115200);

  pinMode(channel_1, INPUT_PULLUP);
  pinMode(channel_2, INPUT_PULLUP);
  pinMode(channel_3, INPUT_PULLUP);
  pinMode(channel_4, INPUT_PULLUP);

  pinMode(UART_tx, OUTPUT);
  digitalWrite(UART_tx, HIGH);
  pinMode(test_button, INPUT_PULLUP);
  noInterrupts(); //Disable interrupts while configuring timer1

  TCCR1A = 0; //Reset Timer 1 control register A
  TCCR1B = 0; //Reset Timer 1 control register B
  TCCR1B |= (1 << WGM12); //Set timer 1 to CTC mode (counts up till it gets to OCR1A value)
  OCR1A = 19; //Timer limit, Arduino clock is set to 16MHz with a prescaler of 8 is 16MHz / 8 = 2million timer counts / s /20 = 100k times per second which is sampling rate
  TCCR1B |= (1 << CS11); //Set prescaler to 8
  interrupts(); //Interrupts back on
}

void loop()
{
  if (digitalRead(test_button) == LOW) //Check if button has been pressed
  {
    sampleIndex = 0; //Reset capture
    data_captured = false;

    uartSending = false; //Reset UART
    uartBit = -1;
    uartTicks = 0;

    TIMSK1 |= (1 << OCIE1A); //Enable timer1 interrupt

    delayMicroseconds(100); //Give capture time to start

    sendMessage(); //Send UART transmission

    delay(500); 
  }

  if (data_captured) //Checks whether capture is finished
  {
    noInterrupts();

    for (int i = 0; i < 1000; i++)
    {
      Serial.println(buffer[i], BIN); //Send each sample to python as binary
    }

    data_captured = false;

    interrupts();
  }
}

void startUART(byte data) //Start UART transmission
{
  uartData = data; 
  uartBit = -1;
  uartTicks = 0;
  uartSending = true;
}

void sendMessage() //Sends UART message
{
  startUART('H');
  while (uartSending) {}

  startUART('E');
  while (uartSending) {}

  startUART('L');
  while (uartSending) {}

  startUART('L');
  while (uartSending) {}

  startUART('O');
  while (uartSending) {}

}

ISR(TIMER1_COMPA_vect) //Timer1 interrupt service routine, executes every 10us
{
  byte value = (PIND >> 2) & 0x0F; //Reads all four digital pins (D2-D5) and packs their states into the lower 4 bits

  if (sampleIndex < 1000) 
  {
    buffer[sampleIndex] = value; //Store sample
    sampleIndex++;
  }

  if (uartSending) //Only execute UART code if we're currently transmitting
  {
    uartTicks++;
    
    if (uartTicks >= 10) //After 10 timer ticks
    {
      uartTicks = 0; //Reset timer ticks

      uartBit++;

      if (uartBit == 0) //UART start bit
      {
        digitalWrite(UART_tx, LOW); //Start bit pulled low
      }

      else if (uartBit >= 1 && uartBit <= 8) //Data bits
      {
        if (uartData & (1 << (uartBit - 1))) //Checks each bit
        {
          digitalWrite(UART_tx, HIGH); //If bit 1 pulled HIGH
        }
        else
        {
          digitalWrite(UART_tx, LOW); //If bit 0 pulled LOW
        }
      }

      else //Stop bit
      {
        digitalWrite(UART_tx, HIGH);
        uartSending = false;
      }
    }
  }

  if (sampleIndex >= 1000) //When the captures complete
  {
    data_captured = true;

    TIMSK1 &= ~(1 << OCIE1A); //Turn off Timer 1 interrupt

    digitalWrite(UART_tx, HIGH); //UART returns to IDLE
    uartSending = false;
  }
}
