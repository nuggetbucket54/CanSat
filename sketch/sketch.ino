#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;
File dataFile = SD.open("data.txt", FILE_WRITE);
int statusLED = 2;
int flash=1;

void setup() {
  Serial.begin(9600);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);
  unsigned status = bmp.begin(0x76, 0x58);
  if (!status) {
    Serial.println("No BMP sensor found, halting,,,");
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    while (true) delay(100); // stop program
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
  
  dataFile.print(F("Temperature = "));
  dataFile.print(bmp.readTemperature());
  dataFile.println(" *C");

  dataFile.print(F("Pressure = "));
  dataFile.print(bmp.readPressure());
  dataFile.println(" Pa");

  dataFile.print(F("Approx altitude = "));
  dataFile.print(bmp.readAltitude(1013.25)); 

  dataFile.println();
  delay(500);
  
  

}
