#include "header.h"
#include "functions.h" 
#include "constants.h"
#include "simulationfuncs.h"
#include "comittees.h"
#include "opengl.h"


struct DISTRIBUTING our_comm;
// communction with opengl
int n, privatefifo, publicfifo;
struct message msg;


int main(int argc, char **argv){
    if(argc != 5){
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
    // creating array of workers
    our_comm.workers = (struct WORKER *) malloc(our_comm.number_of_workers * sizeof(struct WORKER));

      // for to add energy to workers
    for (int i=0;i<our_comm.number_of_workers;i++){
        our_comm.workers[i].energy=get_random_number_in_range(getpid(),50,100);
    }

    // required values for bags shmem
    int            bags_semid, bags_shmid;
    char          *bags_shmptr;
    struct BAGS   *bags_memptr;
    key_t bags_key = atoi(argv[3]);
    int bags_to_get = our_comm.number_of_workers * 20;
    int distribution_amount=0;

    // required values for dist_point shmem
    int            dist_point_semid, dist_point_shmid;
    char          *dist_point_shmptr;
    struct DISTRIBUTION_POINT_TO_FAMILIES   *dist_point_memptr;
    // required values for dist point to families shmem
    key_t dist_point_key = atoi(argv[4]);


    // array of dead workes (flag to indicate that this worker is dead 1 or not 0)
    int dead_workers_index[our_comm.number_of_workers];
    int num_of_worker = our_comm.number_of_workers;
    // fill array
    for (int i = 0; i < our_comm.number_of_workers; i++)
    {
        dead_workers_index[i] = 0;
    }


    int num_of_dead_worker = 0;
    while (1)
    {   
        // bags shmem
        /*
        * Access, attach and reference the shared memory
        */
        if ( (bags_shmid = shmget(bags_key, 0, 0)) != -1 ) {
            if ( (bags_shmptr = (char *) shmat(bags_shmid, (char *)0, 0)) == (char *) -1 ) {
            perror("shmat -- producer -- attach (Producer)");
            exit(1);
            }
            bags_memptr = (struct BAGS *) bags_shmptr;
        }
        else {
            perror("shmget -- producer -- access (producer get)");
            exit(2);
        }
        
        /*
        * Access the semaphore set
        */
        if ( (bags_semid = semget(bags_key, 2, 0)) == -1 ) {
            perror("semget -- producer -- access");
            exit(3);
        }

        // Acquire
        acquire.sem_num =DIST_CONS;
        if ( semop(bags_semid, &acquire, 1) == -1 ) {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        //critical section
        if(bags_memptr->number_of_bags> bags_to_get){
            bags_memptr->number_of_bags -= bags_to_get;
            distribution_amount = bags_to_get;   
        }else{
            distribution_amount = bags_memptr->number_of_bags;
            bags_memptr->number_of_bags = 0;
        }

        printf("[Bags]: Total Amount = %d\n", bags_memptr->number_of_bags);
        fflush(stdout);
        // send to openGl bags to be distributed (b)
        sprintf(msg.cmd_line, "b,%d", bags_memptr->number_of_bags);
        write(publicfifo, (char *)&msg, sizeof(msg));

        printf("[Distribution]: Total Amount to dist= %d\n", distribution_amount);
        fflush(stdout);

        bags_memptr->number_of_dead_worker = num_of_dead_worker;



        //release
        release.sem_num = SPLITTING_PROD;
        if ( semop(bags_semid, &release, 1) == -1 ) {
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




        num_of_dead_worker = 0;
        // go to dist to families
        for (int i = 0; i < our_comm.number_of_workers; i++)
        {
            int death_check = get_random_number_in_range(getpid(), 0, our_comm.workers[i].energy + 5);
            if (death_check > our_comm.workers[i].energy)
            {
                // this worker dead by occupation

                // free
                printf("[Distibuting Death]: the worker %d of distributing team %d is dead\n", i + 1, our_comm.committee_id);
                // update the number of worker
                our_comm.number_of_workers -= 1;
                dead_workers_index[i] = 1;
                num_of_dead_worker += 1;
                kill(getppid(),SIGINT);

            }
            else
            {
                // update the energy for worker
                our_comm.workers[i].energy -= get_random_number_in_range(getpid(), 5, 20);
            }
        }

        // sleep amount of time duration to end the trip to dist
        sleep(2);

        // Distribution Point to Families


        /*
        * Access, attach and reference the shared memory
        */
        if ( (dist_point_shmid = shmget(dist_point_key, 0, 0)) != -1 ) {
            if ( (dist_point_shmptr = (char *) shmat(dist_point_shmid, (char *)0, 0)) == (char *) -1 ) {
            perror("shmat -- producer -- attach (Producer)");
            exit(1);
            }
            dist_point_memptr = (struct DISTRIBUTION_POINT_TO_FAMILIES *) dist_point_shmptr;
        }
        else {
            perror("shmget -- producer -- access (producer get)");
            exit(2);
        }
        
        /*
        * Access the semaphore set
        */
        if ( (dist_point_semid = semget(dist_point_key, 2, 0)) == -1 ) {
            perror("semget -- producer -- access");
            exit(3);
        }

        // Acquire
        acquire.sem_num =DIST_PROD;
        if ( semop(dist_point_semid, &acquire, 1) == -1 ) {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        //critical section

        // add distribution bags to ready bags in dist point shmem
        dist_point_memptr->ready_bags += distribution_amount;
        printf("[Distribution Point]: Ready Bags For Families = %d\n", dist_point_memptr->ready_bags);
        fflush(stdout);


        //release
        release.sem_num = FAMILY_CONS;
        if ( semop(dist_point_semid, &release, 1) == -1 ) {
            perror("semop -- producer -- release");
            exit(5);
        }

    }





    exit(0);

    // read from dist shmem --> families


}