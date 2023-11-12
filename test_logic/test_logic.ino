#include <Arduino.h>

#define button1Pin 3
#define button2Pin 4
#define button3Pin 5
#define led1Pin 7
#define led2Pin 8
#define led3Pin 9


int press1 = 0;
int press2 = 0;
int press3 = 0;


void setup(){
    Serial.begin(9600);
    pinMode(led1Pin,OUTPUT);
    pinMode(button1Pin,INPUT);
    pinMode(led2Pin,OUTPUT);
    pinMode(button2Pin,INPUT);
    pinMode(led3Pin,OUTPUT);
    pinMode(button3Pin,INPUT);
}

void loop() {
    press1 = digitalRead(button1Pin);
    press2 = digitalRead(button2Pin);
    press3 = digitalRead(button3Pin);
    if (press1 == LOW){
        digitalWrite(led1Pin,HIGH);
        Serial.println("PRESSED 1");
    } else {
        digitalWrite(led1Pin,LOW);
        Serial.println("NOT PRESSED 1");
    }
    if (press2 == LOW){
        digitalWrite(led2Pin,HIGH);
        Serial.println("PRESSED 2");
    } else {
        digitalWrite(led2Pin,LOW);
        Serial.println("NOT PRESSED 2");
    }
    if (press3 == LOW){
        digitalWrite(led3Pin,HIGH);
        Serial.println("PRESSED 3");
    } else {
        digitalWrite(led3Pin,LOW);
        Serial.println("NOT PRESSED 3");
    }
    delay(100);
}