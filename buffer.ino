#include <SPI.h>
#include <mcp2515.h>

struct can_frame rawCanMsg;
MCP2515 mcp2515(10);
byte CanMsg[12];
const int interruptPin = 2; //Donde se conecta la interruption


byte buf[120];
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
  CanMsg[10] = 13;
  CanMsg[11] = 10;
}

void loop() {
  
  while(pserial != psave){

    Serial.write(*pserial);
    pserial++;
    if(pserial >= buf+120){
      pserial = buf;
    }
  }
}



void readCAN (){


  if(psave < pserial){
    return;
  }
  
  mcp2515.readMessage(&rawCanMsg);
  CanMsg[0] = rawCanMsg.can_id >> 8;
  CanMsg[1] = rawCanMsg.can_id;
  CanMsg[2] = rawCanMsg.data[0];
  CanMsg[3] = rawCanMsg.data[1];
  CanMsg[4] = rawCanMsg.data[2];
  CanMsg[5] = rawCanMsg.data[3];
  CanMsg[6] = rawCanMsg.data[4];
  CanMsg[7] = rawCanMsg.data[5];
  CanMsg[8] = rawCanMsg.data[6];
  CanMsg[9] = rawCanMsg.data[7];

  for(byte i = 0; i <= 11; i++){
    *psave = CanMsg[i];
    psave++;
  }

  if(psave >= buf+120){
    psave = buf;
  }
}
