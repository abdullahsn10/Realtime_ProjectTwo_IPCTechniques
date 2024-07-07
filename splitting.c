#include "header.h"
#include "functions.h"
#include "constants.h"
#include "simulationfuncs.h"
#include "comittees.h"
#include "opengl.h"


struct SPLITTING our_comm;
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
    // creating array of workers
    our_comm.workers = (struct WORKER *)malloc(our_comm.number_of_workers * sizeof(struct WORKER));

    //     // Print the values
    // printf("Sp Committee ID: %d\n", our_comm.committee_id);
    // printf("Sp Number of Workers: %d\n", our_comm.number_of_workers);

    // for to add energy to workers
    for (int i = 0; i < our_comm.number_of_workers; i++)
    {
        our_comm.workers[i].energy = get_random_number_in_range(getpid(), 50, 100);
        // printf("Sp worker %d with energy: %d\n",i+1,our_comm.workers[i].energy);
    }

    // required values for save storage shmem
    int semid, shmid;
    char *shmptr;
    struct SAVE_STORAGE_SHMEM *memptr;
    key_t key_sv = atoi(argv[3]);

    // splitting vars
    int splitting_amount = 0;

    // required values for bags shmem
    int bags_semid, bags_shmid;
    char *bags_shmptr;
    struct BAGS *bags_memptr;
    key_t bags_key = atoi(argv[4]);

    while (1)
    {
        int getting_amount_from_save_storage = our_comm.number_of_workers * 30;
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
        acquire.sem_num = SPLITTING_CONS;
        if (semop(semid, &acquire, 1) == -1)
        {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        // critical section
        if (memptr->amount > getting_amount_from_save_storage)
        {
            memptr->amount -= getting_amount_from_save_storage;
            splitting_amount = getting_amount_from_save_storage;
        }
        else
        {
            splitting_amount = memptr->amount;
            memptr->amount = 0;
        }
        printf("[Splitting Worker_Collect]: Total Worker before replace  = %d\n", our_comm.number_of_workers);

        if (memptr->num_of_dead_worker <= our_comm.number_of_workers)
        {
            our_comm.number_of_workers -= memptr->num_of_dead_worker;
            printf("[Splitting Workers_Collect]: Total Workers After replace  = %d\n", our_comm.number_of_workers);
        }
        else
        {
            printf("[Splitting Workers_Collect]: No Enough Workers to replace Splitting \n");
        }
        printf("[Save Storage]: Total Amount = %d\n", memptr->amount);
        fflush(stdout);
        // Message character (S) to save amount in storage
        sprintf(msg.cmd_line, "S,%d,%d", our_comm.committee_id, memptr->amount);
        write(publicfifo, (char *)&msg, sizeof(msg));

        printf("[Splitting]: Total Amount = %d\n", splitting_amount);
        fflush(stdout);

        // release
        release.sem_num = COLLECTING_PROD;
        if (semop(semid, &release, 1) == -1)
        {
            perror("semop -- producer -- release");
            exit(5);
        }

        int time_to_splitting_into_bags = get_random_number_in_range(getpid(), 2, 5);
        sleep(time_to_splitting_into_bags);

        // shmem (BAGS)
        // access to shmem, attach, cast to BAGS
        // set the number_of_bags += (splitting bags) in order to give it to dist.
        /*
         * Access, attach and reference the shared memory
         */
        if ((bags_shmid = shmget(bags_key, 0, 0)) != -1)
        {
            if ((bags_shmptr = (char *)shmat(bags_shmid, (char *)0, 0)) == (char *)-1)
            {
                perror("shmat -- producer -- attach (Producer)");
                exit(1);
            }
            bags_memptr = (struct BAGS *)bags_shmptr;
        }
        else
        {
            perror("shmget -- producer -- access (producer get)");
            exit(2);
        }

        /*
         * Access the semaphore set
         */
        if ((bags_semid = semget(bags_key, 2, 0)) == -1)
        {
            perror("semget -- producer -- access");
            exit(3);
        }

        // Acquire
        acquire.sem_num = SPLITTING_PROD;
        if (semop(bags_semid, &acquire, 1) == -1)
        {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        // critical section
        bags_memptr->number_of_bags += splitting_amount;
        printf("[Splitting]: Creating %d Bags\n", splitting_amount);
        fflush(stdout);
        printf("[Bags]: Total Amount = %d\n", bags_memptr->number_of_bags);
        fflush(stdout);
        // send to openGl bags to be distributed (b)
        sprintf(msg.cmd_line, "b,%d", bags_memptr->number_of_bags);
        write(publicfifo, (char *)&msg, sizeof(msg));

        printf("[Splitting Worker _Dist]: Total Worker before replace  = %d\n", our_comm.number_of_workers);

        if (bags_memptr->number_of_dead_worker <= our_comm.number_of_workers)
        {
            our_comm.number_of_workers -= bags_memptr->number_of_dead_worker;
            printf("[Splitting Workers_Dist]: Total Workers After replace  = %d\n", our_comm.number_of_workers);
        }
        else
        {
            printf("[Splitting Workers_Dist]: No Enough Workers to replace Splitting \n");
        }

        // release
        release.sem_num = DIST_CONS;
        if (semop(bags_semid, &release, 1) == -1)
        {
            perror("semop -- producer -- release");
            exit(5);
        }
    }

    exit(0);

    // read save storage(1000) 5k --> bag --> shmem for dist
}