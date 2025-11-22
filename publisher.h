#ifndef PUBLISHER_H
#define PUBLISHER_H


extern MQTTAsync clientPub; // cliente MQTT do publisher
extern volatile int finished; // termina a execução do programa

// callbacks do publisher
void onConnectFailure(void *context, MQTTAsync_failureData *response);
void onSend(void *context, MQTTAsync_successData *response);
void onConnect(void *context, MQTTAsync_successData *response);
void connlost(void *context, char *cause);

// inicializa o publisher
int initPublisher(MQTTAsync client);

#endif
