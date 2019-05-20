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

#include "iota/send-msg.h"

#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "proto_compiled/DataResponse.pb.h"
#include "proto_compiled/DataRequest.pb.h"
#include "proto_compiled/FeatureResponse.pb.h"

//tmp
#include <errno.h>

#define IOTA_HOST "node05.iotatoken.nl"
#define IOTA_PORT 16265
#define IOTA_ADDRESS "V9MUPZNOBTQKUEHE9QAGUMKJRHFUWDQYHSUSZRWAOQBSUILELAYRQNIZXDCTKOUECUGFIHZZOOLGJDCTM"
#define IOTA_BUNDLE "EJAWFID9NNAECB9XNIWIDLIQEYVEO9SLTKHRGALYXGDBDHIZAHWCPWTMEMGUAVNIELPEBNTTYJDFCELHD"

#define DEBUG_SERVER true

bool client_is_running = true;

void client_stop(void) {
    client_is_running = false;
}

float get_scaled_value(environmentSensors_SingleDataPoint *data_point) {
    if(data_point->value == 0){
        return 0;
    }else{
        return data_point->value / pow(10, -data_point->scale);
    }
}

void data_response_to_env_data(env_sensor_data_t * sensor_data, environmentSensors_DataResponse * data_response) {
    sensor_data->humanity = get_scaled_value(&data_response->humanity);
    sensor_data->temperature = get_scaled_value(&data_response->temperature);
    sensor_data->pm2_5 = get_scaled_value(&data_response->pm2_5);
    sensor_data->atmosphericPressure = get_scaled_value(&data_response->atmosphericPressure);
}

void *run_receiver_thread(void *args) {
    (void) args;

    char payload;
    int payload_size;
    retcode_t err = mam_receive(&payload, &payload_size, IOTA_HOST, IOTA_PORT, IOTA_BUNDLE, IOTA_ADDRESS);

    const char* error = error_2_string(err);

    fprintf(stderr, "error %s\n", error);

    environmentSensors_DataResponse data_response = {};

    //env_sensor_data_response_decode(&data_response, (uint8_t *) &payload, &payload_size);
    env_sensor_data_t data = {};

    data_response_to_env_data(&data, &data_response);
    log_sensor_data("DEBUG", "run_receiver_thread", "data", &data);

    int value = 0;
    client_stop();
    pthread_exit(&value);
}

pthread_t receiver_thread;
int main(void) {
    receiver_thread = pthread_create(&receiver_thread, NULL, &run_receiver_thread, NULL);
    while(client_is_running){}
    return 0;
}