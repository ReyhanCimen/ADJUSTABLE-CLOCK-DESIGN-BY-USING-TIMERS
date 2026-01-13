#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int buttonPin = 4;
const int led1Pin = 8;
const int led2Pin = 9;


volatile int seconds = 0;
volatile int minutes = 0;
volatile int hours = 0;

int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int pressCount = 0;
unsigned long firstPressTime = 0;
bool isCountingClicks = false;

unsigned long led1OffTime = 0;
unsigned long led2OffTime = 0;
bool led1Active = false;
bool led2Active = false;

void setup() {
  lcd.begin(16, 2);
  pinMode(buttonPin, INPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  Serial.begin(9600);

  noInterrupts();           
  TCCR1A = 0;               
  TCCR1B = 0;
  TCNT1  = 0;               

  
  OCR1A = 15624;            
  TCCR1B |= (1 << WGM12);   
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  TIMSK1 |= (1 << OCIE1A);  
  interrupts();             
}

ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      if (hours >= 24) {
        hours = 0;
      }
    }
  }
}

void loop() {
  updateLCD();

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      
      if (buttonState == HIGH) {
        if (pressCount == 0) {
          firstPressTime = millis(); 
          isCountingClicks = true;
        }
        pressCount++;
      }
    }
  }
  lastButtonState = reading;

  if (isCountingClicks && (millis() - firstPressTime >= 2000)) {
    handleButtonAction(pressCount); 
    pressCount = 0;
    isCountingClicks = false;
  }

  checkLeds();
}

void handleButtonAction(int count) {
  int serialValue = -1;
  
  if (Serial.available() > 0) {
    serialValue = Serial.parseInt();
    while(Serial.available()) Serial.read(); 
  }

  if (count == 1) {
   
    turnOnLed1(2000);
    turnOnLed2(2000);

    if (serialValue != -1) {
      hours = serialValue % 24; 
    }
  } 
  else if (count == 2) {
   
    turnOnLed1(4000);
    
    if (serialValue != -1) {
      
      minutes = serialValue % 60;
      seconds = 0; 
    }
  }
}


void updateLCD() {
  lcd.setCursor(0, 0);
  if (hours < 10) lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

void turnOnLed1(int duration) {
  digitalWrite(led1Pin, HIGH);
  led1OffTime = millis() + duration;
  led1Active = true;
}

void turnOnLed2(int duration) {
  digitalWrite(led2Pin, HIGH);
  led2OffTime = millis() + duration;
  led2Active = true;
}

void checkLeds() {
  unsigned long currentMillis = millis();

  if (led1Active && currentMillis >= led1OffTime) {
    digitalWrite(led1Pin, LOW);
    led1Active = false;
  }

  if (led2Active && currentMillis >= led2OffTime) {
    digitalWrite(led2Pin, LOW);
    led2Active = false;
  }
}