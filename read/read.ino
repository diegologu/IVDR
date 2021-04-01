#include <SPI.h>
#include <mcp2515.h>

struct can_frame rawCanMsg;
MCP2515 mcp2515(10);
byte CanMsg[16];
const int interruptPin = 2; //Pin where the INT is connected

unsigned long time = 0;


byte buf[160];
byte* pserial = buf;
byte* psave = buf;

void setup() {
  Serial.begin(115200);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  //mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), readCAN, LOW);
  CanMsg[14] = 13;
  CanMsg[15] = 10;
}

void loop() {

  while(pserial != psave){

    Serial.write(*pserial);
    pserial++;
    if(pserial >= buf+160){
      pserial = buf;
    }
  }
}



void readCAN (){


  if(psave < pserial){
    return;
  }

  time = millis();
  mcp2515.readMessage(&rawCanMsg);

  CanMsg[0] = time>>24;
  CanMsg[1] = time>>16;
  CanMsg[2] = time>>8;
  CanMsg[3] = time;
  CanMsg[4] = rawCanMsg.can_id >> 8;
  CanMsg[5] = rawCanMsg.can_id;
  CanMsg[6] = rawCanMsg.data[0];
  CanMsg[7] = rawCanMsg.data[1];
  CanMsg[8] = rawCanMsg.data[2];
  CanMsg[9] = rawCanMsg.data[3];
  CanMsg[10] = rawCanMsg.data[4];
  CanMsg[11] = rawCanMsg.data[5];
  CanMsg[12] = rawCanMsg.data[6];
  CanMsg[13] = rawCanMsg.data[7];

  for(byte i = 0; i <= 15; i++){
    *psave = CanMsg[i];
    psave++;
  }

  if(psave >= buf+160){
    psave = buf;
  }
}
