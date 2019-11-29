// taken from: -
// http://www.chibios.org/dokuwiki/doku.php?id=chibios:book:kernel_mailboxes


#include "ch.h"
#include "msg_core.h"

// uart idle
// http://www.chibios.com/forum/viewtopic.php?f=25&t=3441&start=10


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

void msg_alloc (uint8_t ** pBuff) {

    (void)chMBFetchTimeout(&msgMailbox, (msg_t*)*pBuff, TIME_INFINITE);
}

void msg_free (uint8_t * pBuff) {
    // this should not fail as we can not have more msg to return than were allocated in the first place.
    (void)chMBPostTimeout(&msgMailbox, (msg_t)pBuff, TIME_IMMEDIATE);
}