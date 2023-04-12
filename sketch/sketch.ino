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

#define gpsRX 4
#define gpsTX 3
#define gpsBaud 9600

#define loraRX 8
#define loraTX 9
#define loraBaud 115200

#define statusLED 2

TinyGPSPlus gps;
Adafruit_BMP085 bmp;
//File dataFile = SD.open("data.txt", FILE_WRITE);
byte flash=1;

SoftwareSerial gpsModule(gpsRX, gpsTX);
SoftwareSerial loraModule(loraRX, loraTX);

// data variables
float temp, pressure, altitude;
double latitude, longitude;
TinyGPSDate date;
TinyGPSTime time;

char dataSep[] = ",";
char messagePrefix[] = "AT+SEND=0,";
// none of this shit is memory safe...
// if I buffer overflow I will neck rope
char dataStorage[50];
char messageBuf[50];
char dtosbuf[16]; // stores intermetiates like double/string conversion results

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
float AcX,AcY,AcZ;
float xy_vector_mag, z_vector_mag;
const float scale_factor = pow(2, 15);

// creating an array to be used in filter calculations
const unsigned int ncoeff = sizeof(coeff) / sizeof(coeff[0]);
float lagarray[ncoeff];


void setup() {
  Serial.begin(115200);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);
  gpsModule.begin(gpsBaud);
  loraModule.begin(loraBaud);
  Wire.begin();
  Serial.println(messagePrefix);

  unsigned bmpstatus = bmp.begin();
  if (!bmpstatus) {
    Serial.println("No BMP sensor found, halting,,,");
    while (true) {
      digitalWrite(statusLED, digitalRead(statusLED)^1);
      delay(50);      
    }; // stop program
  }
  Serial.println("Beginning BMP data collection.");

  byte mpustatus = mpu.begin();
  while (mpustatus != 0) {
    mpustatus = mpu.begin();
    delay(50);
  }
  // mpu.upsideDownMounting = true;
  mpu.calcOffsets();
  Serial.println("Beginning MPU data collection.") ;
}


void loop() {

  digitalWrite(statusLED, flash);
  flash ^= 1;
  strcpy(dataStorage, dataSep);
  temp = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude(101500);
  addData(temp, 1, true);
  addData(pressure, 0, true);
  addData(altitude, 1, false);
  

  if (gps.location.isValid()) {
    latitude = gps.location.lat();  
    strcat(dataStorage, dataSep);
    addData(latitude, 7, true);


    longitude = gps.location.lng(); 
    addData(longitude, 7, false);

    
  } 
  
  mpu.update();
  AcX = mpu.getAccX();
  AcY = mpu.getAccY();
  AcZ = mpu.getAccZ();

  XFHigh.input(AcX / 16384.0);
  YFHigh.input(AcY / 16384.0);
  ZFHigh.input(AcZ / 16384.0 - 1.0);
  xy_vector_mag = sqrt(XFHigh.output() * XFHigh.output() + YFHigh.output() * YFHigh.output()) * scale_factor;
  z_vector_mag = abs(ZFHigh.output() * scale_factor);
  Serial.println(z_vector_mag);

  //Serial.print(F("Temperature = "));
  //Serial.print(temp);
  //Serial.println(" *C");

  //Serial.print(F("Pressure = "));
  //Serial.print(pressure);
  //Serial.println(" Pa");

  //Serial.print(F("Approx altitude = "));
  //Serial.print(altitude); 

  //Serial.println();
  //Serial.println(dataStorage);

  sprintf(dtosbuf, "%d", strlen(dataStorage)-1);
  strcpy(messageBuf, messagePrefix);
  strcat(messageBuf, dtosbuf);
  strcat(messageBuf, dataStorage);
  Serial.println(messageBuf);
  loraModule.listen();
  loraModule.println(messageBuf);
  gpsModule.listen();

  
  smartDelay(500);
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
  gpsModule.listen();
  unsigned long start = millis();
  do 
  {
    while (gpsModule.available())
      gps.encode(gpsModule.read());
  } while (millis() - start < ms);
}


static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{

  char sz[32];
  sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
  Serial.print(sz);
  
  
  sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
  Serial.print(sz);

  smartDelay(0);
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