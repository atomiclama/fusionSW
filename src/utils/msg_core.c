// taken from: -
// http://www.chibios.org/dokuwiki/doku.php?id=chibios:book:kernel_mailboxes


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
#define MAP_SIZE 2

typedef enum{
    R1tx,
    R1rx,
    R2tx,
    R2rx,
    U1tx,
    U1rx,
    U2tx,
    U2rx,
    Clitx,
    Clirx
}map_e;
typedef struct {
    msg_t queue[MAP_SIZE];
    mailbox_t mailbox;
    map_e in,out;
}map_t;


static map_t mapping[MAP_NUM];

void map_init() {
    for(uint8_t i=0; i<MAP_NUM; i++ ) {
        chMBObjectInit(&(mapping[i].mailbox), mapping[i].queue, MAP_SIZE);
    }
    // 
    // need to populate the in and out of the mapping
    //
    mapping[0].in = R1rx;
    mapping[0].out = U1tx;
}
}

mailbox_t * map_getMailbox(map_e id) {
    for(uint8_t i=0; i<MAP_NUM; i++ ) {
        if((mapping[i].in == id) || (mapping[i].out == id)) {
            return &(mapping[i].mailbox);
        }
    }
    return NULL;
}

