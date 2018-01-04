// (c) Alvaro Alea Fernandez 2018

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



// Pinouts and other hardware definitions.
int mpap1_pins [4] = {8,9,10,11};
int mpap1_pos = 0;

#define INIT_ENC_VALUE 2994
int enc_p1=6;
int enc_p2=7;
int enc_last_s;
int enc_s;
int enc_b=5;
int enc_last_b=0;
long enc_value=INIT_ENC_VALUE;

// If I know how to programing OO, next will be a object.

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

void mpap_init (int pins[]){
pinMode(pins[0], OUTPUT);
pinMode(pins[1], OUTPUT);
pinMode(pins[2], OUTPUT);
pinMode(pins[3], OUTPUT);
}

int mpap_paso (int pins[],int pos, int dir){
if (dir){
  pos++;
  pos = pos % 8;
  } else {
  if (pos<=0) pos = 8;
  pos --;
  }
  digitalWrite(pins[0], paso[pos][0]);  
  digitalWrite(pins[1], paso[pos][1]);
  digitalWrite(pins[2], paso[pos][2]);
  digitalWrite(pins[3], paso[pos][3]);
  delay(MDELAY);
  return pos;
}
int mpap_off (int pins[]){
  digitalWrite(pins[0], 0);  
  digitalWrite(pins[1], 0);
  digitalWrite(pins[2], 0);
  digitalWrite(pins[3], 0);
  delay(MDELAY);
}

void init_gauges (void){
int c;
//first, advance 500feet in order to ensure that are off the sensor.
for (c=0;c<2000;c++) {  
    mpap1_pos =mpap_paso(mpap1_pins,mpap1_pos,1);
}

// only for check, if sensor is active, increase other 500feet.
if (digitalRead(12)==0) {
for (c=0;c<1000;c++) {  
   mpap1_pos =mpap_paso(mpap1_pins,mpap1_pos,1);
}
}

// now search for the hole:
while (digitalRead(12)==1){
    mpap1_pos =mpap_paso(mpap1_pins,mpap1_pos,0);
}
// offset because course hole in sensor.
for (c=0;c<50;c++) {  
    mpap1_pos =mpap_paso(mpap1_pins,mpap1_pos,0);
}
}


void setup()
{
Serial.begin(115200);
// pinout configuration
//   motor paso a paso
mpap_init(mpap1_pins);
//   encoder
pinMode(enc_p1, INPUT_PULLUP);
pinMode(enc_p2, INPUT_PULLUP);
pinMode(enc_b, INPUT_PULLUP);
enc_last_s=digitalRead(enc_p1);
//   el optoacoplador del paso por cero
pinMode(12, INPUT_PULLUP);
pinMode(13, OUTPUT);
Serial.println("Hola");
init_gauges();
}


long alt=0;  // altitud para la que se calculo posicion destino
long pos=0;  // posicion destino
long apos=0; // posicion actual

void debug (void){
  Serial.println("ALT:    POS:    APOS:");
  Serial.print(alt);
  Serial.print("  ");
  Serial.print("  ");
  Serial.print(pos);  
  Serial.print("  ");
  Serial.println(apos); 
}


// this motor need 4097 and a bit step in one revolution
#define MUL_S 4.0976
void new_alt(long newalt){
//long t;
//t=new_alt-alt;
//if (abs(t)>20){ // with this marvelous motors, it's better to avoid small changes.
pos=newalt*MUL_S;
alt=newalt;
}// end of new_alt routine

char getChar()// The serial buffer routine to get a character
{
  while(Serial.available() == 0);// wait for data
  return((char)Serial.read());// Thanks Doug
}// end of getchar routine 

void doserial(void){
// evaluate command from FS2Link
int CodeIn; 
long a;
int s1;
int s2=0;
 if (Serial.available() > 0) {  //if there is a charactor in the serial receive buffer then ,,,,
    CodeIn = getChar();// read it via the "char getChar" routine
    if (CodeIn == '#') {// The first identifier is "#" 
       CodeIn = getChar();
       if (CodeIn == 'a') {// The second identifier is "a", we use custom FSUIPC offset 3324 to get indicated altitidude corrected with kosllman setting.
       s1=getChar();
       if (s1=='-') {
        s2=1;
        s1=getChar();
       } 
       a=   (       s1-'0')*10000;
       a=a+ (getChar()-'0')*1000;
       a=a+ (getChar()-'0')*100;
       a=a+ (getChar()-'0')*10;
       a=a+ (getChar()-'0')*1;
       if (s2==1) {
          a=-a;
       }
       new_alt(a);
       }
    }
 }
}


void loop(){
// test para el optoacoplador
digitalWrite(13,digitalRead(12));

// check if need to move the needle
if (pos>apos) {
  mpap1_pos = mpap_paso(mpap1_pins,mpap1_pos,1);
  apos++;
}
if (pos<apos) {
  mpap1_pos = mpap_paso(mpap1_pins,mpap1_pos,0);
  apos--;  
}
/*
if (pos==apos) { // as alt_c is float it drift a bit.
    mpap_off(mpap1_pins);  //Check if this cause troubles is for avoid heating
}
*/

// Encoder
int t;
//   push-boton  only edge detection, no anti-bounce yet.
t=digitalRead(enc_b);
if (t==1) {
    if (enc_last_b==0) {
        enc_value=INIT_ENC_VALUE;
    }
}
enc_last_b=t;
//   rotary
enc_s=digitalRead(enc_p1);
if (enc_s != enc_last_s) {
  if (digitalRead(enc_p2)!=enc_s){
    enc_value=enc_value-4;
    Serial.println("C26");
    Serial.println("C26");
    Serial.println("C26");
    Serial.println("C26");
  } else {
    enc_value=enc_value+4;
    Serial.println("C25");
    Serial.println("C25");
    Serial.println("C25");
    Serial.println("C25");
  }
}
enc_last_s=enc_s;

// evaluate new indications from PC
doserial();

}
