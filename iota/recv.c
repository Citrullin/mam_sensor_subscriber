/*
 * Copyright (c) 2018 IOTA Stiftung
 * https:github.com/iotaledger/entangled
 *
 * MAM is based on an original implementation & specification by apmi.bsu.by
 * [ITSec Lab]
 *
 * Refer to the LICENSE file for licensing information
 */

#include <stdio.h>

#include "common.h"


int mam_receive(byte_t *payload, int *payload_size, char *host, int port, char *bundle_hash, char *channel_id) {
  mam_api_t api;
  int ret = EXIT_SUCCESS;
  tryte_t *payload_trytes = NULL;

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  bool is_last_packet;

  // Loading or creating MAM API
  if ((ret = mam_api_load(MAM_FILE, &api)) == RC_UTILS_FAILED_TO_OPEN_FILE) {
    if ((ret = mam_api_init(&api, (tryte_t *)bundle_hash)) != RC_OK) {
      fprintf(stderr, "mam_api_init failed with err %d\n", ret);
      return EXIT_FAILURE;
    }
  } else if (ret != RC_OK) {
    fprintf(stderr, "mam_api_load failed with err %d\n", ret);
    return EXIT_FAILURE;
  }

  receive_bundle(host, port, (tryte_t *)bundle_hash, bundle);

  mam_psk_t_set_add(&api.psks, &psk);
  mam_api_add_trusted_channel_pk(&api, (tryte_t *)channel_id);

  size_t payload_tryte_size;
  if (mam_api_bundle_read(&api, bundle, &payload_trytes,(size_t *) &payload_tryte_size, &is_last_packet) == RC_OK) {
    if (payload_trytes == NULL || payload_tryte_size == 0) {
      fprintf(stderr, "No payload\n");
    } else {

        fprintf(stderr, "payload_trytes: ");
        for(int i = 0; i < payload_tryte_size; i++){
            fprintf(stderr, "%c", payload_trytes[i]);
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "payload_size_trytes: %i\n", payload_tryte_size);

        *payload_size = MIN_BYTES(payload_tryte_size * NUMBER_OF_TRITS_IN_A_TRYTE);
        payload = (byte_t*) malloc(*payload_size * sizeof(byte_t));

        size_t trit_size = (payload_tryte_size * NUMBER_OF_TRITS_IN_A_TRYTE);
        trit_t* payload_trits = (trit_t*) malloc(trit_size * sizeof(trit_t));

        trytes_to_trits(payload_trytes, payload_trits, payload_tryte_size);

        fprintf(stderr, "payload_size_trits: %i\n", trit_size);

        fprintf(stderr, "payload: [ ");
        for (unsigned int i = 0; i < trit_size; i++) {
            if (i == 0) {
                fprintf(stderr, "0x%x", payload_trits[i]);
            } else {
                printf(stderr, ", 0x%x", payload_trits[i]);
            }
        }
        printf(" ]\n");

        trits_to_bytes(payload_trits, payload, trit_size);

        fprintf(stderr, "payload_size: %i\n", *payload_size);

        fprintf(stderr, "payload: [ ");
        for (unsigned int i = 0; i < *payload_size; i++) {
            if (i == 0) {
                fprintf(stderr, "0x%x", payload[i]);
            } else {
                printf(stderr, ", 0x%x", payload[i]);
            }
        }
        printf(" ]\n");
    }
  } else {
    fprintf(stderr, "mam_api_bundle_read_msg failed\n");
  }

  // Saving and destroying MAM API
  if ((ret = mam_api_save(&api, MAM_FILE)) != RC_OK) {
    fprintf(stderr, "mam_api_save failed with err %d\n", ret);
  }
  if ((ret = mam_api_destroy(&api)) != RC_OK) {
    fprintf(stderr, "mam_api_destroy failed with err %d\n", ret);
    return EXIT_FAILURE;
  }

  // Cleanup
  { bundle_transactions_free(&bundle); }

  return ret;
}
