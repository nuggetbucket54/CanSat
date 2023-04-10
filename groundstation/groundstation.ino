#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define loraBaud 38400 // Ground station lora module has baud rate set lower - SoftwareSerial can't handle high throughput
#define pageBut 6
// Pins for rx/tx of Lora module
SoftwareSerial lora(9, 8);
// Create lcd object with i2c address, display dimensions
LiquidCrystal_I2C lcd(0x27,16,2);  
// stores incoming messages
char loraBuffer[80];
int bufferPos = 0;
int clock = 0;

char temp[7], pressure[7], altitude[8];
char lat[13], lng[13]; // 1 byte for sign/3 for degree/8 for precision
const char dataSep[] = ",";
// "page" of the display - changing this causes the display to show different data
int state=0;
int lastPacket=-1; // ms since last packet was received
int ticks=0;
int butDelay=0;


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pageBut, INPUT);

  lora.begin(loraBaud);
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  

}


void loop()
{
  ticks++;
  if (lastPacket >= 0) {
    lastPacket++;
  }
  if (butDelay > 0)
    butDelay--;
  smartDelay(1);
  if (digitalRead(pageBut) == HIGH && butDelay == 0) {
    butDelay = 200;
    state = (state+1)%3;
  }

  // Serial.print(F("Temperature: "));
  // Serial.print(temp);
  // Serial.println(" *C");

  // Serial.print(F("Pressure: "));
  // Serial.print(pressure);
  // Serial.println(" Pa");

  // Serial.print(F("Approx altitude: "));
  // Serial.println(altitude); 

  // Serial.print(F("Latitude: "));
  // Serial.println(lat); 
  // Serial.print(F("Longitude: "));
  // Serial.println(lng); 

  // refresh every 500 ticks
  if (ticks == 500) {
    ticks = 0;
    // refresh display according to the state
    lcd.clear();
    switch (state) {
      case 0: // barometric readings
        lcd.setCursor(0, 0);
        lcd.print(temp);
        lcd.print(" C ");
        lcd.setCursor(8, 0);      
        lcd.print(pressure);
        lcd.print("Pa");

        lcd.setCursor(0, 1);
        lcd.print("Alt: ");
        lcd.print(altitude);
        lcd.print(" m");
        break;
      
      case 1: // location readings
        lcd.setCursor(0, 0);
        lcd.print("Lat ");
        lcd.print(lat);
        lcd.setCursor(0, 1);
        lcd.print("Long ");
        lcd.print(lng);

    }
  }

  

}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    if (lora.available())
      flushLora();
  } while (millis() - start < ms);
}

void flushLora() {

  while (lora.available())
  {
    char k = char(lora.read());

    loraBuffer[bufferPos] = k;
    bufferPos ++;
  
    if (k=='\n') {
      loraBuffer[bufferPos-1] = '\0';
      Serial.println(loraBuffer);
      parseData();
      bufferPos = 0;
    }
  }  
}

void parseData() {
  int index=0;
  char *token = strtok(loraBuffer, dataSep);

  while (token != NULL) {
    switch (index) {
      case 2: // temperature
        strncpy(temp, token, sizeof(temp)-1);
        break;
      case 3: // pressure
        strncpy(pressure, token, sizeof(pressure)-1);
        break;
      case 4: // altitude
        strncpy(altitude, token, sizeof(altitude)-1);
        break;
      case 5: // latitude
        strncpy(lat, token, sizeof(lat)-1);
        break;
      case 6: // longitude
        strncpy(lng, token, sizeof(lng)-1);
        break;
      
    }
    index++;
    token = strtok(NULL, dataSep);
  }

}