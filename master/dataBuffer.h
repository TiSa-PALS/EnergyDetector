#include "libs/mqttConnector/errorTopics.h"
#include "libs/mqttConnector/errorBuffer.h"

class Topics : public ErrorTopics {
public:
    char gain[12];
    char mode[12];
    char background[12];
    char diff[12];
    char data[12];
    char shotNumber[12];
public:
    Topics(int8_t id) : ErrorTopics(id) {
        /*
          config
        */
        this->createTopic(this->gain, id, "/G");
        this->createTopic(this->mode, id, "/M");
        this->createTopic(this->background, id, "/B");
        this->createTopic(this->diff, id, "/DF");
        /*
          data
        */
        this->createTopic(this->shotNumber, id, "/S");
        this->createTopic(this->data, id, "/D");
    };
public:
    char* getTopicPrefix() {
        return "Ene/O/";
    };
};


class DataBuffer : public ErrorDataBuffer {
private:
    Topics *topics;
    word data[10];
    byte gain;
    byte mode;
    byte backgroung;
    byte diff;
public:
    unsigned long shotNumber = 0;
public:
    DataBuffer(uint8_t id) : ErrorDataBuffer() {
        this->sendDelay = 20000000;
        this->topics = new Topics(id);
    };


public:
    void updateShotNumber(byte sn, bool forceSend = false) {
        if (this->shotNumber != sn || forceSend) {
            Serial.println("Shot number updated");
            this->shotNumber = sn;
            sendOnce(this->topics->shotNumber, stringToChar(String(this->shotNumber)));
        }
    };

public:
    void updateData(word d[10], bool forceSend = false) {

        bool hasData = false;
        for (int i = 0; i < 10; i++) {
            if (d[i] != 0) {
                hasData = true;
            }
        }

        for (int i = 0; i < 10; i++) {
            this->data[i] = hasData ? d[i] : 0;
        }
        if (hasData || forceSend) {
            Serial.println("Data number updated");
            this->sendData();
        }
    };
public:
    void sendData() {
        String value = "";

        for (int i = 0; i < 10; i++) {
            if (this->data[i] == 0) {
                continue;
            }
            value += String(this->data[i]);
            value += ",";
        }
        char copy[50];
        memset(copy, 0, sizeof(copy));
        value.toCharArray(copy, 50);
        sendOnce(this->topics->data, copy);
    };
public:
    void updateGain(byte g, bool forceSend = false) {
        if (this->gain != g || forceSend) {
            this->gain = g;
            this->sendGain();
        }
    };
private:
    void sendGain() {
        sendOnce(this->topics->gain, stringToChar(String(this->gain)));
    };
public:
    void updateMode(byte m, bool forceSend = false) {
        if (this->mode != m || forceSend) {
            this->mode = m;
            this->sendMode();
        }
    };
private:
    void sendMode() {
        sendOnce(this->topics->mode, stringToChar(String(this->mode)));
    };
public:
    void updateBackground(byte b, bool forceSend = false) {
        if (this->backgroung != b || forceSend) {
            this->backgroung = b;
            this->sendBackground();
        }
    };
private:
    void sendBackground() {
        sendOnce(this->topics->background, stringToChar(String(this->backgroung)));
    };
public:
    void updateDiff(byte d, bool forceSend = false) {
        if (this->diff != diff || forceSend) {
            this->diff = d;
            this->sendDiff();
        }
    };
private:
    void sendDiff() {
        sendOnce(this->topics->diff, stringToChar(String(this->diff)));
    };
public:
    void sendAll() {
        this->sendDiff();
        this->sendBackground();
        this->sendMode();
        this->sendGain();
    };
    /**
     * TODO FIXME
     */
public:
    void updateError(word e = 0, byte t = 0, byte o = 0, bool forceSend = false) {
        if (o) {
            this->setErrorByType('w', forceSend);
        }
        if (e) {
            this->setErrorByCode('u', forceSend);
        }
        if (t) {
            this->setErrorByType('t', forceSend);
        }


    };
};
