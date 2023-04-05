
#include <SoftwareSerial.h>
 
SoftwareSerial lora(8, 9);

char loraBuffer[40];
int bufferPos = 0;
int clock = 0;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(11, OUTPUT);
  lora.begin(115200);
  lora.print("\r\n"); // flush
  lora.listen();

}
 
void loop()
{
  lora.println("AT+SEND=0,5,HELLO");
  smartDelay(100);

}

void flushLora() {

  while (lora.available())
  {
    char k = char(lora.read());
    loraBuffer[bufferPos] = k;
    bufferPos ++;

    if (bufferPos == 40 || loraBuffer[bufferPos] == 0x0A) { // if too many chars, or newline
      Serial.println(loraBuffer);
      bufferPos=0;
      memset(loraBuffer, '*', sizeof(loraBuffer));
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