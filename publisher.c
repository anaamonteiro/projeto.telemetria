/*******************************************************************************
 * publisher.c
 *
 * sumário: vai receber os dados vindos do microcontrolador e
 * enviá-los de forma assíncrona para o broker
 *
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "MQTTAsync.h" // parte assíncrona da biblioteca paho.mqqt.c
#include "publisher.h"

#define USERNAME "newuser"            // user para autenticação no broker
#define PASSWORD "recrutamento2025"   // password para autenticação
#define TOPIC "recruit/ana/init"      // tópico mqtt
#define PAYLOAD "{\"msg\":\"hello\"}" // mensagem a enviar
#define QOS 1                         // Quality of service 1 = at least once -> a mensagem é entregue pelo menos 1 vez.

/*******************************************************************************
 * onConnectFailure
 *
 * param[in]: context  - contexto associado ao cliente MQTT clientPub
 * param[in]: response - ponteiro para estrutura MQTTAsync_failureData que contém
 *                       informação sobre o motivo da falha
 *
 * retorno: void
 *
 * sumário: callback executado quando o publisher falha ao tentar estabelecer a ligação
 *          inicial ao broker MQTT.
 *
 *******************************************************************************/
void onConnectFailure(void *context, MQTTAsync_failureData *response)
{
    printf("Connection to broker failed.\n");
    finished = 1;
}

/*******************************************************************************
 * onSend
 *
 * param[in]: context  - contexto associado ao envio da mensagem
 * param[in]: response - estrutura MQTTAsync_successData com informações sobre
 *                       o envio da mensagem.
 *
 * retorno: void
 *
 * sumário: Callback chamado quando uma mensagem enviada pelo publisher chega
 *          com sucesso ao broker MQTT.
 *
 *******************************************************************************/

void onSend(void *context, MQTTAsync_successData *response)
{
    printf("Message sent successfully.\n");
}

/*******************************************************************************
 * onConnect
 *
 * param[in]: context  - contexto associado ao cliente MQTT clientPub
 * param[in]: response - estrutura MQTTAsync_successData com dados sobre a ligação
 *
 * retorno: void
 *
 * sumário: callback chamada quando o publisher estabelece ligação com sucesso ao broker.
 *
 *******************************************************************************/

void onConnect(void *context, MQTTAsync_successData *response)
{
    printf("Publisher: Successful connection to the broker.\n");

    /* Estrutura que define callbacks associados ao envio da mensagem */
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onSend; // chamado quando a mensagem é entregue
    opts.context = clientPub;

    /* Payload da mensagem a enviar.
       É const char * porque o conteúdo literal não deve ser alterado. */
    const char *payload = PAYLOAD;

    /* Estrutura que representa a mensagem MQTT */
    MQTTAsync_message msg = MQTTAsync_message_initializer;
    msg.payload = (void *)payload;    // conteúdo da mensagem
    msg.payloadlen = strlen(payload); // tamanho do conteúdo
    msg.qos = QOS;                    // nível de qualidade do serviço
    msg.retained = 1;                 // retem a última mensagem no broker

    // /* Envia a mensagem para o tópico designado */
    MQTTAsync_sendMessage(clientPub, TOPIC, &msg, &opts);
    printf("Message sent to: %s\n", TOPIC);
}

/*******************************************************************************
 * connlost
 *
 * param[in]: context - contexto associado ao cliente MQTT clientPub
 * param[in]: cause   - motivo da perda de ligação
 *
 * retorno: void
 *
 * sumário: callback chamado quando a ligação é perdida. Tenta reestabelecer a ligação
 *
 *******************************************************************************/
void connlost(void *context, char *cause)
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
    conOpts.onSuccess = onConnect;
    conOpts.onFailure = onConnectFailure;
    conOpts.context = clientPub;

    /* Reestabelece a ligação */
    if ((rc = MQTTAsync_connect(clientPub, &conOpts)) != MQTTASYNC_SUCCESS)
    {
        printf("Publisher: failed to reconnect (return conde = %d).\n", rc);
        finished = 1; // termina se reconexão falhar
    }
}

/*******************************************************************************
 * initPublisher
 *
 * param[in]: clientPub - variável cliente criada no main para o publisher
 *
 * retorno: int - código de retorno da função MQTTAsync_connect
 *                0 em caso de pedido de ligação bem sucedido
 *                e qualquer outro número em caso de erro
 *
 * sumário: configuração do publisher para ligação ao broker
 *
 *******************************************************************************/
int initPublisher(MQTTAsync clientPub)
{

    /* Associa funções de callback ao cliente MQTT.
    O publisher só precisa de tratar da perda de ligação,
   por isso apenas o callback 'connlost' é definido. */
    MQTTAsync_setCallbacks(clientPub, NULL, connlost, NULL, NULL);

    /* Estrutura com parâmetros de configuração da ligação ao broker MQTT */
    MQTTAsync_connectOptions conOpts = MQTTAsync_connectOptions_initializer;
    conOpts.username = USERNAME;          // credencial enviada ao broker
    conOpts.password = PASSWORD;          // credencial enviada ao broker
    conOpts.keepAliveInterval = 20;       // tempo máximo entre mensagens
    conOpts.cleansession = 1;             // não guardar estado anterior
    conOpts.onSuccess = onConnect;        // callback: ligação bem sucedida
    conOpts.onFailure = onConnectFailure; // callback: falha na ligação
    conOpts.context = clientPub;          // ponteiro de contexto associado

    // Inicia a ligação ao broker. - O retorno indica sucesso ou falha
    return MQTTAsync_connect(clientPub, &conOpts);
}
