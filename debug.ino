#include "stdio.h"

// void afterInterrupt();

void setup() {  
  sendSkCommands();

  pinMode(2, INPUT);
  pinMode(13, OUTPUT);
  pinMode(6, OUTPUT);
  
  // attachInterrupt(0, afterInterrupt, RISING);
}

void loop() {
  // 10秒間sleep
  sleep(10000);
  
  digitalWrite(6, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
  delay(1000);
  digitalWrite(6, HIGH);
  delay(1000);

  sendSkCommands();

  Serial.print("SKSENDTO 1 FE80:0000:0000:0000:1034:5678:ABCD:EF01 0E1A 0 0005 ");
  Serial.println("00000");
  
  Serial.println("SKDSLEEP");  
}

void sendSkCommands() {
  Serial.begin(115200); 
  
  Serial.println("SKSREG S1 12345678abcdef08");
  delay(100);

  Serial.println("SKSREG S2 21");
  delay(100);
    
  Serial.println("SKSREG S3 8888");
  delay(100);
}

//void afterInterrupt() {    
//
//}

