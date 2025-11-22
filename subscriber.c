/*******************************************************************************
 * subscriber.c
 *
 * sumário: subscreve às mensagens emitidas pelo publisher ao broker
 *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTAsync.h"
#include "subscriber.h"
#include "influxdb.h"

#define USERNAME "newuser"          // user para autenticação no broker
#define PASSWORD "recrutamento2025" // password para autenticação
#define TOPIC "recruit/ana/#"       // subscrição de todos os sub-tópicos do usuário
#define QOS 1                       // Quality of Service 1: entrega pelo menos 1 vez

/*******************************************************************************
 * SubonConnectFailure
 *
 * param[in]: context  - contexto associado ao cliente MQTT clientSub
 * param[in]: response - ponteiro para estrutura MQTTAsync_failureData que contém
 *                       informação sobre o motivo da falha
 *
 * retorno: void
 *
 * sumário: callback executado quando o subscriber falha ao tentar estabelecer a ligação
 *          inicial ao broker MQTT.
 *
 *******************************************************************************/
void SubonConnectFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Connection to broker failed.\n");
    finished = 1;
}

/*******************************************************************************
 * onSubscribe
 *
 * param[in]: context - contexto associado ao cliente MQTT clientSub
 * param[in]: response - estrutura MQTTAsync_successData com dados sobre a ligação
 *
 * retorno: void
 *
 * sumário: callback que imprime mensagem quando a subscrição é bem sucedida
 *
 *****************************************************************************/
void onSubscribe(void *context, MQTTAsync_successData *response)
{
    printf("Subscribed to %s\n", TOPIC);
}

/*******************************************************************************
 * onSubscribeFailureFailure
 *
 * param[in]: context  - contexto associado ao cliente MQTT clientSub
 * param[in]: response - ponteiro para estrutura MQTTAsync_failureData que contém
 *                       informação sobre o motivo da falha
 *
 * retorno: void
 *
 * sumário: callback que imprime mensagem quando a subscrição não é bem sucedida
 *
 *******************************************************************************/
void onSubscribeFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Subscriber: Subscription failed.\n");
    finished = 1;
}

/*******************************************************************************
 * msgarrvd
 *
 * param[in]: context - contexto associado ao cliente MQTT clientSub
 * param[in]: topicName - string com o nome do tópico
 * param[in]: topicLen - comprimento do tópico
 * param[in]: message - estrutura que contém o payload recebido
 *
 * retorno: int - retorna 1 para indicar que a mensagem foi processada com
 *                sucesso. Qualquer outro valor indica falha.
 *
 * sumário: callback executada quando o subscriber recebe uma mensagem MQTT publicada
 *          num dos tópicos aos quais está subscrito.
 *
 *******************************************************************************/
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    char *payloadptr = (char *)message->payload; // ponteiro para os dados da mensagem

    printf("Received message on topic %s: %s\n", topicName, payloadptr);

    // Verifica o tópico e envia para o InfluxDB correspondente
    if (strstr(topicName, "/temp"))
    {
        writeToInfluxDB("temp", payloadptr);
    }
    else if (strstr(topicName, "/volt"))
    {
        writeToInfluxDB("volt", payloadptr);
    }
    else if (strstr(topicName, "/ack"))
    {
        writeToInfluxDB("ack", payloadptr);
    }
    else if (strstr(topicName, "/lat"))
    {
        writeToInfluxDB("lat", payloadptr);
    }
    else if (strstr(topicName, "/lon"))
    {
        writeToInfluxDB("lon", payloadptr);
    }
    else if (strstr(topicName, "/power"))
    {
        writeToInfluxDB("power", payloadptr);
    }
    else if (strstr(topicName, "/roll_angle"))
    {
        writeToInfluxDB("roll_angle", payloadptr);
    }
    else if (strstr(topicName, "/velocity"))
    {
        writeToInfluxDB("velocity", payloadptr);
    }



    // Liberta memória alocada pelo MQTTAsync
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1; // indica que a mensagem foi processada
}

/*******************************************************************************
 * SubonConnect
 *
 * param[in]: context  - contexto associado ao cliente MQTT clientSub
 * param[in]: response - estrutura MQTTAsync_successData com dados sobre a ligação
 *
 * retorno: void
 *
 * sumário: callback chamada quando o subscriber estabelece ligação com sucesso ao broker.
 *
 *******************************************************************************/
void SubonConnect(void *context, MQTTAsync_successData *response)
{
    printf("Subscriber: Successful connection to the broker.\n");

    // Estrutura com opções para a subscrição
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

    // Define callbacks da subscrição
    opts.onSuccess = onSubscribe;        // chamada se subscrição for bem sucedida
    opts.onFailure = onSubscribeFailure; // chamada se subscrição falhar
    opts.context = clientSub;            // contexto associado

    // Tenta subscrever ao tópico definido
    if (MQTTAsync_subscribe(clientSub, TOPIC, QOS, &opts) != MQTTASYNC_SUCCESS)
    {
        printf("Subscriber: Failed to start subscription.\n");
        finished = 1; // interrompe a execução do programa
    }
}

/*******************************************************************************
 * Subconnlost
 *
 * param[in]: context - contexto associado ao cliente MQTT clientPub
 * param[in]: cause   - motivo da perda de ligação
 *
 * retorno: void
 *
 * sumário: callback chamado quando a ligação é perdida. Tenta reestabelecer a ligação
 *
 *******************************************************************************/
void Subconnlost(void *context, char *cause)
{
    int rc;

    printf("Connection lost...Cause: %s.\n", cause);
    printf("Reconnecting...\n");

    /* Parâmetros para tentar nova ligação */
    MQTTAsync_connectOptions conOpts = MQTTAsync_connectOptions_initializer;
    conOpts.username = USERNAME;
    conOpts.password = PASSWORD;
    conOpts.keepAliveInterval = 20;
    conOpts.cleansession = 1;
    conOpts.onSuccess = SubonConnect;
    conOpts.onFailure = SubonConnectFailure;
    conOpts.context = clientSub;

    /* Reestabelece a ligação */
    if ((rc = MQTTAsync_connect(clientSub, &conOpts)) != MQTTASYNC_SUCCESS)
    {
        printf("Subscriber: failed to reconnect (status = %d).\n", rc);
        finished = 1;
    }
}

/*******************************************************************************
 * initSubscriber
 *
 * param[in]: client - instância do cliente MQTT previamente criada no main
 *
 * retorno: int - código de retorno da função MQTTAsync_connect
 *                0 em caso de pedido de ligação bem sucedido
 *                e qualquer outro número em caso de erro
 *
 * sumário: inicializa e configura o subscriber
 *
 *******************************************************************************/
int initSubscriber(MQTTAsync clientSub)
{

    /* Associa as funções de callback ao cliente MQTT.
   No subscriber é preciso tratar tanto da perda de ligação (Subconnlost)
   como da receção de mensagens (msgarrvd).
   Os restantes callbacks não são necessários aqui, por isso ficam a NULL. */
    MQTTAsync_setCallbacks(clientSub, NULL, Subconnlost, msgarrvd, NULL);

    // Configura parâmetros da ligação
    MQTTAsync_connectOptions conOpts = MQTTAsync_connectOptions_initializer;
    conOpts.username = USERNAME;             // credencial enviada ao broker
    conOpts.password = PASSWORD;             // credencial enviada ao broker
    conOpts.keepAliveInterval = 20;          // tempo máximo entre mensagens
    conOpts.cleansession = 1;                // não guardar estado anterior
    conOpts.onSuccess = SubonConnect;        // callback: ligação bem sucedida
    conOpts.onFailure = SubonConnectFailure; // callback: falha na ligação
    conOpts.context = clientSub;             // ponteiro de contexto associado

    // Inicia a ligação ao broker e retorna o código de resultado
    return MQTTAsync_connect(clientSub, &conOpts);
}
