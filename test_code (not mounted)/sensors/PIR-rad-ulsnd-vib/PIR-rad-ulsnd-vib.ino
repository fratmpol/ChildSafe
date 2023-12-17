#define Vib_Pin 7
#define Pir_Pin 8
#define Rad_Pin 8
#define CO2_Pin A0
#define Trig_Pin 7
#define Echo_Pin 8
int out = 0;
int p_out = 0;
long duration;
int distance;

void setup() {
  // initialize serial communication at 9600 bits per second:
  pinMode(Vib_Pin,INPUT);
  pinMode(Pir_Pin,INPUT);
  pinMode(Rad_Pin,INPUT);
  pinMode(Trig_Pin,OUTPUT);
  pinMode(Echo_Pin,INPUT);
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {

// Vibrometer

//  p_out=out;
//  out=digitalRead(Vib_Pin);
//  if(p_out!=out){
//    Serial.println("VIBRAAAAAA"); 
//  }else{
//    Serial.println("NON VIBRA :(");
//  }
//  // delay in between reads for stability

// All the rest (PIR,Sonar)

//  out = digitalRead(Pir_Pin);
//  Serial.print("Readout: ");
//  Serial.println(out);
//  delay(100);

// CO2

//   out = analogRead(CO2_Pin);
//   Serial.print("Readout: ");
//   Serial.println(out);

 // Clears the Trig_Pin
  digitalWrite(Trig_Pin, LOW);
  delayMicroseconds(2);
  // Sets the Trig_Pin on HIGH state for 10 micro seconds
  digitalWrite(Trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig_Pin, LOW);
  // Reads the Echo_Pin, returns the sound wave travel time in microseconds
  duration = pulseIn(Echo_Pin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);



}
