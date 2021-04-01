#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg1;
struct can_frame canMsg2;
struct can_frame canMsg3;
MCP2515 mcp2515(10);


void setup() {
  canMsg1.can_id  = 0x740; //@
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x41; //A
  canMsg1.data[1] = 0x42; //B
  canMsg1.data[2] = 0x43; //C
  canMsg1.data[3] = 0x44; //D
  canMsg1.data[4] = 0x45; //E
  canMsg1.data[5] = 0x46; //F
  canMsg1.data[6] = 0x47; //G
  canMsg1.data[7] = 0x48; //H

  canMsg2.can_id  = 0x63D; //=
  canMsg2.can_dlc = 8;
  canMsg2.data[0] = 0x61; //a
  canMsg2.data[1] = 0x62; //b
  canMsg2.data[2] = 0x63; //c
  canMsg2.data[3] = 0x64; //d
  canMsg2.data[4] = 0x65; //e
  canMsg2.data[5] = 0x66; //f
  canMsg2.data[6] = 0x67; //g
  canMsg2.data[7] = 0x68; //h

  canMsg3.can_id  = 0x724; //$
  canMsg3.can_dlc = 8;
  canMsg3.data[0] = 0x30; //0
  canMsg3.data[1] = 0x31; //1
  canMsg3.data[2] = 0x32; //2
  canMsg3.data[3] = 0x33; //3
  canMsg3.data[4] = 0x34; //4
  canMsg3.data[5] = 0x35; //5
  canMsg3.data[6] = 0x36; //6
  canMsg3.data[7] = 0x37; //7
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  //mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

}

void loop() {
  mcp2515.sendMessage(&canMsg1);
  delay(50);
  mcp2515.sendMessage(&canMsg2);
  delay(50);
  mcp2515.sendMessage(&canMsg3);
  delay(50);
  
}
