// (c) Alvaro Alea Fernandez 2018

// Use Adafruit's PWM Servo library under BSD Licence. https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library

// For use with:
// My altimeter made with 3d Printer. https://www.thingiverse.com/thing:2744359
// A arduino UNO or similar
// FS2Link by Jim NZ http://www.jimspage.co.nz/intro.htm
// Flight Simulator X
//
// Stepmotor on 8,9,10 and 11
// encoder on 6,7
// button of encoder on 5
// optical sensor on 12


#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SRV_ZERO 0
#define SRV_SPAN 1
#define FSX_ZERO 2
#define FSX_SPAN 3
#define SRV_INIT 4
long servodata [16][5] = // SERVO MIN, SEVO SPAM, FS MIN, FS SPAM, center or init position
{
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 1 needle 1 - Fuel Left
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 1 needle 2 - Fuel Right
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 2 needle 1 - EGT Exausted Gas Temp
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 2 needle 2 - CHT Cylinder Temp
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 3 needle 1 - Oil Temperature
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 3 needle 2 - Oil Pressure
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 4 needle 1 - Volts
  {150, 600 - 150, 0, 1000 - 0, 375}, // mini gauge 4 needle 2 - Amps
  {150, 600 - 150, 0, 1000 - 0, 375}, // Airspeed
  {150, 600 - 150, 0, 1000 - 0, 375}, // Turn Coordinator, plane
  {150, 600 - 150, 0, 1000 - 0, 375}, // Turn Coordinator, ball
  {150, 600 - 150, 0, 1000 - 0, 375}, // Attitude, Pitch
  {150, 600 - 150, 0, 1000 - 0, 375}, // Attitude, Bank
  {150, 600 - 150, 0, 1000 - 0, 375}, // Vertical Speed
  {150, 600 - 150, 0, 1000 - 0, 375}, // RPM
  {150, 600 - 150, 0, 1000 - 0, 375} // Reserve
};

// Pinouts and other hardware definitions.
int mpap1_pins [4] = {8, 9, 10, 11};
int mpap1_pos = 0;

#define INIT_ENC_VALUE 2994
int enc_p1 = 6;
int enc_p2 = 7;
int enc_last_s;
int enc_s;
int enc_b = 5;
int enc_last_b = 0;
long enc_value = INIT_ENC_VALUE;

// If will I know how to programing OO, next will be a object.

#define MDELAY 2

// Steps sequence.
int paso [8][4] = // soft
{
  {1, 0, 0, 0},
  {1, 0, 0, 1},
  {0, 0, 0, 1},
  {0, 0, 1, 1},
  {0, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 0},
  {1, 1, 0, 0}
};

void mpap_init (int pins[]) {
  pinMode(pins[0], OUTPUT);
  pinMode(pins[1], OUTPUT);
  pinMode(pins[2], OUTPUT);
  pinMode(pins[3], OUTPUT);
}

int mpap_paso (int pins[], int pos, int dir) {
  if (dir) {
    pos++;
    pos = pos % 8;
  } else {
    if (pos <= 0) pos = 8;
    pos --;
  }
  digitalWrite(pins[0], paso[pos][0]);
  digitalWrite(pins[1], paso[pos][1]);
  digitalWrite(pins[2], paso[pos][2]);
  digitalWrite(pins[3], paso[pos][3]);
  delay(MDELAY);
  return pos;
}
int mpap_off (int pins[]) {
  digitalWrite(pins[0], 0);
  digitalWrite(pins[1], 0);
  digitalWrite(pins[2], 0);
  digitalWrite(pins[3], 0);
  delay(MDELAY);
}

void init_gauges(void) {
  int c;
  //first, advance 500feet in order to ensure that are off the sensor.
  for (c = 0; c < 2000; c++) {
    mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 1);
  }

  // only for check, if sensor is active, increase other 250feet.
  if (digitalRead(12) == 0) {
    for (c = 0; c < 1000; c++) {
      mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 1);
    }
  }

  // now search for the hole:
  while (digitalRead(12) == 1) {
    mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 0);
  }
  // offset because course hole in sensor.
  for (c = 0; c < 50; c++) {
    mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 0);
  }
}// end of init_gauges routine


void setup(void) {
  Serial.begin(115200);
  int c;
  // pinout configuration

  //   motor paso a paso
  mpap_init(mpap1_pins);

  //   encoder
  pinMode(enc_p1, INPUT_PULLUP);
  pinMode(enc_p2, INPUT_PULLUP);
  pinMode(enc_b, INPUT_PULLUP);
  enc_last_s = digitalRead(enc_p1);
  //   the zero position sensor
  pinMode(12, INPUT_PULLUP);

  //   de servo board.
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  for (c = 0; c < 16; c++) {
    pwm.setPWM(c, 0, servodata [c][SRV_INIT] );
  }

  init_gauges();
}// end of setup routine


long alt = 0; // altitud para la que se calculo posicion destino
long pos = 0; // posicion destino
long apos = 0; // posicion actual


// this motor need 4097 and a bit step in one revolution
#define MUL_S 4.0976
void new_alt (long newalt) {
  pos = newalt * MUL_S;
  alt = newalt;
}// end of new_alt routine

char getChar()// The serial buffer routine to get a character
{
  while (Serial.available() == 0); // wait for data
  return ((char)Serial.read()); // Thanks Doug
}// end of getchar routine


double serialget2 (void) {
  int a;
  int s1;
  int s2 = 0;
  s1 = getChar();
  if (s1 == '-') {
    s2 = 1;
    s1 = getChar();
  } else if (s1 == '+') {
    s1 = getChar();
  }
  a =   (       s1 - '0') * 10;
  a = a + (getChar() - '0');
  if (s2 == 1) {
    a = -a;
  }
  return (double)a;
}// end of serialget2 function


double serialget3 (void) {
  int a;
  int s1;
  int s2 = 0;
  s1 = getChar();
  if (s1 == '-') {
    s2 = 1;
    s1 = getChar();
  } else if (s1 == '+') {
    s1 = getChar();
  }
  a =   (       s1 - '0') * 100;
  a = a + (getChar() - '0') * 10;
  a = a + (getChar() - '0');
  if (s2 == 1) {
    a = -a;
  }
  return (double)a;
}// end of serialget3 function


double serialget5 (void) {
  long a;
  int s1;
  int s2 = 0;
  s1 = getChar();
  if (s1 == '-') {
    s2 = 1;
    s1 = getChar();
  } else if (s1 == '+') {
    s1 = getChar();
  }
  a =   (       s1 - '0') * 10000;
  a = a + (getChar() - '0') * 1000;
  a = a + (getChar() - '0') * 100;
  a = a + (getChar() - '0') * 10;
  a = a + (getChar() - '0');
  if (s2 == 1) {
    a = -a;
  }
  return (double)a;
}// end of serialget5 function

double serialget31 (void) {
  double a;
  int s1;
  int s2 = 0;
  s1 = getChar();
  if (s1 == '-') {
    s2 = 1;
    s1 = getChar();
  } else if (s1 == '+') {
    s1 = getChar();
  }
  a =   (       s1 - '0') * 10000;
  a = a + (getChar() - '0') * 1000;
  a = a + (getChar() - '0');
  a = a + (getChar() - '0') * 0.1;
  a = a + (getChar() - '0') * 0.01;
  if (s2 == 1) {
    a = -a;
  }
  return a;
}// end of serialget31 function


// This function get the info from FSX and translate to servo position.
void setservopos (int servo, double value) {
  double y;
  long x;
  y = (value - servodata [servo][FSX_ZERO]) / servodata [servo][FSX_SPAN] ;
  x = servodata [servo][SRV_ZERO] + (y * servodata [servo][SRV_SPAN]) ;
  pwm.setPWM(servo, 0, x );
}// end of setservopos function



void doserial (void) {
  // evaluate command from FS2Link
  int CodeIn;
  long a;
  if (Serial.available() > 0) {  //if there is a charactor in the serial receive buffer then ,,,,
    CodeIn = getChar();// read it via the "char getChar" routine
    if (CodeIn == '#') {// The first identifier is "#"
      CodeIn = getChar();
      if (CodeIn == 'a') {// "#a", altitude, we use custom FSUIPC offset 3324 to get indicated altitidude corrected with kosllman setting.
        new_alt(serialget5());
      }
      if (CodeIn == 'b') {// #b = stick of turn coordinator we use custom FSUIPC offset 037C because fs2link don't export this (but ball yes, WTF?)
        // lsetservopos(9,serialget3());
      }
    }// end if #
    if (CodeIn == '<') {// The first identifier is "<"
      CodeIn = getChar();
      if (CodeIn == 'P') {// <P = Airspeed
        setservopos(8, serialget3());
      }
      if (CodeIn == 'Q') {// <Q = Pitch (attitude)
        setservopos(11, serialget31());
      }
      if (CodeIn == 'R') {// <R = Roll (attitude)
        setservopos(12, serialget31());
      }
      if (CodeIn == 'L') {// <L = VSI
        setservopos(13, serialget5());
      }
      if (CodeIn == 'N') {// <N = ball of turn coordinator
        setservopos(10, serialget3());
      }
      if (CodeIn == 'T') {// <T = RPM1
        setservopos(14, serialget5());
      }
      if (CodeIn == 'O') {// <O = Oil Temp 1
        setservopos(4, serialget3());
      }
      if (CodeIn == 't') {// <t = Oil Press 1
        setservopos(5, serialget2());
      }
      if (CodeIn == 'X') {// <X = Left Fuel %
        setservopos(0, serialget2());
      }
      if (CodeIn == 'Z') {// <Z = Right Fuel %
        setservopos(1, serialget2());

      }
      if (CodeIn == 'v') {// <v = voltage
        setservopos(6, serialget2());

      }
    }// end if <
    if (CodeIn == '?') {// The first identifier is "?"
      CodeIn = getChar();
      if (CodeIn == 'M') {// ?M = EGT1
        setservopos(2, serialget3());
      }
      if (CodeIn == 'S') {// ?S = CHT1
        setservopos(3, serialget3());
      }
      if (CodeIn == 'O') {// ?O = Oil Temp 1
        setservopos(4, serialget3());
      }
      if (CodeIn == 'J') {// ?J = Current in Amps
        setservopos(7, serialget2());
      }
    }// end if ?
  }
}// end of doserial routine


void loop() {

  // check if need to move the needle
  if (pos > apos) {
    mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 1);
    apos++;
  }
  if (pos < apos) {
    mpap1_pos = mpap_paso(mpap1_pins, mpap1_pos, 0);
    apos--;
  }


  // Encoder
  int t;
  //   push-boton  only edge detection, no anti-bounce yet.
  t = digitalRead(enc_b);
  if (t == 1) {
    if (enc_last_b == 0) {
      enc_value = INIT_ENC_VALUE;
    }
  }
  enc_last_b = t;
  //   rotary
  enc_s = digitalRead(enc_p1);
  if (enc_s != enc_last_s) {
    if (digitalRead(enc_p2) != enc_s) {
      enc_value = enc_value - 4;
      Serial.println("C26");
      Serial.println("C26");
      Serial.println("C26");
      Serial.println("C26");
    } else {
      enc_value = enc_value + 4;
      Serial.println("C25");
      Serial.println("C25");
      Serial.println("C25");
      Serial.println("C25");
    }
  }
  enc_last_s = enc_s;

  // evaluate new indications from PC
  doserial();

}// end of loop routine

