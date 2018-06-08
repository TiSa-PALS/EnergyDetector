

#ifndef ARD_ERRORBUFFER_H
#define ARD_ERRORBUFFER_H

#include "iDataBuffer.h"
#include "errorTopics.h"

class ErrorDataBuffer : public IDataBuffer {
private:
    ErrorTopics *topics;
    int errorCode;
public:
    ErrorDataBuffer() : IDataBuffer() {};

private:
    int errorToCode(char type) {
        switch (type) {
            case 'w':
                return 3;
            case 'u':
                return 2;
            case 't':
                return 1;
            case 'o':
                return 0;
        }
    }

public:
    void setErrorByType(char type, bool forceSend = false) {
        int numCode = this->errorToCode(type);
        this->setErrorByCode(numCode, forceSend);
    };
public:
    void setErrorByCode(int code, bool forceSend = false) {
        if (this->errorCode != code || forceSend) {
            this->errorCode = code;
            switch (this->errorCode) {
                case 3:
                    this->sendOnce(this->topics->error, "3-OVERFLOW");
                    break;
                case 2:
                    this->sendOnce(this->topics->error, "2-UNDEFINED");
                    break;
                case 1:
                    this->sendOnce(this->topics->error, "1-TIMEOUT");
                    break;
                case 0:
                    this->sendOnce(this->topics->error, "0-OK");
                    break;
            }
        }
    };

};

#endif //ARD_ERRORBUFFER_H
