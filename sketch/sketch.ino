#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define gpsRX 4
#define gpsTX 3
#define gpsBaud 9600

#define loraRX 8
#define loraTX 9
#define loraBaud 115200

int statusLED = 2;

TinyGPSPlus gps;

Adafruit_BMP280 bmp;
File dataFile = SD.open("data.txt", FILE_WRITE);
int flash=1;

SoftwareSerial gpsModule(gpsRX, gpsTX);
SoftwareSerial loraModule(loraRX, loraTX);

// data variables
float temp, pressure, altitude;
double latitude, longitude;
TinyGPSDate date;
TinyGPSTime time;

char dataSep[] = ",";
char dataStorage[50];
char messageBuf[50];
char dtosbuf[16];

void setup() {
  Serial.begin(115200);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);
  gpsModule.begin(gpsBaud);
  loraModule.begin(loraBaud);
  unsigned status = bmp.begin(0x76);
  if (!status) {
    Serial.println("No BMP sensor found, halting,,,");
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    while (true) {
      digitalWrite(statusLED, digitalRead(statusLED)^1);
      delay(50);      
    }; // stop program
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  Serial.println("Beginning data collection.");

}

void loop() {

  digitalWrite(statusLED, flash);
  flash ^= 1;
  strcpy(dataStorage, dataSep);
  temp = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude(1013.25);
  addData(temp, 2, true);
  addData(pressure, 2, true);
  addData(altitude, 2, false);
  

  if (gps.location.isValid()) {
    latitude = gps.location.lat();  
    strcat(dataStorage, dataSep);
    addData(latitude, 6, true);
      
    Serial.print(F("Latitude: "));
    Serial.println(latitude, 6);

    longitude = gps.location.lng(); 
    addData(longitude, 8, false);
    Serial.print(F("Longitude: "));
    Serial.println(longitude, 6);
    
  }

  loraModule.listen();
  loraModule.println("AT+SEND=0,5,HELLO");
  gpsModule.listen();
  
  Serial.print(F("Temperature = "));
  Serial.print(temp);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure);
  Serial.println(" Pa");

  Serial.print(F("Approx altitude = "));
  Serial.print(altitude); 

  Serial.println();
  Serial.println(dataStorage);

  sprintf(dtosbuf, "%d", strlen(dataStorage)-1);
  //strcpy(messageBuf, "AT+SEND=0,");
  //strcpy(messageBuf, dataSep);

  
  smartDelay(500);
}
  

static void addData(double val, int precis, bool sep) {
  dtosbuf[0] = '\0';
  dtostrf(val, 1, precis, dtosbuf);
  Serial.println(dtosbuf);
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



