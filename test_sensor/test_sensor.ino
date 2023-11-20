#include <Arduino.h>
#define PIN 5
#define N 50
int val[N];
int firstPIRVal = 1;
int i = 0;

void setup(){
    Serial.begin(115200);
    Serial.println("----STARTING----");
    pinMode(PIN,INPUT);
}

void loop(){
    Serial.print("is moving: "); Serial.println(digitalRead(PIN));
    delay(100);
}

void loop34(void){
    int sum = 0;
  
    if(firstPIRVal){

        Serial.println("---FIRST READOUT---");
        Serial.println("Array values: ");

        for(i = 0; i<N; i++){
        val[i] = digitalRead(PIN);
        sum = sum + val[i];
        Serial.print(i); Serial.print(" -> "); Serial.println(val[i]);
        }

        firstPIRVal = 0;

    }else{

        Serial.println("---WINDOW READOUT---");
        Serial.println("Array values: ");

        for(i=1; i<N; i++){
            val[i-1] = val[i];
            sum = sum + val[i-1];
        }
        val[N-1] = digitalRead(PIN);
        sum = sum + val[N-1];
        for(i=0; i<N; i++){
        Serial.print(i); Serial.print(" -> "); Serial.println(val[i]);
        }

    }

    Serial.print("Sum of the RAD readouts: "); Serial.println(sum);

    if(sum!=0){
        Serial.println("CHANGE");
    }else{
        Serial.println("NO CHANGE");
    }

    //delay for analysis
}