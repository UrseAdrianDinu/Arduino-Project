#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "Wire.h"
#include "SparkFun_APDS9960.h"
#include "LiquidCrystal_I2C.h"
#define APDS9960_INT    2
#define BUTTON PB5
#define RED_PIN 11
#define GREEN_PIN 10
#define BLUE_PIN 9
// Use pins 6 and 7 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 7; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 6; // Connects to module's TX 
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
LiquidCrystal_I2C lcd(0x27,16,2); 

DFRobotDFPlayerMini player;
int sensorValue;

SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
int lastsong = 1;
int song = 1;
byte lastButtonState = LOW;
byte ledState = LOW;
long ts = 0;
short state = 0;
short print_flag = 0;
short first_play = 1;
long rgb_ts = 0;

void setup() {

  // Initializare interfata seriala
  Serial.begin(9600);
  // Initializare port serial pentru DFPlayer Mini
  softwareSerial.begin(9600);
  pinMode(APDS9960_INT, INPUT);
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - GestureTest"));
  Serial.println(F("--------------------------------"));
  
  // Adaug rutina de intrerupere pentru senzorul de gesturi
  attachInterrupt(0, interruptRoutine, FALLING);

  // Initializare senzorul de gesturi
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Pornire senzor de gesturi
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }

  // Intializare comunicare cu DFPlayer Mini
  if (player.begin(softwareSerial)) {
   Serial.println("OK");

    // Setare volum la 15/30
    player.volume(15);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }

  // Initializare LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();

  // Initializare buton
  DDRB &= ~(1 << BUTTON);
  PORTB |= (1 << BUTTON);

  // Initializare LED RGB
  DDRB |= (1 << PB1);
  DDRB |= (1 << PB2);
  DDRB |= (1 << PB3);
  
  
}
void interruptRoutine() {
  isr_flag = 1;
}
void print_song(int pos)
{
  lcd.clear();
  switch(pos)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Axwell|Ingrosso");
      lcd.setCursor(0, 1);
      lcd.print("MoreThanYouKnow");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Kungs");
      lcd.setCursor(0, 1);
      lcd.print("This Girl");
      break;
   case 3:
      lcd.setCursor(0, 0);
      lcd.print("Shouse");
      lcd.setCursor(0, 1);
      lcd.print("Love Tonight");
      break;
   case 4:
      lcd.setCursor(0, 0);
      lcd.print("The Chainsmokers");
      lcd.setCursor(0, 1);
      lcd.print("Closer");
      break;
   case 5:
      lcd.setCursor(0, 0);
      lcd.print("Vance Joy");
      lcd.setCursor(0, 1);
      lcd.print("Riptide");
      break;
    
  }
}

// Functie care citeste gesturile detectate de senzor
// Pentru UP, afisez NEXT pe ecran pentru 1 sec si 
// trec la urmatoarea melodie
// Pentru DOWN, afisez BACK pe ecran pentru 1 sec si
// revin la melodia anterioara
// Pentru LEFT, afisez PAUSE pe ecran si pun pauza
// Pentru RIGHT, afisez UNPAUSE pe ecran si reiau redarea
void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("NEXT");
        delay(1000);
        song++;
        if(song > 5)
          song = 1;
        lcd.clear();
        print_song(song);
        player.volume(30);
        player.play(song);
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("BACK");
        delay(1000);
        song--;
        if (song == 0)
          song = 5;
        lcd.clear();
        print_song(song);
        player.volume(30);
        player.play(song);
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("PAUSE");
        player.pause(); 
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("UNPAUSE");
        delay(1000);
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("MP3 Player ON");
        player.start(); 
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}

// Functie care seteaza culoarea LED-ULUI RGB
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(RED_PIN, red_light_value);
  analogWrite(GREEN_PIN, green_light_value);
  analogWrite(BLUE_PIN, blue_light_value);
}

void loop() {

  // Citesc butonul
  // Starea player-ului comuta la apasarea butonului
  if (millis() - ts > 300) {
    byte buttonState = (PINB & (1 << BUTTON));
    if (buttonState != lastButtonState) {
      ts = millis();
      lastButtonState = buttonState;
      if (buttonState == LOW) {
        if(state == 1)
        {
          state = 0;
          print_flag = 0;
        }
        else
        {
          state = 1;
          first_play = 1;
        }
        Serial.println(state);
      }
    }
  }

  // Player este pornit
  if(state)
  {
      // Daca a fost pornit pentru prima data,
      // afisez pe ecran "MP3 Player ON"
      // si dau play la prima melodie
      if(first_play)
      {
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("MP3 Player ON");
        delay(2000);
        print_song(song);
        player.play(song);
        RGB_color(255, 255, 255);
        first_play = 0;
        rgb_ts = millis();
        
      }
      // La fiecare 500ms, setez alta culoare pentru LED-UL RGB
      if (millis() - rgb_ts > 500) {
        int r = random(256);
        int g = random(256);
        int b = random(256);
        RGB_color(r, g, b);
        rgb_ts = millis();
        
      }
      // Citesc valoarea potentiometrului si setez volumul
      sensorValue = analogRead(A1);
      sensorValue = map(sensorValue,0,1023,0,30);
      player.volume(sensorValue);

      // Daca s-a detectat un gest, apelez handleGesture
      // pentru a procesa gestul facut
      if( isr_flag == 1 ) {
        sensorValue = analogRead(A1);
        sensorValue = map(sensorValue,0,1023,0,30);
        player.volume(sensorValue);
        detachInterrupt(0);
        handleGesture();
        isr_flag = 0;
        attachInterrupt(0, interruptRoutine, FALLING);
      }
  }
  else
  {
    // Player-ul a fost oprit
    // Afisez pe ecran "MP3 Player OFF"
    // si opresc player-ul
    if(!print_flag) {
      song = 1;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("MP3 Player OFF");
      print_flag = 1;
      RGB_color(0, 0, 0);
      player.stop();
    }
  } 
}
