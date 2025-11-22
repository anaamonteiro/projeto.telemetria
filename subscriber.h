#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

extern MQTTAsync clientSub;   // cliente MQTT do subscriber
extern volatile int finished; // termina a execução do programa

// callbacks do subscribe
void SubonConnectFailure(void *context, MQTTAsync_failureData *response);
void onSubscribe(void *context, MQTTAsync_successData *response);
void onSubscribeFailure(void *context, MQTTAsync_failureData *response);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void SubonConnect(void *context, MQTTAsync_successData *response);
void Subconnlost(void *context, char *cause);

// Inicializa o subscriber
int initSubscriber(MQTTAsync client);

#endif
