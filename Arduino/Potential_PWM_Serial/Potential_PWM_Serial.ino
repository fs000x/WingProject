// Fast Send Serial Data Version
//
// Read PotentialMeter from A2
// Write D5 pin, PWM 0~179, by A2 map(0~1023, to 0~179)
// Write Serial PotentialMeter 'S' map(0~1023, to 0~255)  'E'
//   Ex) S000E, S111E, S999E
#include <Servo.h>

Servo SimonKESC;
int pot1Pin = A2;
void setup() {
  SimonKESC.attach(5);
   Serial.begin(9600);
}
void loop() {
  int pot1 = analogRead(pot1Pin);
  int mapData = map(pot1, 0, 1023, 0, 179);
  SimonKESC.write(mapData);

  // mapping 0~1023 to 0 ~ 255  
  int sendData1 = map(pot1, 0, 1023, 0, 255);
  int sendData2 = sendData1;
  int sendData3 = sendData1;
  char buffer[6];
  buffer[0] = 'S';
  buffer[1] = sendData1;
  buffer[2] = sendData2;
  buffer[3] = sendData3;
  int CRC = (buffer[1] + buffer[2] + buffer[3]) % 255;
  buffer[4] = CRC;
  buffer[5] = 'E';
  Serial.write(buffer,6);
  
  delay(20);
}

