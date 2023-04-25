#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include <Filters.h> // https://github.com/JonHub/Filters

// noise filter imports
extern "C" {
  #include "biquad.h"
  #include "boost_filter.h"
  #include "detrend_filter.h"
  #include "denoise_filter.h"
}

#define gpsBaud 9600

#define loraRX 4
#define loraTX 5
#define loraBaud 38400


TinyGPSPlus gps;
Adafruit_BMP085 bmp;
//File dataFile = SD.open("data.txt", FILE_WRITE);
byte flash=1;

SoftwareSerial loraModule(loraRX, loraTX);

// data variables
float temp, pressure, altitude;
double latitude=0.0, longitude=0.0;
TinyGPSDate date;
TinyGPSTime time;

char dataSep[] = ",";
char messagePrefix[] = "AT+SEND=5656,";
// none of this shit is memory safe...
// if I buffer overflow I will neck rope
char dataStorage[241];
char messageBuf[260];
char dtosbuf[16]; // stores intermetiates like double/string conversion results
char seismometerBuf[187]="";

// seismometer variables
FilterOnePole XFHigh(HIGHPASS, 1), YFHigh(HIGHPASS, 1), ZFHigh(HIGHPASS, 1);
MPU6050 mpu(Wire);

biquad_z_t boost_z[BOOST_BIQUADS_SIZE];
biquad_z_t detrend_z[DETREND_BIQUADS_SIZE];

int out = 0;
int ANALOGSEISMO = 0;
int first_loop = 1; // flag for processing samples
float Bfmult;
float BfdB = 1; // scale factor of filter (scale = 10^BfdB)
float AcX,AcY,AcZ,AngX,AngY,AngZ;
float xy_vector_mag, z_vector_mag;
const float scale_factor = pow(2, 15);

// creating an array to be used in filter calculations
const unsigned int ncoeff = sizeof(coeff) / sizeof(coeff[0]);
float lagarray[ncoeff];

bool loraReady=false;

void setup() {
  Serial.begin(gpsBaud);
  pinMode(A3, OUTPUT);
  loraModule.begin(loraBaud);
  loraModule.listen();
  loraModule.println("AT+NETWORKID=12");
  Wire.begin();

  unsigned bmpstatus = bmp.begin();
  if (!bmpstatus) {
    while (true) {
      digitalWrite(A3, LOW);
      delay(50);      
    }; // stop program
  }
  byte mpustatus = mpu.begin();
  while (mpustatus != 0) {
      digitalWrite(A3, LOW);
    mpustatus = mpu.begin();
    delay(50);
  }
  // mpu.upsideDownMounting = true;
  mpu.calcOffsets();
  loraModule.listen();
  
}

void loop() {
  // for loading lora parameters

  temp = bmp.readTemperature();
  pressure = bmp.readPressure();
  mpu.update();

  AngX = mpu.getAngleX();
  AngY = mpu.getAngleY();
  AngZ = mpu.getAngleZ();

  AcX = mpu.getAccX();
  AcY = mpu.getAccY();
  AcZ = mpu.getAccZ();

  AcX = (AcX * cos(AngY) * cos(AngZ)) - (AcY * cos(AngY) * sin(AngZ)) + (AcZ * cos(AngY));
  AcY = (AcX * (sin(AngX) * sin(AngY) * cos(AngZ) + cos(AngX) * sin(AngZ))) + (AcY * (cos(AngX) * cos(AngZ) - sin(AngX) * sin(AngY) * sin(AngZ))) - (AcZ * sin(AngX) * cos(AngY));
  AcZ = (AcX * (sin(AngX) * sin(AngZ) - cos(AngX) * sin(AngY) * cos(AngZ))) + (AcY * (cos(AngX) * sin(AngY) * sin(AngZ) + sin(AngX) * cos(AngZ))) + (AcZ * cos(AngX) * cos(AngY));

  XFHigh.input(AcX / 16384.0);
  YFHigh.input(AcY / 16384.0);
  ZFHigh.input(AcZ / 16384.0 - 1.0);

  z_vector_mag = abs(ZFHigh.output() * scale_factor) * 100;
  int zprepared = z_vector_mag;
  zprepared %= 100;
  int csiz = strlen(seismometerBuf-1);
  if (csiz < 185) {
    dtosbuf[0] = '\0';
    sprintf(dtosbuf, "%d", zprepared);
    dtosbuf[2] = '\0';
    if (zprepared < 10) {
      strcat(seismometerBuf, "0");
      dtosbuf[1] = '\0';
    }
    //Serial.println(dtosbuf);
    strcat(seismometerBuf, dtosbuf);
    strcat(seismometerBuf, ",");

  }

  if (loraReady) {
    digitalWrite(A3, LOW);    
    loraReady = false;

    strcpy(dataStorage, dataSep);
    addData(temp, 1, true);
    addData(pressure, 0, true);
    strcat(dataStorage, seismometerBuf);

    if (gps.location.isValid()) {
      latitude = gps.location.lat();  
      addData(latitude, 7, true);
      longitude = gps.location.lng(); 
      addData(longitude, 7, false);
    } else {
      strcat(dataStorage, "X,X");
    }
    seismometerBuf[0] = '\0';
    sprintf(dtosbuf, "%d", strlen(dataStorage)-1);
    strcpy(messageBuf, messagePrefix);
    strcat(messageBuf, dtosbuf);
    strcat(messageBuf, dataStorage);
    loraModule.println(messageBuf);
    Serial.println(messageBuf);
  } else {
    digitalWrite(A3, HIGH);
  }

  smartDelay(100);
}
  

static void addData(double val, int precis, bool sep) {
  dtosbuf[0] = '\0';
  dtostrf(val, 1, precis, dtosbuf);
  strcat(dataStorage, dtosbuf);
  if (sep)
    strcat(dataStorage, dataSep);    
}


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available())
      gps.encode(Serial.read());
    while (loraModule.available()) {
      Serial.print(loraModule.read());
      loraReady=true;
    }
  } while (millis() - start < ms);
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