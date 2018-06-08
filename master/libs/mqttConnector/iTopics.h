#ifndef ARD_ITOPICS_H
#define ARD_ITOPICS_H

class ITopics {
protected:
    void createTopic(char *topic, uint8_t id, char sulfix[3]) {
        String tmp = "";
        tmp.concat(this->getTopicPrefix());
        tmp.concat(id);
        tmp.concat(sulfix);
        tmp.toCharArray(topic, 15);
    };
public:
    virtual char *getTopicPrefix()=0;
};

#endif //ARD_ITOPICS_H
