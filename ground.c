#include "header.h"
#include "plane.h"
#include "msg_queue.h"
#include "comittees.h"

int main(int argc, char **argv){
    if(argc != 3){
        perror("Ground Arguments Error\n");
        exit(10);
    }

    // containers on ground data fields
    struct CONTAINER on_ground_container[GROUND_CAPACITY];
    int container_index = 0;

    // msg rcvd from plane
    struct PLANE_TO_GND_MSG msg;

    // get arguments for plane to ground queue commu
    int plane_to_gnd_mqid = atoi(argv[1]);

    // get arguments for gnd to collect commu
    int gnd_to_collect_mqid = atoi(argv[2]);

    // msg to be sent to collecting mq
    struct GND_TO_COLLECT_MSG msg_to_collect;



    // read containers from msg queue and send them to collected 
    while(1){
        if(msgrcv(plane_to_gnd_mqid, &msg, sizeof(msg), 20, 0) == -1){
                perror("Recv Error\n");
                exit(70);
        }
        on_ground_container[container_index] = msg.container;
        // send this container to the gnd-collecter msg queue
        msg_to_collect.msg_type = 50;
        msg_to_collect.container = on_ground_container[container_index];
        if(msgsnd(gnd_to_collect_mqid, &msg_to_collect, sizeof(msg_to_collect), IPC_NOWAIT) == -1){
            perror("Collecting Queue send error\n");
            exit(75);
        }
        printf("[Ground]: Container received at ground with Amount = %d\n", on_ground_container[container_index].flour_amount_kg);
        fflush(stdout);
        container_index++;
            
    }

    sleep(2);
    exit(0);

}