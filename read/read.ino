#include <SPI.h>
#include <mcp2515.h>

struct can_frame rawCanMsg;
struct obd2_frame {
    unsigned long can_id;
    byte dlc;
    byte mode;
    byte PID;
    byte data[4];
};
byte OBD2_Msg[10];

MCP2515 mcp2515(10);

const byte PIDs_OBD2[] = { 0x0C, 0x0D, 0x0A};
const byte size_PIDs = 3;
byte iPID = 0;
const byte interruptPin = 2;
unsigned long time = 0;
const byte sizeBuff = 128;
byte CanMsg[16];
byte buf[sizeBuff];
byte* pserial = buf;
byte* psave = buf;

const byte opMode = 1; // :)
const unsigned long spID = 0x0C;

void setup() {
  Serial.begin(115200);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  pinMode(interruptPin, INPUT_PULLUP);
  CanMsg[14] = 13;
  CanMsg[15] = 10;




  switch (opMode) {
    case 1: // sniffer
    attachInterrupt(digitalPinToInterrupt(interruptPin), sniffer, LOW);
    break;

    case 2: // specfic id
    attachInterrupt(digitalPinToInterrupt(interruptPin), specID, LOW);
    break;

    case 3: // OBD2 PID
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;

    case 4: // combined
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), comined, LOW);
    break;

    case 5: // OBD2 DTC
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;

    case 6: // Borrar DTC
    pit();
    attachInterrupt(digitalPinToInterrupt(interruptPin), obd2, LOW);
    break;
  }
}

void loop() {
  byte* temp8 = buf+sizeBuff;

  while(pserial != psave){
    Serial.write(*pserial);
    pserial++;
    if(pserial >= buf+sizeBuff){
      pserial = buf;
    }
  }
}

ISR(TIMER2_OVF_vect){ //funcion del PIT cada 32.64ms
  mcp2515.sendMessage(&OBD2_Msg);
  Serial.print("Request enviado con PID: ");
  Serial.println(PIDs_OBD2[iPID]);
  iPID++;
  if(iPID == size_PIDs){
    iPID = 0;
  }
  OBD2_Msg.PID = PIDs_OBD2[iPID];
}

void pit() {
  SREG = (SREG & 0b01111111);//deshabilitar interrupciones globales
  TCNT2 = 0; // limpiar registro de la cuenta del timer-2
  TIMSK2 = TIMSK2 | 0b00000001; //habilita interrupcion por Overflow
  TCCR2B = 0b00000111; // reloj a 7812.5 Hz
  SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones globales
  OBD2_Msg.can_id = 0x7DF;

  if (opMode == 5) /*DTC*/ {
    OBD2_Msg.dlc = 0x01;
    OBD2_Msg.mode = 0x03;
    OBD2_Msg.PID = 0x00;
  } else if(opMode == 6) /*delete DTC*/ {
    OBD2_Msg.dlc = 0x01;
    OBD2_Msg.mode = 0x04;
    OBD2_Msg.PID = 0x00;
  }else /*PID*/{
    OBD2_Msg.dlc = 0x02;
    OBD2_Msg.mode = 0x01;
    OBD2_Msg.PID = PIDs_OBD2[iPID];
  }

  OBD2_Msg.data[0] = 0x00;
  OBD2_Msg.data[1] = 0x00;
  OBD2_Msg.data[2] = 0x00;
  OBD2_Msg.data[3] = 0x00;
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
}

void writeBuff(){
  for(byte i = 0; i <= 15; i++){
    *psave = CanMsg[i];
    psave++;
  }
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
  if (rawCanMsg.can_id == spID) {
    writeBuff();
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
  if (rawCanMsg.can_id == 0x7E8 || rawCanMsg.can_id == spID) {
    writeBuff();
  }
}
