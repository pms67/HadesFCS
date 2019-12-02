#include "MPU9250.h"
#include "KalmanRollPitch.h"
#include "FIR.h"

MPU9250 mpu(Wire, 0x68);
KalmanRollPitch kal;

FIRFilter lpfGyrP;
FIRFilter lpfGyrQ;
FIRFilter lpfGyrR;

FIRFilter lpfAccX;
FIRFilter lpfAccY;
FIRFilter lpfAccZ;

float lpfGyrCoeff[] = {0.0000069f,0.0000000f,-0.0000899f,-0.0001611f,0.0003930f,0.0013753f,0.0000000f,-0.0049735f,-0.0053085f,0.0087375f,0.0224203f,0.0000000f,-0.0531261f,-0.0511723f,0.0857082f,0.2961945f,0.4000000f,0.2961945f,0.0857082f,-0.0511723f,-0.0531261f,0.0000000f,0.0224203f,0.0087375f,-0.0053085f,-0.0049735f,0.0000000f,0.0013753f,0.0003930f,-0.0001611f,-0.0000899f,0.0000000f};
float lpfAccCoeff[] = {-0.0000011f,-0.0000139f,-0.0000662f,-0.0001802f,-0.0003389f,-0.0004011f,0.0000000f,0.0015241f,0.0050860f,0.0116617f,0.0219439f,0.0359377f,0.0526573f,0.0700909f,0.0855263f,0.0961888f,0.1000000f,0.0961888f,0.0855263f,0.0700909f,0.0526573f,0.0359377f,0.0219439f,0.0116617f,0.0050860f,0.0015241f,0.0000000f,-0.0004011f,-0.0003389f,-0.0001802f,-0.0000662f,-0.0000139f};

float lpfGyrPBuf[32];
float lpfGyrQBuf[32];
float lpfGyrRBuf[32];

float lpfAccXBuf[32];
float lpfAccYBuf[32];
float lpfAccZBuf[32];

void setup() {

  Serial.begin(115200);

  IMU_Init();

  float gyroNoiseStdDev = 0.01f * (M_PI / 180.0f) * 10.0f;
  float accNoiseStdDev  = 0.0003f * 9.81f * 10.0f;

  float Pinit = 10.0f;
  float Q[] = {3.0f * gyroNoiseStdDev * gyroNoiseStdDev, 2.0f * gyroNoiseStdDev * gyroNoiseStdDev};
  float R[] = {accNoiseStdDev, accNoiseStdDev, accNoiseStdDev};

  KalmanRollPitch_Init(&kal, Pinit, Q, R);

  FIRFilter_Init(&lpfGyrP, lpfGyrCoeff, lpfGyrPBuf, 32);
  FIRFilter_Init(&lpfGyrQ, lpfGyrCoeff, lpfGyrQBuf, 32);
  FIRFilter_Init(&lpfGyrR, lpfGyrCoeff, lpfGyrRBuf, 32);

  FIRFilter_Init(&lpfAccX, lpfAccCoeff, lpfAccXBuf, 32);
  FIRFilter_Init(&lpfAccY, lpfAccCoeff, lpfAccYBuf, 32);
  FIRFilter_Init(&lpfAccZ, lpfAccCoeff, lpfAccZBuf, 32);
}

void loop() {

  IMU_Update();

  float acc[3];
  float gyr[3];
  
  IMU_getAcc(acc);
  IMU_getGyr(gyr);

  /* Filter measurements */
  FIRFilter_Update(&lpfGyrP, gyr[0]);
  FIRFilter_Update(&lpfGyrQ, gyr[1]);
  FIRFilter_Update(&lpfGyrR, gyr[2]);

  FIRFilter_Update(&lpfAccX, acc[0]);
  FIRFilter_Update(&lpfAccY, acc[1]);
  FIRFilter_Update(&lpfAccZ, acc[2]);

  gyr[0] = lpfGyrP.out;
  gyr[1] = lpfGyrQ.out;
  gyr[2] = lpfGyrR.out;

  acc[0] = lpfAccX.out;
  acc[1] = lpfAccY.out;
  acc[2] = lpfAccZ.out;

  KalmanRollPitch_Update(&kal, gyr, acc, 0.0f, 0.01f);
   
  Serial.print(kal.phi * 180.0f / M_PI, 3); Serial.print(" "); Serial.println(kal.theta * 180.0f / M_PI, 3);

  delay(10);
  
}


int IMU_Init() {
  return mpu.begin();
}

void IMU_Update() {
  mpu.readSensor();
}

void IMU_getAcc(float *acc) {
  acc[0] = 0.995f * mpu.getAccelX_mss() - 0.185f;
  acc[1] = 1.001f * mpu.getAccelY_mss() + 0.0703f;
  acc[2] = 1.010f * mpu.getAccelZ_mss() - 1.219f;
}

void IMU_getGyr(float *gyr) {
  gyr[0] = mpu.getGyroX_rads();
  gyr[1] = mpu.getGyroY_rads();
  gyr[2] = mpu.getGyroZ_rads();
}

void IMU_getMag(float *mag) {
  mag[0] = mpu.getMagX_uT();
  mag[1] = mpu.getMagY_uT();
  mag[2] = mpu.getMagZ_uT();
}

void IMU_Print() {
    Serial.print(mpu.getAccelX_mss(),6);
    Serial.print("\t");
    Serial.print(mpu.getAccelY_mss(),6);
    Serial.print("\t");
    Serial.print(mpu.getAccelZ_mss(),6);
    Serial.print("\t");
    Serial.print(mpu.getGyroX_rads(),6);
    Serial.print("\t");
    Serial.print(mpu.getGyroY_rads(),6);
    Serial.print("\t");
    Serial.print(mpu.getGyroZ_rads(),6);
    Serial.print("\t");
    Serial.print(mpu.getMagX_uT(),6);
    Serial.print("\t");
    Serial.print(mpu.getMagY_uT(),6);
    Serial.print("\t");
    Serial.println(mpu.getMagZ_uT(),6);
}
