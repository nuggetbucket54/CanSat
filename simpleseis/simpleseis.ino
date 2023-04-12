#include <Wire.h>
#include <MPU6050_light.h>
#include <Filters.h>

// imports for noise filters
extern "C" {
  #include "biquad.h"
  #include "boost_filter.h"
  #include "detrend_filter.h"
  #include "denoise_filter.h"
}

FilterOnePole XFHigh(HIGHPASS, 1), YFHigh(HIGHPASS, 1), ZFHigh(HIGHPASS, 1);
MPU6050 mpu(Wire);

biquad_z_t boost_z[BOOST_BIQUADS_SIZE];
biquad_z_t detrend_z[DETREND_BIQUADS_SIZE];

int out = 0;
int ANALOGSEISMO = 0;
int first_loop = 1; // flag for processing samples
float Bfmult;
float BfdB = 1; // scale factor of filter (scale = 10^BfdB)
float AcX,AcY,AcZ;
float xy_vector_mag, z_vector_mag;
const float scale_factor = pow(2, 15);

// creating an array to be used in filter calculations
const unsigned int ncoeff = sizeof(coeff) / sizeof(coeff[0]);
float lagarray[ncoeff];


void setup() {
  Serial.begin(9600);

  Wire.begin();
  byte status = mpu.begin();

  Serial.print(F("MPU6050 status: "));
  Serial.println(status);

  while (status != 0) {
    status = mpu.begin();
  } //wait for MPU6050 to connect

  //mpu.upsideDownMounting = true;
  mpu.calcOffsets();
  Serial.println("Done! \n");
}

float process_sample(const float y) {
  float z;
  float fL;
  unsigned int i;

  if (first_loop) {
    first_loop = 0;
  
    // flood the lag array with the first value
    for (i = 1; i < ncoeff; i++) {
        lagarray[i] = y;
    }

    // initialize the biquad delay lines
    biquad_clear(detrend_z, DETREND_BIQUADS_SIZE, (biquad_sample_t) y);
    biquad_clear(boost_z, BOOST_BIQUADS_SIZE, (biquad_sample_t) y);

    // compute the multiplicative gain factor
    Bfmult = pow(10.0, BfdB);

  } else {
    // update the bucket brigade
    for (i = ncoeff - 1; i > 0; i--) {
      lagarray[i] = lagarray[i - 1];
    }
    lagarray[0] = y;

  }

  // apply N to the raw sample series
  z = 0.0;
  for(i = 0; i < ncoeff; i++)
      z += lagarray[i] * coeff[i];

  // apply L to the output of BN
  fL = biquad_filter(z, boost_biquads, boost_z, BOOST_BIQUADS_SIZE) * boost_biquads_g;
  // compute and return the weighted trace
  return z + Bfmult * fL;
}

void loop() {


  mpu.update();
  AcX = mpu.getAccX();
  AcY = mpu.getAccY();
  AcZ = mpu.getAccZ();
  // Serial.println(AcX);
  // Serial.println(AcY);
  // Serial.println(AcZ);

  //  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  //  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  //  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  //  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  
  XFHigh.input(AcX / 16384.0);
  YFHigh.input(AcY / 16384.0);
  ZFHigh.input(AcZ / 16384.0 - 1.0);
  xy_vector_mag = sqrt(XFHigh.output() * XFHigh.output() + YFHigh.output() * YFHigh.output()) * scale_factor;
  z_vector_mag = abs(ZFHigh.output() * scale_factor);
  // Serial.println("|XY| = "); Serial.println(xy_vector_mag); 
  Serial.println(z_vector_mag);

  delay(100);
}