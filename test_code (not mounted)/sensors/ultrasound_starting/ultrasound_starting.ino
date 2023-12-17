#define CONT_ULTRA_TRIG_PIN 7
#define CONT_ULTRA_ECHO_PIN 8
#define N 50
float data[N];
int first = 1;
int i = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("----STARTING----");
  // put your setup code here, to run once:
  pinMode(CONT_ULTRA_TRIG_PIN,OUTPUT);
  pinMode(CONT_ULTRA_ECHO_PIN,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  float duration_us, distance_cm;
  float sum = 0;
  float mean = 0;
  float dev = 0;

  if(first){
    for(i=0;i<N;i++){

       digitalWrite(CONT_ULTRA_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(CONT_ULTRA_TRIG_PIN, LOW);

      duration_us = pulseIn(CONT_ULTRA_ECHO_PIN, HIGH);

      data[i] = duration_us * 0.017;

      sum = sum + data[i];

    }

    first = 0;
    
  }else{

    for(i=1;i<N;i++){

      data[i-1] = data[i];
      sum = sum + data[i-1];
    }

    digitalWrite(CONT_ULTRA_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(CONT_ULTRA_TRIG_PIN, LOW);

    duration_us = pulseIn(CONT_ULTRA_ECHO_PIN, HIGH);
    data[N-1] = duration_us * 0.017;

    sum = sum + data[N-1];

  }

  mean = sum/N;

  sum = 0;
  for(i=0;i<N;i++){
    sum = pow(data[i]-mean,2);
  }

  dev = sqrt(sum)/(N-1);
  
  Serial.print("standard deviation: "); Serial.println(dev);
  if(dev>10){
    Serial.println("------SEDUTO------");
  }
}
