int trig_Pin = 3;    // TRIG pin
int echo_Pin = 2;    // ECHO pin
float duration_us, distance_cm;
int car_width = 160; //[cm]
//float R = 287.05; //costante specifica per l'aria secca
//float gamma = 1.4; // c_p / c_v
//int T = 300; // [K]
//float c = sqrt(R*gamma*T) / 10000; // speed of sound for ideal gas in [cm/us]

#define N_Read 10

float dist_array[N_Read];



void setup() {
  // begin serial port
  Serial.begin (9600);

  // configure the trigger pin to output mode
  pinMode(trig_Pin, OUTPUT);
  // configure the echo pin to input mode
  pinMode(echo_Pin, INPUT);
}

void loop() {
  
  
  
  //generazione primi N_Read elementi dell'array
  for(int j=0; j<N_Read; j++){
   
  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Pin, LOW);

  // measure duration of pulse from ECHO pin
  duration_us = pulseIn(echo_Pin, HIGH);

  // calculate the distance (speed of sound = 340 m/s = 0.034 cm/us)
  distance_cm = 0.017 * duration_us; // distance = travel_distance / 2 = (0.034 cm/ micros  * pulse_dur) / 2  

   dist_array[j] = distance_cm;
  
  }
  int sum = 0; //inizializzo la somma ad ogni loop, atrimenti si accumula ad ogni loop

  //calculate walking mean 
  for(int i=0; i<N_Read; i++)
    {
      sum = sum + dist_array[i];
    }
int mean = sum / N_Read;

//leave an empty space for the new value
for(int i=0; i<(N_Read-1); i++){ 
      dist_array[i] = dist_array[i+1];
    }
//fill the empty space

// generate 10-microsecond pulse to TRIG pin
  digitalWrite(trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Pin, LOW);

  // measure duration of pulse from ECHO pin
  duration_us = pulseIn(echo_Pin, HIGH);

  // calculate the distance (speed of sound = 340 m/s = 0.034 cm/us)
  distance_cm = 0.017 * duration_us; // distance = travel_distance / 2 = (0.034 cm/ micros  * pulse_dur) / 2  

   
     
 dist_array[N_Read-1] = distance_cm;

  
  
  
  
  
  
   //print the value to Serial Monitor
  //Serial.print("distance: ");
  //Serial.print(distance_cm);
  //Serial.println(" cm");

  Serial.print("----mean distance: ");
  Serial.print(mean);
  Serial.println(" cm");

// Qui al posto di serial print si assegnerà il valore del peso
if(mean  > car_width){
Serial.print("finestrini aperti");
  
}

else{
  Serial.print("finestrini chiusi");

}

delay(500);
}




// Fondo scala: circa 1200 cm
// scarto medio : circa 0.5 cm



