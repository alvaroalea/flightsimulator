// Bendix King KAP-140 Simulator for SIOC
// (C) 2016 Alvaro Alea Fernandez <alvaroaleaATgmail.com>
// Licensed under GPL 2.0
// Arduino Micro + LCD 24x2 I2C + Encoder + buttons (10+encoders) + 3 led.

//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// ENCODER
#define encoder0PinA  2  // encoder pin on interrupt 0 - pin 2
#define encoder0PinB  4
volatile unsigned int encoder0Pos = 0;
void doEncoder(void) {
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
    encoder0Pos++;
  } else {
    encoder0Pos--;
  }
}


// LCD
LiquidCrystal_I2C lcd(0x3f,24,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
// AP
byte ch0[8] = {
  0b11111,
  0b10000,
  0b10010,
  0b10101,
  0b10111,
  0b10101,
  0b10000,
  0b11111
};
byte ch1[8] = {
  0b11111,
  0b00001,
  0b11101,
  0b10101,
  0b11101,
  0b10001,
  0b00001,
  0b11111
};
// YD
byte ch2[8] = {
  0b11111,
  0b00000,
  0b00101,
  0b00111,
  0b00010,
  0b00010,
  0b00000,
  0b11111
};
byte ch3[8] = {
  0b11111,
  0b00000,
  0b11000,
  0b10100,
  0b10100,
  0b11000,
  0b00000,
  0b11111
};
// ARM
byte ch4[8] = {
  0b00000,
  0b00000,
  0b01100,
  0b10010,
  0b11110,
  0b10010,
  0b00000,
  0b11100
};
byte ch5[8] = {
  0b10010,
  0b10100,
  0b10010,
  0b00000,
  0b11011,
  0b10101,
  0b10001,
  0b10001
};
// Las 4 versiones de PT y las flechas
byte ch61[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b01100,
  0b01010,
  0b01100,
  0b01000
};
byte ch71[8] = {
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte ch62[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01100,
  0b01010,
  0b01100,
  0b01000
};
byte ch72[8] = {
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b11111,
  0b01110,
  0b00100
};
byte ch63[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b01100,
  0b01010,
  0b01100,
  0b01000
};
byte ch73[8] = {
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b11111,
  0b01110,
  0b00100
};
byte ch64[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01100,
  0b01010,
  0b01100,
  0b01000
};
byte ch74[8] = {
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void arm1(int s){
  // ARM1
  lcd.setCursor(4,0);
  if (s!=0){
    lcd.write(byte(4));
    lcd.setCursor(4,1);
    lcd.write(byte(5));
  }else{
    lcd.print(" ");
    lcd.setCursor(4,1);
    lcd.print(" ");
  }
}
void arm2(int s){
  // ARM2
  lcd.setCursor(13,0);
  if (s!=0){
    lcd.write(byte(4));
    lcd.setCursor(13,1);
    lcd.write(byte(5));
  }else{
    lcd.print(" ");
    lcd.setCursor(13,1);
    lcd.print(" ");
  }
}

void ap(int s){
  // AP
  lcd.setCursor(5,0);
  if (s!=0){
    lcd.write(byte(0));
    lcd.write(byte(1));
  }else{
    lcd.print("  ");
  }
}
 
void yd(int s){
  // YD
  lcd.setCursor(5,1);
  if (s!=0){
    lcd.write(byte(2));
    lcd.write(byte(3));
  }else{
    lcd.print("  ");
  }
}

void pt(int s){
  // PT
  switch (s){
    case 1:
     lcd.createChar(6,ch61);
     lcd.createChar(7,ch71);
     break;
    case 2:
     lcd.createChar(6,ch62);
     lcd.createChar(7,ch72);
     break;
    case 3:
     lcd.createChar(6,ch63);
     lcd.createChar(7,ch73);
     break;
    case 4:
     lcd.createChar(6,ch64);
     lcd.createChar(7,ch74);
     break;
  }
  lcd.setCursor(14,0);
  if (s!=0){
    lcd.write(byte(6));
    lcd.setCursor(14,1);
    lcd.write(byte(7));
  }else{
    lcd.print(" ");
    lcd.setCursor(14,1);
    lcd.print(" ");
  }
}

void alert(int s){
  // AP
  lcd.setCursor(15,1);
  if (s!=0){
    lcd.print("ALRT");
  }else{
    lcd.print("    ");
  }
}

void mode(int s){
  // PT
  lcd.setCursor(20,1);
  switch (s){
    case 0:
     lcd.print("    ");
     break;
    case 1:
     lcd.print("FPM ");
     break;
    case 2:
     lcd.print("  FT");
     break;
    case 3:
     lcd.print("INHG");
     break;
    case 4:
     lcd.print("HPA ");
     break;
  }
}

void palt(long int a){
  int b,c;
  c=a%1000;
  b=a/1000;
  lcd.setCursor(18,0);
  if (a>999){ lcd.print(" 0.000");};
  if (b<10){ lcd.setCursor(19,0);}
  else     { lcd.setCursor(18,0);};
  lcd.print(b);
  if (c<100) {
      if (c<10) {lcd.setCursor(23,0);}
      else {lcd.setCursor(22,0);}
  } else {lcd.setCursor(21,0);};
  lcd.print(c);  
}

void setup()
{
  int c;
  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH);       // turn on pullup resistor
  pinMode(encoder0PinB, INPUT); 
  digitalWrite(encoder0PinB, HIGH);       // turn on pullup resistor
  attachInterrupt(0, doEncoder, CHANGE);  // encoder pin on interrupt 0 - pin 2
  
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.createChar(0,ch0);
  lcd.createChar(1,ch1);
  lcd.createChar(2,ch2);
  lcd.createChar(3,ch3);
  lcd.createChar(4,ch4);
  lcd.createChar(5,ch5);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(8,0);
  lcd.print("P F T");
  for(c=0;c<8;c++){
    lcd.setCursor(15,0);
    lcd.print(c);
    delay(400);
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("888     888");
  lcd.setCursor(0,1);
  lcd.print("888     888");
  arm1(1);
  arm2(1);
  ap(1);
  yd(1);
  pt(3);
  alert(1);
  mode(3);
  palt(88888);
  delay(3500);
  lcd.clear();
  // END OF INIT
  lcd.setCursor(0,0);
  lcd.print("HDG");
  lcd.setCursor(8,0);
  lcd.print("ALT");
  ap(1);
  alert(1);
  mode(2);
  palt(12500);
}


void loop()
{
  delay(500);
  pt(1);
  delay(500);
  pt(0);
}
