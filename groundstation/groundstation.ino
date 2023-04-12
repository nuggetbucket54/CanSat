#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

 


SoftwareSerial lora(9, 8);
LiquidCrystal_I2C lcd(0x27,16,2);  

char loraBuffer[80];
int bufferPos = 0;
int clock = 0;
char temp[7], pressure[10], altitude[10];
char lat[13], lng[13]; // 1 byte for sign/3 for degree/8 for precision
const char dataSep[] = ",";


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(11, OUTPUT);
  lora.begin(38400);
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  

}


void loop()
{
  smartDelay(500);
  Serial.print(F("Temperature: "));
  Serial.print(temp);
  Serial.println(" *C");

  Serial.print(F("Pressure: "));
  Serial.print(pressure);
  Serial.println(" Pa");

  Serial.print(F("Approx altitude: "));
  Serial.println(altitude); 

  Serial.print(F("Latitude: "));
  Serial.println(lat); 
  Serial.print(F("Longitude: "));
  Serial.println(lng); 

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(temp);
  
  lcd.setCursor(0, 1);
  lcd.print(pressure);

  

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