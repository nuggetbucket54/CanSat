
#include <SoftwareSerial.h>
 


SoftwareSerial lora(9, 8);

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

}


void loop()
{
  if(lora.available())
  {
    flushLora();
  }

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
      case 2:
        Serial.print(F("Temperature: "));
        break;
      case 3:
        Serial.print(F("Pressure: "));
        break;
    }
    index++;
    Serial.println(token);
    token = strtok(NULL, dataSep);
  }

}