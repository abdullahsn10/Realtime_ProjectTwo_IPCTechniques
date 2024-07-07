#include "header.h"
#include "functions.h"
#include "constants.h"
#include "simulationfuncs.h"
#include "comittees.h"
#include "opengl.h"


struct COLLECTING our_comm;
// communction with opengl
int n, privatefifo, publicfifo;
struct message msg;


int main(int argc, char **argv)
{
    if (argc != 5)
    {
        perror("our_comm Arguments Error\n");
        exit(10);
    }


    /* Open the public FIFO for writing */
    if ((publicfifo = open(PUBLIC, O_WRONLY)) == -1)
    {
        printf("Public error");
        perror(PUBLIC);
        exit(2);
    }

    our_comm.committee_id = atoi(argv[1]);
    our_comm.number_of_workers = atoi(argv[2]);
    // gnd to collect mqid commu
    int gnd_to_collect_mqid = atoi(argv[3]);

    // creating array of workers
    our_comm.workers = (struct WORKER *)malloc(our_comm.number_of_workers * sizeof(struct WORKER));

    // for to add energy to workers
    for (int i = 0; i < our_comm.number_of_workers; i++)
    {
        our_comm.workers[i].energy = get_random_number_in_range(getpid(), 50, 100);
        // printf("CC worker %d with energy: %d\n",i+1,our_comm.workers[i].energy);
    }
    int num_of_worker = our_comm.number_of_workers;

    // array of dead workes (flag to indicate that this worker is dead 1 or not 0)
    int dead_workers_index[our_comm.number_of_workers];
    // fill array
    for (int i = 0; i < our_comm.number_of_workers; i++)
    {
        dead_workers_index[i] = 0;
    }

    // msg rcvd from gnd_to_collect msgq
    struct GND_TO_COLLECT_MSG rcvd_msg_from_gnd;

    // required values for save storage shmem
    int semid, shmid;
    pid_t ppid = getppid();
    char *shmptr;
    struct SAVE_STORAGE_SHMEM *memptr;
    key_t key_sv = atoi(argv[4]);

    // read one container and send to
    while (1)
    {
        // read current_cotainer from gnd-to-collect msgq
        if (msgrcv(gnd_to_collect_mqid, &rcvd_msg_from_gnd,
                   sizeof(rcvd_msg_from_gnd), 50, 0) == -1)
        {
            perror("Recv Error\n");
            exit(70);
        }

        printf("[collecting %d]: Container received from ground with Amount = %d\n", our_comm.committee_id, rcvd_msg_from_gnd.container.flour_amount_kg);
        fflush(stdout);
        // Send to OpenGL
        // change color of collecting team -> C to represent the message
        sprintf(msg.cmd_line, "C,%d,%d", our_comm.committee_id - 1, our_comm.number_of_workers);
        write(publicfifo, (char *)&msg, sizeof(msg));

        // get the amount of dead worker
        int num_of_dead_worker = 0;

        // here the comottee in safe storage
        for (int i = 0; i < our_comm.number_of_workers; i++)
        {
            int death_check = get_random_number_in_range(getpid(), 0, our_comm.workers[i].energy + 10);
            if (death_check > our_comm.workers[i].energy)
            {
                // this worker dead by ide

                // free
                printf("[Collecting Death]: the worker %d of collecting team %d is dead\n", i + 1, our_comm.committee_id);
                fflush(stdout);
                // update the number of worker
                our_comm.number_of_workers -= 1;
                dead_workers_index[i] = 1;
                num_of_dead_worker += 1;
                kill(getppid(),SIGBUS);
                // send informations to opengl that worker is died due to shooting
                sprintf(msg.cmd_line, "d,%d,%d", our_comm.committee_id - 1, our_comm.number_of_workers);
                write(publicfifo, (char *)&msg, sizeof(msg));

            }
            else
            {
                // update the energy for worker
                our_comm.workers[i].energy -= get_random_number_in_range(getpid(), 5, 20);
            }
        }

        // sleep amount of time duration to arrive the safe storege
        int dutration_time = get_random_number_in_range(getpid(), 2, 8);
        sleep(dutration_time);

        // add its amount to save storage += amount
        // open the shaerd memory and add the new amount
        /*
         * Access, attach and reference the shared memory
         */
        if ((shmid = shmget(key_sv, 0, 0)) != -1)
        {
            if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
            {
                perror("shmat -- producer -- attach (Producer)");
                exit(1);
            }
            memptr = (struct SAVE_STORAGE_SHMEM *)shmptr;
        }
        else
        {
            perror("shmget -- producer -- access (producer get)");
            exit(2);
        }

        /*
         * Access the semaphore set
         */
        if ((semid = semget(key_sv, 2, 0)) == -1)
        {
            perror("semget -- producer -- access");
            exit(3);
        }

        // Acquire
        acquire.sem_num = COLLECTING_PROD;
        if (semop(semid, &acquire, 1) == -1)
        {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        // critical section
        memptr->amount += rcvd_msg_from_gnd.container.flour_amount_kg;
        memptr->num_of_dead_worker = num_of_dead_worker;
        printf("[Save Storage]: Total Amount = %d with dead worker %d\n", memptr->amount, memptr->num_of_dead_worker);
        fflush(stdout);

        // add to save storage --> amount
        // rechange color of the collecting team
        // Message character (S) to save amount in storage
        sprintf(msg.cmd_line, "S,%d,%d", our_comm.committee_id, memptr->amount);
        write(publicfifo, (char *)&msg, sizeof(msg));


        // release
        release.sem_num = SPLITTING_CONS;
        if (semop(semid, &release, 1) == -1)
        {
            perror("semop -- producer -- release");
            exit(5);
        }
        // replace the worker dead
        for (int i = 0; i < num_of_worker; i++)
        {
            if (dead_workers_index[i] == 1)
            {
                // ask spillting to get new worker
                // sent signal to spillting to reduse the worker by 1
                //
                our_comm.workers[i].energy = get_random_number_in_range(getpid(), 50, 100);
                dead_workers_index[i] = 0;
                our_comm.number_of_workers += 1;
            }
        }
    }
    exit(0);
}