/*******************************************************************************
 * influxdb.c
 *
 * sumário: funções para inicializar, enviar e fechar a ligaçao ao InfluxDB
 *          Recebe mensagens do subscriber, converte para line protocol e envia
 *          via HTTP POST usando a libcurl
 *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curl/curl.h>
#include "influxdb.h"

#define INFLUX_URL "http://172.25.16.1:8086/api/v2/write?org=tsb_projeto&bucket=dados_tel&precision=s"
#define INFLUX_TOKEN "3Lg0HGIbHDdEs_otiKLuUXKdFPDCPcqhm-ctmIu51KEEKe5LGCqLOdONnSpJfkzZ8MyzNtQEqPnMhHP6a6-h1A=="

static CURL *curl = NULL; // sessão CURL global

/*******************************************************************************
 * initInfluxDB
 *
 * param[in]: nenhum
 *
 * retorno: void
 *
 * sumário: inicializa a biblioteca CURL para comunicação HTTP com InfluxDB
 *
 *******************************************************************************/
void initInfluxDB()
{
    curl_global_init(CURL_GLOBAL_ALL); // Inicializa a biblioteca CURL globalmente
    curl = curl_easy_init();           // Cria uma sessão CURL para enviar pedidos HTTP
    if (curl == NULL)
    {
        printf("Error: Failed to initialize CURL for InfluxDB.\n");
        exit(1);
    }
}

/*******************************************************************************
 * closeInfluxDB
 *
 * param[in]: void
 *
 * retorno: void
 *
 * sumário: limpa recursos da biblioteca CURL
 *
 *******************************************************************************/
void closeInfluxDB()
{
    if (curl != NULL)
    {
        curl_easy_cleanup(curl); // limpa a sessão CURL
        curl = NULL;
    }
    curl_global_cleanup(); // limpa os recursos globais do CURL
}

/*******************************************************************************
 * writeToInfluxDB
 *
 * param[in]: measurement  - nome da medição (ex: "temperatura", "tensao", "acks")
 * param[in]: payload - string recebida do subscriber (JSON ou valor simples)
 *
 * retorno: void
 *
 * sumário: converte payload para line protocol e envia via HTTP POST para o InfluxDB
 *          Respeita intervalo mínimo de 3 segundos entre escritas
 *
 *******************************************************************************/
void writeToInfluxDB(const char *measurement, const char *payload)
{
    if (curl == NULL)
        return;

    sleep(3); // intervalo entre cada registo

    char line[256] = {0}; // vetor que guarda a mensagem que será enviada para o influxdb

    // Se o payload contiver "status", então é um ack
    if (strstr(payload, "status") != NULL)
    {
        char status[32] = {0};
        char received_topic[64] = {0};
        char time[64] = {0};

        // Ex do que pode ser enviado pelo validador: {"status":"ok","received_topic":"recruit/ana/init"}
        sscanf(payload, "{\"status\": \"%31[^\"]\", \"received_topic\": \"%63[^\"]\", \"time\": \"%63[^\"]\"", status, received_topic, time);
        snprintf(line, sizeof(line), "%s status=\"%s\",received_topic=\"%s\",time=\"%s\"", measurement, status, received_topic, time);
    }
    // Se o payload for numérico (temp, volt, lat, etc...)
    else
    {
        snprintf(line, sizeof(line), "%s value=%s", measurement, payload);
    }

    // Cria uma lista ligada de headers HTTP para o pedido CURL
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8"); // adiciona line protocol à lista
    char authHeader[128];                                                            // header de autenticação
    snprintf(authHeader, sizeof(authHeader), "Authorization: Token %s", INFLUX_TOKEN);
    headers = curl_slist_append(headers, authHeader); // Adiciona o header de autenticação à lista

    curl_easy_setopt(curl, CURLOPT_URL, INFLUX_URL);     // configura a url do influxdb
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // associa os headers HTTP criados à sessão CURL
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, line);    // Define os dados que vamos enviar (a linha em line protocol)

    // Executa o pedido HTTP POST
    CURLcode res = curl_easy_perform(curl);

    // Verifica se o envio foi bem-sucedido
    if (res != CURLE_OK)
    {
        printf("Error sending to InfluxDB: %s\n", curl_easy_strerror(res)); 
    }
    else
    {
        printf("InfluxDB write: %s\n", line); 
    }

    // Liberta headers
    curl_slist_free_all(headers);
}
