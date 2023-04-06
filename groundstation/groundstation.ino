
#include <SoftwareSerial.h>
 
SoftwareSerial lora(9, 8);

char loraBuffer[80];
int bufferPos = 0;
int clock = 0;
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
      bufferPos = 0;
    }
  

  }  

}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    flushLora();
  } while (millis() - start < ms);
}