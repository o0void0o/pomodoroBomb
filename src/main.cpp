#include <Arduino.h>

/*
    A
   ---
F |   | B
  | G |
   ---
E |   | C
  |   |
   ---
    D
 */

// Pin x-xx is connected to the 7 segments of the display.

long input = 20;
boolean isCountingDown = false;
boolean paused = false;
boolean armed = false;
boolean done = false;
boolean arming = false;
int btnPressed = 0;
long mode = -1; //-1= startup 1=25 2=5 3=15
long longPress = 0;
int pinA = 6;
int pinB = 7;
int pinC = 8;
int pinD = 9;
int pinE = 10;
int pinF = 11;
int buzzer = 12;
int pinH = 13;
int An0 = 0;
int D1 = 2;
int D2 = 3;
int D3 = 4;
int D4 = 5;
int timer1_counter;
int displaySegmants[4] = {D1, D2, D3, D4};
int dashArming = 0;

long MODE_0 = 24 * 60;
long MODE_1 = 10 * 60;
long MODE_2 = 30 * 60;

long modes[3] = {MODE_0, MODE_1, MODE_2};

void seqmentControl(int bitmap[])
{
  for (unsigned int i = 0; i < 4; i++)
  {
    if (bitmap[i] == 1)
    {
      digitalWrite(displaySegmants[i], LOW);
    }
    else
    {
      digitalWrite(displaySegmants[i], HIGH);
    }
  }
}

void displayDash(int bitmap[])
{
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, LOW);
  digitalWrite(pinC, LOW);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, LOW);
  digitalWrite(pinH, LOW);
  seqmentControl(bitmap);
}
void dashArm()
{
  arming = true;
  if (dashArming == 0)
  {
    displayDash(new int[4]{1, 0, 0, 0});
  }
  else if (dashArming == 1)
  {
    displayDash(new int[4]{1, 1, 0, 0});
  }
  else if (dashArming == 2)
  {
    displayDash(new int[4]{1, 1, 1, 0});
  }
  else if (dashArming == 3)
  {
    displayDash(new int[4]{1, 1, 1, 1});
  }
  dashArming++;
  delay(1000);
}
void reset()
{
  Serial.println("restting");
  done = false;
  armed = false;
  arming = false;
  dashArming = 0;
  input = 24 * 60;
  paused=false;
  isCountingDown=false;
  btnPressed=0;
  mode=-1;
}
void clearScreen()
{
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
}

void boom()
{
  digitalWrite(D1, LOW);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinH, HIGH);
  tone(buzzer, 200, 3000);
  reset();
}

void cycleModes()
{
  if(dashArming>0)
  {
    return;
  }
  mode++;
  if (mode >= 3)
  {
    mode = 0;
  }

  tone(buzzer, 1000, 100);
  input = modes[mode];
  Serial.println("input");
  Serial.println(input);
  Serial.println("mode");
  Serial.println(mode);
}
// the setup routine runs once when you press reset:
void setup()
{
  // initialize the digital pins as outputs.
  noInterrupts(); // disable all interrupts
  Serial.begin(9600);
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT);
  pinMode(pinF, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pinH, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(An0, INPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  noInterrupts(); // disable all interrupts

  //  TCCR0A = 0;// set entire TCCR2A register to 0
  //  TCCR0B = 0;// same for TCCR2B
  //  TCNT0  = 0;//initialize counter value to 0
  TCCR1A = 0;

  TCCR1B = 0;

  TCNT1 = 0;

  OCR1A = 31250; // compare match register 16MHz/256/2Hz

  TCCR1B |= (1 << WGM12); // CTC mode

  TCCR1B |= (1 << CS12); // 256 prescaler

  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  //  TCNT0  = 0;//initialize counter value to 0
  //  OCR0A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  //  TCCR0A |= (1 << WGM01);
  //  TCCR0B |= (1 << CS01) | (1 << CS00);
  //  TIMSK0 |= (1 << OCIE0A);

  interrupts(); // enable all interrupts
}
unsigned long iterateDigit(unsigned long kkk)
{
  unsigned long i = 1;
  while (kkk--)
  {
    i *= 10;
  }

  return i;
}

ISR(TIMER1_COMPA_vect) // interrupt service routine
{
  TCNT1 = 34286; // preload timer

  if (isCountingDown && armed)
  {
    Serial.println(isCountingDown);
    if (input == 0)
    {
      return;
    }
    input--;

    if (input < 10)
    {
      tone(buzzer, 1000, 100);
    }
  }
}

// the loop routine runs over and over again forever:
void loop()
{
  //  Serial.println(mode);
  if (mode == 9 && !paused)
  {
    isCountingDown = true;
    armed = true;
  }

  int anInput = analogRead(An0);

  if (anInput > 1020)
  {
    if (!dashArming)
    {
      clearScreen();
    }

    delay(100);
    anInput = analogRead(An0);
    btnPressed = 1;

    longPress += anInput;
    if (longPress > 5000 && (isCountingDown||paused))
    { 
      tone(buzzer, 1100, 100);
      delay(2000);
      reset();
      return;
    }



    if (longPress > 5000 && !isCountingDown && dashArming < 4)
    {
      Serial.println("poes1");
      dashArm();
    }
    else if (!isCountingDown && dashArming == 4)
    {
      Serial.println("poes2");

      mode = 9;
      armed = true;
      btnPressed = 0;
      tone(buzzer, 1100, 100);
      delay(500);
      tone(buzzer, 1100, 100);
      delay(2000);
      arming = false;
      dashArming = 0;
      Serial.println("righty");
      Serial.println(mode);
      return;
    }
  }

  else
  {

    longPress = 0;
    
    arming = 0;

    if (btnPressed == 1)
    {
      btnPressed = 0;

      if (done)
      {
        reset();
      }

      if (paused)
      {

        tone(buzzer, 1100, 100);
        delay(500);
        tone(buzzer, 1100, 100);
        isCountingDown = true;
        paused = false;
      }
      else if (isCountingDown)
      {

        isCountingDown = false;
        paused = true;
        tone(buzzer, 1100, 100);
      }
      else
      {
        cycleModes();
      }

      dashArming = 0;
      return;
    }
  }

  if (mode == -1)
  {

    displayDash(new int[4]{1, 1, 1, 1});
    return;
  }

  if (input == 0 && armed)
  {
    boom();
  }

  if (!arming && btnPressed == 0)
  {

    int hhh = 0;
    unsigned short digit = 1;

    if (input > 9)
    {
      digit = 2;
    }
    if (input > 99)
    {
      digit = 3;
    }
    if (input > 999)
    {
      digit = 4;
    }

    //  Serial.println(digit);

    while (digit--)
    {

      int koos = input / iterateDigit(digit) % 10;

      if (hhh == 0)
      {

        digitalWrite(D1, LOW);
        digitalWrite(D2, HIGH);
        digitalWrite(D3, HIGH);
        digitalWrite(D4, HIGH);
      }
      if (hhh == 1)
      {

        digitalWrite(D1, HIGH);
        digitalWrite(D2, LOW);
        digitalWrite(D3, HIGH);
        digitalWrite(D4, HIGH);
      }

      if (hhh == 2)
      {

        digitalWrite(D1, HIGH);
        digitalWrite(D2, HIGH);
        digitalWrite(D3, LOW);
        digitalWrite(D4, HIGH);
      }
      if (hhh == 3)
      {

        digitalWrite(D1, HIGH);
        digitalWrite(D2, HIGH);
        digitalWrite(D3, HIGH);
        digitalWrite(D4, LOW);
      }

      if (koos == 0)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, HIGH);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 1)
      {

        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, LOW);
      }
      else if (koos == 2)
      {

        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, LOW);
        digitalWrite(pinC, HIGH);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 3)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 4)
      {
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, LOW);
      }
      else if (koos == 5)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, LOW);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 6)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, HIGH);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, LOW);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 7)
      {
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, LOW);
        digitalWrite(pinE, LOW);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 8)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, HIGH);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      else if (koos == 9)
      {
        digitalWrite(pinA, HIGH);
        digitalWrite(pinB, HIGH);
        digitalWrite(pinC, LOW);
        digitalWrite(pinD, HIGH);
        digitalWrite(pinE, HIGH);
        digitalWrite(pinF, HIGH);

        digitalWrite(pinH, HIGH);
      }
      hhh++;
      delay(5);
    }
  }
}
