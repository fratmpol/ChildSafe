void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("----STARTING----");
  pinMode(2,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = digitalRead(2);
  Serial.println(val);
}
