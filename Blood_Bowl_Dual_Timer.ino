#include <SoftwareSerial.h>
#include <elapsedMillis.h>
#include <LiquidCrystal.h>

elapsedMillis timeElapsedClock; //For Tracking Seconds Countdown
elapsedMillis timeElapsedTooLong; //For Tracking When Unit has been left on too long

const byte CLR = 12; const byte Line2 = 148; const byte Line1 = 128; const byte Note1 = 220; const byte Note2 = 222; const byte Note3 = 224; // Vars for Making lcd Commands Eaiser
int Timer1Mins; byte Timer1Secs; int Timer2Mins; byte Timer2Secs; //Vars for storing Time
byte Timer1Counting = 0; byte Timer1InAlarm = 0; byte Timer2InAlarm = 0; byte Timer2Counting = 0; byte OnTooLong = 0; //Vars for tracking running status
int Timer1Countdown = 0; int Timer2Countdown = 0; //Vars for Counting down
int Timer1AlarmTracking; int Timer2AlarmTracking;  //Vars for Alarms
int TooLongAlarm = 0;

const byte Timer1Button = 3; const byte Timer2Button = 4; const byte Timer1LED = 5; const byte Timer2LED = 6; //Setup Inputs & Outputs Pins
const int buzzer = 2;
const int AlarmDuration = 2; // Sets How long the Alarm goes on for (This is NOT Seconds or Milliseconds This number is program loop count)
const int Timer1Time = 240; const int Timer2Time = 240; //Seconds to countdown from for each timer
const long LeftOnTooLong = 600; //Seconds till alarm goes off for being turned on too long with no fresh inputs (Currently set for 30 seconds after last timer went off)
//SoftwareSerial lcd = SoftwareSerial(255, lcdPin); // Setup for lcd Communications
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);



void setup(void)
{
  Serial.begin(9600); // Start Serial Output for Diagnostics
  pinMode(buzzer, OUTPUT); //Set buzer - pin 2 as an output
  pinMode(Timer1Button, INPUT_PULLUP); pinMode(Timer2Button, INPUT_PULLUP); // Sets pins for Input with Pull Up Resistor Internally
  pinMode(Timer1LED, OUTPUT); pinMode(Timer2LED, OUTPUT); // Sets pins for Output
  lcd.begin(16, 2); // Start lcd Communications
  lcd.clear(); lcd.noBlink(); delay (100); // Sets system to on with no cursor, Turn backlight on, Clear
  Timer1Countdown = Timer1Time; Timer2Countdown = Timer2Time; // Sets The wanted countdown Secs into the vars that do the counting
  lcd.write(CLR);
}



void loop(void)
{
  ReadButtons(); // Go check inputs for button presses
  UpdateTime(); // Go update the countdowns if needed
  if (OnTooLong == 0) {
    Updatelcd(); //Show the Screen For Counting down if it has not been on too long
  }
  else {
    lcdTooLong(); //Display shutdown screen because unit was left on too long
  }
  TimerAlarms(); // Go see if either timer is in Alarm if so then run alarm
  TooLong(); // Go check to see if the unit has been on too long
}




void ReadButtons()
{
  if (digitalRead(Timer1Button) == LOW) {
    Timer1Countdown = Timer1Time;  // Timer 1 Start
    Timer1InAlarm = 0;
    Timer1Counting = 1;
    timeElapsedTooLong = 0;
    OnTooLong = 0;
  }
  if (digitalRead(Timer2Button) == LOW) {
    Timer2Countdown = Timer2Time;  // Timer 2 Start
    Timer2InAlarm = 0;
    Timer2Counting = 1;
    timeElapsedTooLong = 0;
    OnTooLong = 0;
  }
}

void Updatelcd()
{
  if (Timer1Secs < 10) {
    //lcd.write(Line1);  //If below 10 seconds then pad extra :0x
    lcd.setCursor(0,0);
    lcd.print ("Player1 = ");
    lcd.print (Timer1Mins);
    lcd.print(":0");
    lcd.print(Timer1Secs);
    lcd.print("   ");
  }
  else {
    //lcd.write(Line1);  // Print Time Left
    lcd.setCursor(0,0);    
    lcd.print ("Player1 = ");
    lcd.print (Timer1Mins);
    lcd.print(":");
    lcd.print(Timer1Secs);
    lcd.print("   ");
  }
  if (Timer2Secs < 10) {
    //lcd.write(Line2);  //If below 10 seconds then pad extra :0x
    lcd.setCursor(0,1);
    lcd.print ("Player2 = ");
    lcd.print (Timer2Mins);
    lcd.print(":0");
    lcd.print(Timer2Secs);
    lcd.print("   ");
  }
  else {
    //lcd.write(Line2);  // Print Time Left
    lcd.setCursor(0,1);
    lcd.print ("Player2 = ");
    lcd.print (Timer2Mins);
    lcd.print(":");
    lcd.print(Timer2Secs);
    lcd.print("   ");
  }
}

void UpdateTime()
{
  if (timeElapsedClock >= 1000) // If 1 second has passed since the last second counted then run code to update times andremove one second
  {
    if (Timer1Counting == 1) // If Timer 1 is running then run the code to subtract time
    {
      digitalWrite(Timer1LED, HIGH); Timer1Mins = floor(Timer1Countdown / 60); Timer1Secs = (Timer1Countdown - (Timer1Mins * 60)); // Compute how much time left vs how much passed
      Timer1Countdown --; // Subtract 1 Second from the clock Var
      if (Timer1Countdown < 0) {
        Timer1Counting = 0;  // If time is up then stop the timer and turn on the alarm
        Timer1InAlarm = 1;
        digitalWrite(Timer1LED, LOW);
      }
    }

    if (Timer2Counting == 1) // If timer 2 is running then run the code to subtract time
    {
      digitalWrite(Timer2LED, HIGH); Timer2Mins = floor(Timer2Countdown / 60); Timer2Secs = (Timer2Countdown - (Timer2Mins * 60)); // Compute how much time left vs how much passed
      Timer2Countdown --; // Subtract 1 Second from the clock Var
      if (Timer2Countdown < 0) {
        Timer2Counting = 0;  // If time is up then stop the timer and turn on the alarm
        Timer2InAlarm = 1;
        digitalWrite(Timer2LED, LOW);
      }
    }
    timeElapsedClock = 0; //1 second has passed now so reset the millisecond count again.
  }
}

void TimerAlarms ()
{
  if (Timer1InAlarm == 1)  //If timer 1 is in alarm then flash the light and play notes on lcd
  {
    if (timeElapsedClock <= 300) {
      digitalWrite(Timer1LED, LOW);
    }
    if (timeElapsedClock >= 700) {
      digitalWrite(Timer1LED, HIGH);
      Timer1AlarmTracking++;
      tone(buzzer, Note2);
      delay(1000);
      noTone(buzzer);
    }
    if (Timer1AlarmTracking >= AlarmDuration) {
      digitalWrite(Timer1LED, LOW);  //If alarm duration has been met then stop the alarm
      Timer1AlarmTracking = 0;
      Timer1InAlarm = 0;
    }
  }


  if (Timer2InAlarm == 1) //If timer 2 is in alarm then flash the light and play notes on lcd
  {
    if (timeElapsedClock <= 400) {
      digitalWrite(Timer2LED, LOW);
    }
    if (timeElapsedClock >= 800) {
      digitalWrite(Timer2LED, HIGH);
      Timer2AlarmTracking++;
      tone(buzzer, Note2);
      delay(1000);
      noTone(buzzer);
    }
    if (Timer2AlarmTracking >= AlarmDuration) {
      digitalWrite(Timer2LED, LOW);  //If alarm duration has been met then stop the alarm
      Timer2AlarmTracking = 0;
      Timer2InAlarm = 0;
    }
  }
}

void TooLong ()
{
  if (Timer1Counting == 1 || Timer2Counting == 1) {
    timeElapsedTooLong = 0; // If either timer is running then keep reseting the TooLong timer so it is not counting time when the time in use
    TooLongAlarm = 0;
  }
  if (timeElapsedTooLong > (LeftOnTooLong * 1000)) // If it's been too long since the last alarm or keypress then go into too long alarm
  {
    OnTooLong = 1;
    if (timeElapsedClock <= 200) {
      digitalWrite(Timer2LED, LOW);
      digitalWrite(Timer1LED, HIGH);
    }
    if (timeElapsedClock >= 800) {
      digitalWrite(Timer2LED, HIGH);
      digitalWrite(Timer1LED, LOW);
      if (TooLongAlarm <= 3) {
        tone(buzzer, Note3);
        delay(1000);
        noTone(buzzer);
        TooLongAlarm++;
      }
      //tone(buzzer, Note3);
      //delay(1000);
      //noTone(buzzer);
    }
  }
}

void lcdTooLong ()
{
  //lcd.write(Line1);  // Change Display to msg until turned off or or another timer is started
  lcd.setCursor(0,0);
  lcd.print(" You Forgot Me! ");
  //lcd.write(Line2);
  lcd.setCursor(0,1);
  lcd.print("  Turn Me Off! ");
}

