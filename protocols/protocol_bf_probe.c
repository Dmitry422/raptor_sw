#include "protocol_bf_probe.h"

#include "protocols_common.h"
#include "psa.h"
#include "renault_v1.h"

#include <furi.h>

#define HITAG2_BF_KEY_FIELD "Hitag2 Key"
#define HITAG2_BF_RECOVERED "Recovered"
#define HITAG2_BF_KEY_SIZE  6U

static bool bf_probe_protocol_is(FlipperFormat* ff, const char* name) {
    FuriString* value = furi_string_alloc();
    flipper_format_rewind(ff);
    const bool match = flipper_format_read_string(ff, FF_PROTOCOL, value) &&
                       furi_string_cmp_str(value, name) == 0;
    furi_string_free(value);
    return match;
}

static bool bf_probe_has_key(FlipperFormat* ff) {
    FuriString* value = furi_string_alloc();
    flipper_format_rewind(ff);
    const bool has_key = flipper_format_read_string(ff, FF_KEY, value);
    furi_string_free(value);
    return has_key;
}

bool protopirate_bf_probe_psa_needs_bruteforce(FlipperFormat* ff) {
    if(!ff || !bf_probe_protocol_is(ff, PSA_PROTOCOL_NAME) || !bf_probe_has_key(ff)) {
        return false;
    }

    // A serial is only written once the key has been recovered.
    uint32_t serial = 0;
    flipper_format_rewind(ff);
    return !flipper_format_read_uint32(ff, FF_SERIAL, &serial, 1);
}

bool protopirate_bf_probe_hitag2_needs_bruteforce(FlipperFormat* ff) {
    if(!ff || !bf_probe_protocol_is(ff, RENAULT_PROTOCOL_V1_NAME) || !bf_probe_has_key(ff)) {
        return false;
    }

    // Older captures store Recovered as hex, newer ones as uint32.
    uint8_t recovered = 0;
    flipper_format_rewind(ff);
    if(!flipper_format_read_hex(ff, HITAG2_BF_RECOVERED, &recovered, 1)) {
        uint32_t recovered_u32 = 0;
        flipper_format_rewind(ff);
        if(flipper_format_read_uint32(ff, HITAG2_BF_RECOVERED, &recovered_u32, 1)) {
            recovered = (uint8_t)recovered_u32;
        }
    }
    if(recovered != 0) {
        return false;
    }

    // A stored non-zero key leaves nothing to search for.
    uint8_t stored_key[HITAG2_BF_KEY_SIZE] = {0};
    flipper_format_rewind(ff);
    if(flipper_format_read_hex(ff, HITAG2_BF_KEY_FIELD, stored_key, sizeof(stored_key))) {
        for(size_t i = 0; i < sizeof(stored_key); i++) {
            if(stored_key[i]) {
                return false;
            }
        }
    }

    return true;
}

bool protopirate_bf_probe_needs_bruteforce(FlipperFormat* ff) {
    return protopirate_bf_probe_psa_needs_bruteforce(ff) ||
           protopirate_bf_probe_hitag2_needs_bruteforce(ff);
}
