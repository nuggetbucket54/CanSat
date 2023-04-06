#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

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

void setup() {
  Serial.begin(115200);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);
  gpsModule.begin(gpsBaud);
  loraModule.begin(loraBaud);
  Serial.println(messagePrefix);
  unsigned status = bmp.begin();
  if (!status) {
    Serial.println("No BMP sensor found, halting,,,");
    while (true) {
      digitalWrite(statusLED, digitalRead(statusLED)^1);
      delay(50);      
    }; // stop program
  }
 
  Serial.println("Beginning data collection.");

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



