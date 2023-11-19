#define N_samples 10
int arr[N_samples];
float sum = 0;
float dev = 0;

#include<Wire.h>

const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ;
void setup() {
 
  Serial.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
}

void loop() {

 // Raccolta primi N_samples valori--questo for si svolgerà solo una volta
 for (int j = 0; j < N_samples ; j++){
  
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  //Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  //GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  //GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  //GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
   
  arr[j] = AcX;
  
  }

int sum = 0;  //Azzero ad ogni loop la somma, così da sommare sempre N_samples valori ad ogni loop

//calculate walking mean 
 
  for(int i=0; i<N_samples; i++){
      sum = sum + arr[i];
    }
  int mean = sum / N_samples;


 // Deviazione Standard
   sum = 0;
 
  for (int i=0; i < N_samples ; i++){
   sum =  sum + pow(arr[i] - mean, 2);
    
  //long  int acceleraz_x = arr[i];
 }
 
 dev = sqrt( sum / (N_samples - 1) );
 Serial.println(dev);

////shift and leave an empty space for the new value
for(int i=0; i<(N_samples-1); i++){ 
      arr[i] = arr[i+1];
    }
//fill the empty space with the new value

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  //Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  //GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  //GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  //GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
   
  arr[N_samples-1] = AcX; //update just the last vale of the array

  Serial.print("standard deviation: ");
  Serial.print(dev);
  

}

 




 
 

 
  


 
  
  //Serial.println(AcX);
  