#ifndef __MSGQ_H_
#define __MSGQ_H_
#include "header.h"

int create_MQ(key_t key){

    int mid;
    if((mid = msgget(key, IPC_CREAT | 0777)) == -1){
        perror("Creation of the msg Q error\n");
        exit(66);
    }
    return mid;

}

void clean_MQ(int mid){
    msgctl(mid, IPC_RMID, (struct msqid_ds *) 0);
}

int check_queue_empty(int queue, long queue_type)
{
    if (msgrcv(queue, NULL, 0, queue_type, IPC_NOWAIT) == -1)
    {
        if (errno == E2BIG)
            return 0; // there is data
    }

    return 1; // empty queue
}


#endif