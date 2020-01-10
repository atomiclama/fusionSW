// taken from: -
// http://www.chibios.org/dokuwiki/doku.php?id=chibios:book:kernel_mailboxes

#include "config.h"
#include "ch.h"
#include "msg_core.h"

// 2x per port 
// 1 for log

#define MSG_NUM 16

// this is our pool of storage for messages
static uint8_t msgPool [MSG_NUM] [MSG_SIZE];

// this is the que use to manage the storage
static msg_t msgQueue[MSG_NUM];

static mailbox_t msgMailbox;

void msg_init(void) {

    chMBObjectInit(&msgMailbox, msgQueue, MSG_NUM);

    /* Pre-filling the free buffers pool with the available buffers, the post
       will not stop because the mailbox is large enough. */
    for (uint8_t i = 0; i < MSG_NUM; i++) {
        (void)chMBPostTimeout(&msgMailbox, (msg_t)&msgPool[i], TIME_IMMEDIATE);
    }
}

void msg_alloc (uint8_t * pBuff) {

    (void)chMBFetchTimeout(&msgMailbox, (msg_t*)pBuff, TIME_INFINITE);
}

void msg_free (uint8_t * pBuff) {
    // this should not fail as we can not have more msg to return than were allocated in the first place.
    (void)chMBPostTimeout(&msgMailbox, (msg_t)pBuff, TIME_IMMEDIATE);
}

// Mapping to use to transfer the messages around the system

#define MAP_NUM 6
#define MAP_SIZE 1


typedef struct {
    msg_t queue[MAP_SIZE];
    mailbox_t mailbox;
    map_e in,out;
}map_t;


static map_t mapping[MAP_NUM];

void map_init(void) {
    for(uint8_t i=0; i<MAP_NUM; i++ ) {
        chMBObjectInit(&(mapping[i].mailbox), mapping[i].queue, MAP_SIZE);
        mapping[i].in = Free;
        mapping[i].out = Free;
    }
    // 
    // need to populate the in and out of the mapping
    //
    #ifdef working_point
    mapping[0].in = U1rx;
    mapping[0].out = U2tx;

    mapping[1].in = U2rx;
    mapping[1].out = U1tx;

    mapping[2].in = R1rx;
    mapping[2].out = Clirx;

    mapping[3].in = Clitx;
    mapping[3].out = R1tx;
    #endif

#if defined USE_RX_CFG == true
    mapping[0].in = R1rx;
    mapping[0].out = decodeIn;

    mapping[1].in = decodeOut;
    mapping[1].out = U2tx;

#endif

#if defined USE_TX_CFG == true
    mapping[0].in = U1rx;
    mapping[0].out = R1tx;

    mapping[1].in = R1rx;
    mapping[1].out = Clirx;
#endif


}

mailbox_t * map_getMailbox(map_e id) {
    for(uint8_t i=0; i<MAP_NUM; i++ ) {
        if((mapping[i].in == id) || (mapping[i].out == id)) {
            return &(mapping[i].mailbox);
        }
    }
    return NULL;
}
