
#include "ch.h"
#include "log_core.h"
#include <string.h>


static void swoLogger(const char * msg) {

    uint16_t msgSize = strlen(msg);

    while(msgSize--){
        ITM_SendChar(*msg++);
    }
}

void log_swoInit(void) {
    // init ITM swo channel
    
    log_registerLogger(swoLogger);
}