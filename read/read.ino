#include <SPI.h>
#include <mcp2515.h>

struct can_frame rawCanMsg;
struct can_frame OBD2_Msg;
MCP2515 mcp2515(10);
const byte PIDs_OBD2[] = { 0x0C, 0x0D, 0x0A};
const byte size_PIDs = 3;
byte iPID = 0;
const byte interruptPin = 2;
unsigned long time = 0;
const byte sizeBuff = 128;
byte buf[sizeBuff];
byte* pserial = buf;
byte* psave = buf;
const byte opMode = 1;
const byte spID_size = 3;
const byte spID[] = {0x740, 0x63D, 0x646};

void setup() {
  Serial.begin(115200);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  pinMode(interruptPin, INPUT_PULLUP);

  switch (opMode) {
    case 1: // sniffer
    attachInterrupt(digitalPinToInterrupt(interruptPin), sniffer, LOW);
    break;
    case 2: // specific id
    attachInterrupt(digitalPinToInterrupt(interruptPin), specID, LOW);
    break;
    case 3: // OBD2 PID
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;
    case 4: // combined
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), combined, LOW);
    break;
    case 5: // OBD2 DTC
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;
    case 6: // OBD2 Delete DTC
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;
  }
}

void loop() {
  while(pserial != psave){
    Serial.write(*pserial);
    pserial++;
    if(pserial >= buf+sizeBuff){
      pserial = buf;
    }
  }
}

ISR(TIMER2_OVF_vect){ //PIT function every 32.64ms
  mcp2515.sendMessage(&OBD2_Msg);
  iPID++;
  iPID = iPID % size_PIDs;
  OBD2_Msg.data[2] = PIDs_OBD2[iPID];
}

void pit() {
  SREG = (SREG & 0b01111111);//Global interruption OFF
  TCNT2 = 0; // celan timer-2 register
  TIMSK2 = TIMSK2 | 0b00000001; //Overflow interruption
  TCCR2B = 0b00000111; // clock at 7812.5 Hz
  SREG = (SREG & 0b01111111) | 0b10000000; //Global interruption ON
  OBD2_Msg.can_id = 0x7DF;

  if (opMode == 5) /*DTC*/ {
    OBD2_Msg.data[0] = 0x01; //dlc
    OBD2_Msg.data[1] = 0x03; //mode
    OBD2_Msg.data[2] = 0x00; //PID
  } else if(opMode == 6) /*delete DTC*/ {
    OBD2_Msg.data[0] = 0x01; //dlc
    OBD2_Msg.data[1] = 0x04; //mode
    OBD2_Msg.data[2] = 0x00; //PID
  }else /*PID*/{
    OBD2_Msg.data[0] = 0x02; //dlc
    OBD2_Msg.data[1] = 0x01; //mode
    OBD2_Msg.data[2] = PIDs_OBD2[iPID]; // PID (iPID starts at 0)
  }

  OBD2_Msg.data[3] = 0x00;
  OBD2_Msg.data[4] = 0x00;
  OBD2_Msg.data[5] = 0x00;
  OBD2_Msg.data[6] = 0x00;
  OBD2_Msg.data[6] = 0x00;
}

void readMCP() {
  byte temp8 = 0;
  temp8 = psave-pserial;
  temp8 = temp8 & 0x7F;
  if(temp8 > 115){
    Serial.println("Overflow");
    delay(5000);
    return;
  }
  time = millis();
  mcp2515.readMessage(&rawCanMsg);

}

void writeBuff(){

  *psave = time>>24;
  psave++;
  *psave = time>>16;
  psave++;
  *psave = time>>8;
  psave++;
  *psave = time;
  psave++;
  *psave = rawCanMsg.can_id >> 8;
  psave++;
  *psave = rawCanMsg.can_id;
  psave++;
  *psave = rawCanMsg.data[0];
  psave++;
  *psave = rawCanMsg.data[1];
  psave++;
  *psave = rawCanMsg.data[2];
  psave++;
  *psave = rawCanMsg.data[3];
  psave++;
  *psave = rawCanMsg.data[4];
  psave++;
  *psave = rawCanMsg.data[5];
  psave++;
  *psave = rawCanMsg.data[6];
  psave++;
  *psave = rawCanMsg.data[7];
  psave++;
  *psave = 13;
  psave++;
  *psave = 10;
  psave++;

  if(psave >= buf+sizeBuff){
    psave = buf;
  }
}

void sniffer() {
  readMCP();
  writeBuff();
}

void specID() {
  readMCP();
  for(byte k = 0; k<spID_size; k++){
      if(rawCanMsg.can_id == spID[k]){
      writeBuff();
    }
  }
}

void obd2() {
  readMCP();
  if (rawCanMsg.can_id == 0x7E8) {
    writeBuff();
  }
}

void combined() {
  readMCP();
  if (rawCanMsg.can_id == 0x7E8){
    for(byte k = 0; k<spID_size; k++){
      if(rawCanMsg.can_id == spID[k]){
          writeBuff();
      }
    }
  }
}
