#include <stdio.h>
#include <unistd.h>
#include "MQTTAsync.h"
#include "influxdb.h"
#include "publisher.h"
#include "subscriber.h"

#define ADDRESS "tcp://tsb1.vps.tecnico.ulisboa.pt:1883"
#define CLIENTID_PUB "tsbpublisher"
#define CLIENTID_SUB "tsbsubscriber"

MQTTAsync clientPub;
MQTTAsync clientSub;

volatile int finished = 0; // flag de comunicação entre as callbacks e o loop principal.

int main()
{
    int rc; //return code

    // Inicializa InfluxDB
    initInfluxDB();

    // Cria e inicializa publisher
    rc = MQTTAsync_create(&clientPub, ADDRESS, CLIENTID_PUB, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create publisher, return code = %d\n", rc);
        return rc;
    }
    initPublisher(clientPub);

    // Cria e inicializa subscriber
    rc = MQTTAsync_create(&clientSub, ADDRESS, CLIENTID_SUB, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create publisher, return code = %d\n", rc);
        return rc;
    }
    initSubscriber(clientSub);

    
    while (finished == 0)
    {
        usleep(10000L); // pausa de 10ms para o loop não consumir 100% da cpu
    }

    // destrói clientes MQTT e fecha conexão com InfluxDB
    MQTTAsync_destroy(&clientPub);
    MQTTAsync_destroy(&clientSub);
    closeInfluxDB();

    return 0;
}
