#include "header.h"
#include "functions.h" 
#include "constants.h"
#include "simulationfuncs.h"
#include "plane.h"
#include "msg_queue.h"
#include "opengl.h"

// OpenGL
struct message new_message;
int n, privatefifo, publicfifo;

struct PLANE plane;
void re_fill_containers();

int main(int argc, char **argv){
    if(argc != 6){
        perror("Plane Arguments Error\n");
        exit(10);
    }

    /* Open the public FIFO for OpenGL writing */
    if ((publicfifo = open(PUBLIC, O_WRONLY)) == -1)
    {
        printf("Public error");
        perror(PUBLIC);
        exit(2);
    }

    // fill arguments
    plane.my_id = atoi(argv[1]);
    plane.number_of_containers = atoi(argv[2]);
    plane.drop_time = atoi(argv[3]);
    plane.refill_period = atoi(argv[4]);

    // plane to ground commu parameters
    int plane_to_gnd_mqid = atoi(argv[5]);

    printf("[Plane %d Notification]: My Capacity is %d Containers\n", plane.my_id, plane.number_of_containers);
    fflush(stdout);

    
//--------------------------------------------------------------
    while(1){
        // fill containers
            // create containers (fill containers)
        plane.containers = (struct CONTAINER *) malloc(plane.number_of_containers * sizeof(struct CONTAINER));
        for(int i=0; i<plane.number_of_containers; i++){
            plane.containers[i].flour_amount_kg = get_random_number_in_range(getpid(), 300, 500);
            plane.containers[i].height = INITIAL_HEIGHT;
        }   

        // drop containers into ground
        for(int i=0; i<plane.number_of_containers; i++){
            // drop to ground
            // write it to msg queue

            struct PLANE_TO_GND_MSG msg;
            msg.container = plane.containers[i];
            msg.msg_type = 20;
            printf("[Plane %d]: A container %d (%d) is being dropped now\n", plane.my_id, plane.containers[i].flour_amount_kg, i+1);
            fflush(stdout);

            // send to opengl that container is being dropped
            sprintf(new_message.cmd_line, "c,%d,%d,%d", plane.my_id - 1, i, plane.containers[i].flour_amount_kg);
            write(publicfifo, (char *)&new_message, sizeof(new_message));

            // While dropping container, Attack from occupation may happen
            // Attacking Scenario Simulation
            int time_to_drop_container = 6,j;
            for(j=0; j< time_to_drop_container; j++){
                sleep(1);
                int attack = get_random_number_in_range(getpid(),0,100);
                if(attack > 93){
                    // j refers somehow to the needed time to drop container
                    // lower j means its far from ground
                    // higher j means near the ground
                    if(j<2){
                        printf("[Attack]: Shooting All Plane %d's Container of(%dKg) Amount"
                        ,plane.my_id, plane.containers[i].flour_amount_kg);
                        // Send to OpenGL that this container is being fully attacked
                        // disappear
                        fflush(stdout);
                        msg.container.flour_amount_kg=0;
                        plane.containers[i].flour_amount_kg =0;
                        kill(getppid(), SIGUSR2);
                        break;
                    }else{
                        printf("[Attack]: Attack Plane %d's Container of(%dKg) Amount --> (%dKg)"
                        ,plane.my_id, plane.containers[i].flour_amount_kg,plane.containers[i].flour_amount_kg -200 );
                        fflush(stdout);
                        // send to openGL that this container is being partially attacked
                        // send to opengl that container has been shot --> (s) got shot
                        sprintf(new_message.cmd_line, "s,%d,%d,%d", plane.my_id - 1, i, plane.containers[i].flour_amount_kg - 200);
                        write(publicfifo, (char *)&new_message, sizeof(new_message));
                        // Change color only
                        msg.container.flour_amount_kg -=200;
                        plane.containers[i].flour_amount_kg -=200;
                        kill(getppid(), SIGUSR2);
                        break;
                    }         
                }
            }

            // send the msg to the msg queue
            if(msg.container.flour_amount_kg !=0 ){
                if(msgsnd(plane_to_gnd_mqid, &msg, sizeof(msg), IPC_NOWAIT) == -1){
                    perror("Queue send error\n");
                    exit(70);
                }
            }
            // wait for drop time period
            sleep(plane.drop_time - j+1);
        }

        // free containers memory
        free(plane.containers);
        
        // re-fill
        printf("[Plane %d]: Going to Re-Fill\n", plane.my_id);
        fflush(stdout);
        // Send to OpenGL that we are going to re-fill
        sprintf(new_message.cmd_line, "r,%d,%d", plane.my_id - 1, plane.number_of_containers);
        write(publicfifo, (char *)&new_message, sizeof(new_message));
        sleep(plane.refill_period);
        // e --> end refill period
        sprintf(new_message.cmd_line, "e,%d,%d,%d", plane.my_id - 1, plane.number_of_containers, plane.drop_time);
        write(publicfifo, (char *)&new_message, sizeof(new_message));
    }

    exit(0);
}

