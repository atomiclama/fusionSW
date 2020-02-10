
#include <stdint.h>
#include <string.h>

#include "log_core.h"
#include "log_swo.h"

#include "ch.h"

// void ITM_SendChar(char);



static void swoLogger(const char * msg) {

    uint16_t msgSize = strlen(msg);

    while(msgSize--){
        ITM_SendChar(*msg++);
    }
}

void log_swoInit(void) {
    log_registerLogger(swoLogger);
}