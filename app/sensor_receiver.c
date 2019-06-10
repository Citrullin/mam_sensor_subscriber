#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>

#include "pthread.h"
#include <unistd.h>

#include "iota/recv.h"

#include "encode/decode.h"
#include "encode/encode.h"
#include "logging/logging.h"

#include "jsmn/jsmn.h"

#include "iota/send-msg.h"

#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "proto_compiled/DataResponse.pb.h"
#include "proto_compiled/DataRequest.pb.h"
#include "proto_compiled/FeatureResponse.pb.h"

//tmp
#include <errno.h>

#define IOTA_HOST "turbor.ddns.net"
#define IOTA_PORT 14265
#define IOTA_ADDRESS "YKEGEJIDKYCCRF9YZGUHKXVIDFLBVXUHKAAWMEISBKPFIK9OTMB9XNYQJJBGNRWXSSEWZMKFRWXOQLFLY"
#define IOTA_BUNDLE "ZCZLAXHSI9DBSRB9ONKZXXOGAFLWCKEKQGDURQBCG9WEYEELHRKVQTYFQZ9FKCHUJZJDAL9VRG9PGLJ9D"

#define DEBUG_SERVER true

bool client_is_running = true;

void client_stop(void) {
    client_is_running = false;
}

#define AVAILABLE_TOKENS 20
jsmn_parser parser;
jsmntok_t tokens[AVAILABLE_TOKENS];

int json_get_token_value(char * result, char * payload, jsmntok_t * token) {
    int amount = token->end - token->start;

    char * start_ptr = payload + token->start;
    for(int i = 0; i < amount; i++) {
        result[i] = start_ptr[i];
    }
    start_ptr[amount] = '\0';

    return 0;
}

char temp_buffer[100];
void get_temperature(float * result, char * payload, jsmntok_t * tokens) {

    char string_temperature[] = "temperature";

    for(int i = 0; i < AVAILABLE_TOKENS - 1; i++) {
        memset(temp_buffer, 0, sizeof(temp_buffer));
        json_get_token_value(temp_buffer, payload, &tokens[i]);

        if(strcmp(temp_buffer, string_temperature) == 0) {
            memset(temp_buffer, 0, sizeof(temp_buffer));
            json_get_token_value(temp_buffer, payload, &tokens[i + 1]);

            break;
        }
    }

    memset(tokens, 0, AVAILABLE_TOKENS * sizeof(jsmntok_t));
    memset(&parser, 0, sizeof(jsmn_parser));

    jsmn_init(&parser);
    jsmn_parse(&parser, temp_buffer, strlen(temp_buffer), tokens, AVAILABLE_TOKENS);

    char value_buffer[10];

    int value = 0;
    int scale = 0;

    for(int i = 0; i < AVAILABLE_TOKENS - 1; i++) {
        if(tokens[i].type == JSMN_STRING){
            memset(value_buffer, 0, sizeof(value_buffer));
            json_get_token_value(value_buffer, temp_buffer, &tokens[i]);

            if(strcmp(value_buffer, "value") == 0) {
                memset(value_buffer, 0, sizeof(value_buffer));
                json_get_token_value(value_buffer, temp_buffer, &tokens[i + 1]);

                value = atof(value_buffer);
            }else if(strcmp(value_buffer, "scale") == 0) {
                memset(value_buffer, 0, sizeof(value_buffer));
                json_get_token_value(value_buffer, temp_buffer, &tokens[i + 1]);

                scale = atof(value_buffer);
            }
        }else if (tokens[i].type == JSMN_UNDEFINED){
            break;
        }
    }

    *result = value * powf(10, scale);
}


void *run_receiver_thread(void *args) {
    (void) args;

    const char payload[500];
    int payload_size;
    retcode_t err = mam_receive(payload, &payload_size, IOTA_HOST, IOTA_PORT, IOTA_BUNDLE, IOTA_ADDRESS);

    printf("Payload size: %i\n", payload_size);

    const char* error = error_2_string(err);

    fprintf(stderr, "error %s\n", error);

    printf("Payload sensor: ");
    for(int i = 0; i < payload_size; i++){
        printf("%c", payload[i]);
    }
    printf("\n");

    jsmn_init(&parser);
    jsmn_parse(&parser, payload, payload_size, tokens, AVAILABLE_TOKENS);

    float temperature = 0;
    get_temperature(&temperature, (char *) payload, tokens);

    printf("test\n");

    int value = 0;
    client_stop();
    pthread_exit(&value);
}


pthread_t receiver_thread;
int main(void) {
    pthread_create(&receiver_thread, NULL, &run_receiver_thread, NULL);
    while(client_is_running){}
    return 0;
}