#ifndef ARD_IDATABUFFER_H
#define ARD_IDATABUFFER_H

class IDataBuffer {
protected:
    unsigned long lastUpdated;
    unsigned long sendDelay;
public:
    bool canForceSend(unsigned long timer) {
        bool f = (timer - this->lastUpdated) >= this->sendDelay;
        if (f) {
            this->lastUpdated = timer;
        }
        return f;
    }

protected:
    static char *stringToChar(String &&s) {
        char ch[50];
        memset(ch, 0, sizeof(ch));
        s.toCharArray(ch, 50);
        return ch;
    }

protected:
    static void sendOnce(char topic[12], char *data) {

        //#ifdef DEBUG_MODE
        Serial.write(topic);
        Serial.print(":");
        Serial.write(data);
        Serial.println();
        //#endif
        client.publish(topic, data);
    }

protected:
    static char *printDouble(double val, unsigned int precision) {
        int frac;
        if (val >= 0) {
            frac = (val - int(val)) * precision;
        } else {
            frac = (int(val) - val) * precision;
        }
        String s = String(int(val));
        s.concat(".");
        s.concat(frac);
        char ch[50];
        memset(ch, 0, sizeof(ch));
        s.toCharArray(ch, 50);
        return ch;
    };
};
#endif
