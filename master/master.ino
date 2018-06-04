#include <FastCRC.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <Dhcp.h>
#include <Dns.h>
#include <PubSubClient.h>

// podmienený preklad
//#define DEBUG_MODE=1

FastCRC8 CRC8;
/**
 * must be unique for cleint
 */
const char* clientName = "arduinoClientEne";
const int CommDelay_us = 360;
const char Delimiter = 44;
uint8_t bufRecCRC[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                      };
uint8_t bufSendCRC[] = {0x32, 0x05, 0x01, 0x0D};
uint8_t bufSendCRC2[] = {0x32, 0x05, 0x01, 0x0D};
byte recCRC = 0;
byte recCRC2 = 0;
unsigned long shotNum = 0;
byte byteReceived;
byte byteMasterReceived;
byte byteAddressReceived;
byte byteSend;
byte byteSendData;
byte byteAddrSlave;
const byte SlaveCount = 8;
byte AddrSlave = 0;
int Ptr = 48;
int NextState = 0;
int Timeout = 0;
int Value = 980;
byte messLen = 0;
byte dataCount = 0;
byte dataSts = 0;
word dataSlaves[SlaveCount + 1][10];
byte recMessage[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0
                    };
byte RecBuf[70];
//byte RecBuf[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // buffer for received data from Slaves (changes every loop)
byte RecBuf2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // buffer with old received data, used for check if data are changed
unsigned long RecBuf3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0
                          }; // buffer with old received data, used for check if data are changed
byte SendBuf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0
                 }; // buffer with data send to Slaves
byte SendBuf2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0
                  }; // buffer with data send to Slaves
bool RecNew = 0; // new data from slave indicator
int DebugSend = 0;
bool configSend = 0;
bool configSend2 = 0;
int DebugBuffer;
byte addrSlaveCfg = 0;
byte modeSlaveCfg = 0;
byte gainSlaveCfg = 0;
byte bckSlaveCfg = 0;
byte diffSlaveCfg = 0;
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long loopLen = 0;


int Dhcp = 0;
// the media access control (ethernet hardware) address for the shield:
byte mac[] = {0xDA, 0xAD, 0xBE, 0xC8, 0x1A, 0xE4};
//the IP address for the shield:
byte ip[] = {192, 168, 95, 154};
// the router's gateway address:
byte gateway[] = {192, 168, 95, 1};
// the subnet:
byte subnet[] = {255, 255, 255, 0};
int ClientCon = 0;
int delka = 0;
char Command[2]; //command that comes from control
byte Buffer[] = {0, 0, 0, 0, 0, 0}; //numerical that comes from control
byte BufferTmp[] = {0x32, 0x02, 0x00, 0x0D};
String stringS = ""; //string from ethernet
int LastInterface = 0;
bool LastDataLenght = 0;
unsigned long shotNumbers[9];

EthernetClient ethClient;
IPAddress server(192, 168, 95, 8);

#include <PubSubClient.h>

PubSubClient client(ethClient);

char* stringToChar(String&& s) {
  char ch[50];
  s.toCharArray(ch, 50);
  return ch;
}

void sendOnce(char topic[12], char* data) {
#ifdef DEBUG_MODE
  Serial.write(topic);
  Serial.print(":");
  Serial.write(data);
  Serial.println();
#endif

  client.publish(topic, data);
}

const uint8_t dataLength = 10;

void sendData(char topic[12], word d[10], int8_t i) {
  String value = "";
  char copy[70];
  bool hasData = false;
  for (int i = 0; i < dataLength; i++) {
    if (d[i] == 0) {
      continue;
    } else {
      hasData = true;
    }
    value += String(d[i]);
    value += ",";
  }
  if (!hasData) {
    return;
  }
  value.toCharArray(copy, 70);
  sendOnce(topic, copy);
};

String value = "";

/* load až tu musí existovať context*/
#include "SimpleMQTTClient.h";
DataBuffer* dataBuffer[8] = {
  new DataBuffer(0),
  new DataBuffer(1),
  new DataBuffer(2),
  new DataBuffer(3),
  new DataBuffer(4),
  new DataBuffer(5),
  new DataBuffer(6),
  new DataBuffer(7)
};

int CommandSel() {
  switch (Command[0]) {
    case 'd':
      DebugSend = 1;
      break;
    case 'm':
      addrSlaveCfg = Buffer[0];
      modeSlaveCfg = Buffer[1];
      bufSendCRC[0] = addrSlaveCfg;
      bufSendCRC[1] = 0x06;
      bufSendCRC[2] = modeSlaveCfg;
      bufSendCRC[3] = 0x0D;
      configSend = 1;
      Serial.println("ACK");
      break;
    case 'g':
      addrSlaveCfg = Buffer[0];
      gainSlaveCfg = Buffer[1];
      bufSendCRC[0] = addrSlaveCfg;
      bufSendCRC[1] = 0x01;
      bufSendCRC[2] = gainSlaveCfg;
      bufSendCRC[3] = 0x0D;
      configSend = 1;
      Serial.println("ACK");
      break;
    case 'b':
      addrSlaveCfg = Buffer[0];
      bckSlaveCfg = Buffer[1];
      bufSendCRC[0] = addrSlaveCfg;
      bufSendCRC[1] = 0x02;
      bufSendCRC[2] = bckSlaveCfg;
      bufSendCRC[3] = 0x0D;
      configSend = 1;
      Serial.println("ACK");
      break;
    case 's':
      addrSlaveCfg = Buffer[0];
      diffSlaveCfg = Buffer[1];
      bufSendCRC[0] = addrSlaveCfg;
      bufSendCRC[1] = 0x03;
      bufSendCRC[2] = diffSlaveCfg;
      bufSendCRC[3] = 0x0D;
      configSend = 1;
      Serial.println("ACK");
      break;
    default:
      break;
  }
}

void sendSlaveCmd() {
  for (unsigned int i = 0; i < 4; i++) {
    delayMicroseconds(CommDelay_us);
    Serial1.write(bufSendCRC[i]);
  }
  delayMicroseconds(CommDelay_us);
  Serial1.write(CRC8.smbus(bufSendCRC, 4));
}

void sendSlaveCmd2() {
  for (unsigned int i = 0; i < 4; i++) {
    delayMicroseconds(CommDelay_us);
    Serial1.write(bufSendCRC2[i]);
    Serial.write(bufSendCRC2[i]);
  }
  delayMicroseconds(CommDelay_us);
  Serial1.write(CRC8.smbus(bufSendCRC2, 4));
  Serial.write(CRC8.smbus(bufSendCRC2, 4));
}

void SerialEvent() {
  Command[0] = "";
  Serial.readBytes(Command, 1);
  delayMicroseconds(100);
  for (int i = 0; i < 4; i++) {
    Buffer[i] = 0;
    Buffer[i] = Serial.read();
    delayMicroseconds(100);
  }
  LastInterface = 0;
  CommandSel();
}

const char* inTopic = "Ene/I/C";

void callback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, inTopic) == 0) {

    char charMsg[length];
    for (int i = 0; i < length; i++) {
      charMsg[i] = (char)payload[i];
    }
    String msg = String(charMsg);
    int l =  msg.indexOf(" ");
    char type[2];
    msg.substring(0, l).toCharArray(type, 2);
    msg = msg.substring(l + 1);

    l =  msg.indexOf(" ");
    int address = msg.substring(0, l).toInt();
    int com = msg.substring(l + 1).toInt();
    //#ifdef DEBUG_MODE
    Serial.println(type);
    Serial.println(address);
    Serial.println(com);
    //#endif

    switch ((char)type[0]) {
      case 'm':
        // nastavuje mode
        bufSendCRC2[1] = 0x06;
        break;
      case 'g':
        // nastavuje gain
        bufSendCRC2[1] = 0x01;
        break;
      case 'b':
        // nastavuje background
        bufSendCRC2[1] = 0x02;
        break;
      case 's':
        bufSendCRC2[1] = 0x03;
        break;
      case 'd':
        // debbug vypíše všetky dáta
        return dataBuffer[address]->sendAll();
      default:
        break;
    }
    bufSendCRC2[0] = (0x30) + address;
    bufSendCRC2[2] = com;
    bufSendCRC2[3] = 0x0D;
    configSend2 = 1;
    Serial.println("ACK");
  }
  //#ifdef DEBUG_MODE
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  //#endif
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientEne")) {
      Serial.println("OK");
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
};

void setup() { /****** SETUP: RUNS ONCE ******/
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  Serial1.begin(38400); // set the data rate
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial1.setTimeout(5);
  Serial.setTimeout(5);
  Serial2.setTimeout(5);
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);

  client.setServer(server, 1883);
  client.setCallback(callback);

  bufSendCRC[0] = 0x31;
  bufSendCRC[1] = 0x06;
  bufSendCRC[2] = 0x02;
  bufSendCRC[3] = 0x0D;
  sendSlaveCmd();
  Ethernet.begin(mac, ip);
}

char* d[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
unsigned long lastLC = 0;
bool forceSend = false;

void loop() {
  if(!client.loop()){
    Serial.println("-- LF ---;");
    reconnect();
  }
  
  //

  timer2 = micros();
  if ( (timer2 - lastLC) > 10000000) {
    lastLC = timer2;
    int r = random(0, 9);
    sendOnce("Ene/O/LC", d[r]);
    forceSend = true;
  } else {
    forceSend = false;
  }

  //********* Address overflow reset and data handling **************

  if ((AddrSlave) > (SlaveCount - 1)) {
    //    DebugSend = 1;
    if (configSend) {
      sendSlaveCmd();
      configSend = 0;
    }
    if (configSend2) {
      sendSlaveCmd2();
      configSend2 = 0;
    }
#ifdef DEBUG_MODE
    Serial.print(loopLen);
    Serial.print(" ");
    Serial.println(millis() - timer1);
#endif
    timer1 = millis();
    if (DebugSend == 1) {
      for (int i = 0; i < SlaveCount; i++) {
        for (int y = 0; y < 10; y++) {
          Serial.print(dataSlaves[i][y]);
          Serial.print(" ");
        }
        Serial.print(": ");
        Serial.print(RecBuf[(i * 6) + 0]);
        Serial.print(" ");
        Serial.print(RecBuf[(i * 6) + 1]);
        Serial.print(" ");
        Serial.print(RecBuf[(i * 6) + 2]);
        Serial.print(" ");
        Serial.print(RecBuf[(i * 6) + 3]);
        Serial.print(" ");
        Serial.print((RecBuf[(i * 6) + 4]) * 100);
        Serial.print(" ");
        Serial.print((RecBuf[(i * 6) + 5]) * 100);
        Serial.print(" ");
        Serial.print(RecBuf2[i]);
        Serial.print(" ");
        Serial.print(RecBuf3[i]);
        Serial.println();
      }
      Serial.println();
      DebugSend = 0;
    }
    if (DebugSend == 2) {
      NextState = 2;
      DebugSend = 0;
      Timeout = 0;
    }
    for (int i = 0; i < SlaveCount; i++) {
      if (SendBuf2[i] > 0) {
        SendBuf[i] = SendBuf2[i];
        SendBuf2[i] = 0;
      } else SendBuf[i] = 0;
    }
    for (int i = 0; i < sizeof (RecBuf); i++) {
      RecBuf[i] = 0;
    }
    AddrSlave = 0;
    Ptr = 48;
    delay(10);
  }

  //////////// COMMUNICATION WITH SLAVES ////////////////
  // Runs continuously in loop with cca 15 ms tick //////

  //********* Send request to actual slave **************

  if (NextState == 0) {
    bufSendCRC[0] = (byte(Ptr));
    bufSendCRC[1] = 0x05;
    bufSendCRC[2] = 0x01;
    bufSendCRC[3] = 0x0D;
    sendSlaveCmd();
    NextState = 1;
  }

  //********* Wait for answer from actual slave **************

  if (NextState == 1) {
    if (Serial1.available()) //Look for data from slave
    {
      byteSend = Serial1.read(); // Read received byte 1
      bufRecCRC[0] = byteSend;
      if (byteSend == (Ptr)) // Slave always start answer with 254 value
      {
        delayMicroseconds(CommDelay_us);
        messLen = Serial1.read(); // Read the byte 2 (slave address)
        bufRecCRC[1] = messLen;
        delayMicroseconds(CommDelay_us);
        if ((messLen > 8) && (messLen < 40)) {
          for (int i = 0; i < (messLen - 1); i++) {
            bufRecCRC[i + 2] = Serial1.read(); // Read the byte 3 (slave data)
            delayMicroseconds(CommDelay_us);
          }
        }
        if (Serial1.read() == 13) {
          bufRecCRC[messLen + 1] = 13;
        }
        delayMicroseconds(CommDelay_us);
        recCRC = Serial1.read();
        //      delayMicroseconds(CommDelay_us);
        //      recCRC2 = Serial1.read();
        if ((CRC8.smbus(bufRecCRC, (messLen + 2))) == recCRC) {
          for (int i = 0; i < 10; i++) {
            dataSlaves[AddrSlave][i] = 0;
          }
          for (int i = 0; i < 10; i++) {
            word result = 0;
            result = (bufRecCRC[(i * 2) + 8] * 256);
            result = result + bufRecCRC[(i * 2) + 9];
            dataSlaves[AddrSlave][i] = result;
            // Serial.println( dataSlaves[AddrSlave][i]);
          }
          RecBuf[(AddrSlave * 6) + 0] = bufRecCRC[2];
          RecBuf[(AddrSlave * 6) + 1] = bufRecCRC[3];
          RecBuf[(AddrSlave * 6) + 2] = bufRecCRC[4];
          RecBuf[(AddrSlave * 6) + 3] = bufRecCRC[5];
          RecBuf[(AddrSlave * 6) + 4] = bufRecCRC[6];
          RecBuf[(AddrSlave * 6) + 5] = bufRecCRC[7];
          shotNum = 0;
          for (int i = 4; i > 0; i--) {
            shotNum = (shotNum | bufRecCRC[i + messLen - 4]);
            shotNum = shotNum << 8;
          }
          shotNum = shotNum >> 8;
          RecBuf3[AddrSlave] = shotNum;
          bufSendCRC[0] = (byte(Ptr));
          bufSendCRC[1] = 0x04;
          bufSendCRC[2] = 0x01;
          bufSendCRC[3] = 0x0D;
          sendSlaveCmd();
        }
        if (dataBuffer[AddrSlave]->shotNumber != shotNum) {

          dataBuffer[AddrSlave]->updateShotNumber(shotNum);
          dataBuffer[AddrSlave]->updateError(0, bufRecCRC[5], bufRecCRC[4]);
          dataBuffer[AddrSlave]->updateGain(bufRecCRC[2]);
          dataBuffer[AddrSlave]->updateMode(bufRecCRC[3]);
          dataBuffer[AddrSlave]->updateBackground(bufRecCRC[6]);
          dataBuffer[AddrSlave]->updateDiff(bufRecCRC[7]);
          dataBuffer[AddrSlave]->updateData(dataSlaves[AddrSlave]);
#ifdef DEBUG_MODE
          Serial.println("--DS--");
          for (int i = 0; i < 10; i++) {
            Serial.println(dataSlaves[AddrSlave][i]);
          }
#endif
        }
        for (int i = 0; i < sizeof (bufRecCRC); i++) {
          bufRecCRC[i] = 0;
        }

        Timeout = 0;
        RecBuf2[AddrSlave] = 0;
        NextState = 2;
        AddrSlave++; // next slave address pointer
        Ptr++; // next slave address value
      }
    }

    Timeout++; // Slave answer timeout counter
    if (Timeout > 1000) {
      Timeout = 0;
      NextState = 2;
      shotNum = 0;
      dataBuffer[AddrSlave]->updateError(0, 1, 0);
      RecBuf2[AddrSlave] = 255;
      dataSts = 0;
      AddrSlave++;
      Ptr++;
      delayMicroseconds(CommDelay_us);
    }
  }

  if (NextState == 2) {
#ifdef DEBUG_MODE
    Serial.println(AddrSlave);
    Serial.print("SN: ");
    Serial.println(shotNumbers[AddrSlave]);
    Serial.print("SN: ");
    Serial.println(shotNum);
#endif
    NextState = 0;
  }

  //////////// COMMUNICATION WITH REMOTE PC ///////////////
  //  EthernetRead();

  if (Serial.available() > 0) { //look for serial client
    SerialEvent();
  }
  loopLen = micros() - timer2;
}

