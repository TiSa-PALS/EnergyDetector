#ifndef ARD_ERRORTOPICS_H
#define ARD_ERRORTOPICS_H

#include "iTopics.h"

class ErrorTopics : protected ITopics {

public:
    ErrorTopics() : ITopics() {};

public:
    char error[12];
public:
    ErrorTopics(int8_t id) : ITopics() {
        /*
            error
        */
        this->createTopic(this->error, id, "/E");
    };
};

#endif
