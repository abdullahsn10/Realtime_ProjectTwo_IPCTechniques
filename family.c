#include "family.h"
#include "header.h"
#include "functions.h"
#include "constants.h"
#include "simulationfuncs.h"
#include "comittees.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        perror("Family Argument Error\n");
        exit(66);
    }
    // families need wheat flour
    struct FAMILY families[NUMBER_OF_FAMILIES];

    // initialize values
    initialize_families(families);


    // required values for dist point to families shmem
    key_t dist_point_key = atoi(argv[1]);
        // required values for dist_point shmem
    int            dist_point_semid, dist_point_shmid;
    char          *dist_point_shmptr;
    struct DISTRIBUTION_POINT_TO_FAMILIES   *dist_point_memptr;

    int amount_to_dist=0;

    while (1)
    {
        // get amount of bags from shmem


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
        acquire.sem_num =FAMILY_CONS;
        if ( semop(dist_point_semid, &acquire, 1) == -1 ) {
            perror("semop -- producer -- acquire");
            exit(4);
        }
        //critical section
        
        // add distribution bags to ready bags in dist point shmem
        amount_to_dist += dist_point_memptr->ready_bags ;
        dist_point_memptr->ready_bags=0;


        //release
        release.sem_num = DIST_PROD;
        if ( semop(dist_point_semid, &release, 1) == -1 ) {
            perror("semop -- producer -- release");
            exit(5);
        }

        // sorting families on starvation rate
        sort_families_on_starvation_rate_desc(families, NUMBER_OF_FAMILIES);
        for (int i = 0; i < NUMBER_OF_FAMILIES; i++)
        {
            if (amount_to_dist)
            {
                if (!families[i].is_dead)
                {   
                    printf("[Family %d]: Take 20 Bags of 1Kg\n", families[i].family_id);
                    fflush(stdout);
                    families[i].starvation_rate -= 15;
                    amount_to_dist -= 20;
                }
            }
            else
            {
                break;
            }
        }
        sleep(3);
        increase_starvation_and_check_dead(families);
    }

}
void initialize_families(struct FAMILY *families)
{
    for (int i = 0; i < NUMBER_OF_FAMILIES; i++)
    {
        families[i].family_id = i + 1;
        families[i].is_dead = 0;
        families[i].starvation_rate = get_random_number_in_range(getpid(), 50, 81);
    }
}

// Comparison function for qsort
int compare_families(const void *a, const void *b)
{
    struct FAMILY *family1 = (struct FAMILY *)a;
    struct FAMILY *family2 = (struct FAMILY *)b;
    return (family2->starvation_rate - family1->starvation_rate);
}

// Function to sort families based on starvation rate in descending order
void sort_families_on_starvation_rate_desc(struct FAMILY *families, int num_families)
{
    qsort(families, num_families, sizeof(struct FAMILY), compare_families);
}

void increase_starvation_and_check_dead(struct FAMILY *families)
{
    for (int i = 0; i < NUMBER_OF_FAMILIES; i++)
    {
        if (!families[i].is_dead)
        {
            families[i].starvation_rate += 12;
            if (families[i].starvation_rate >= 90)
            {
                kill(getppid(), SIGRTMIN);
                families[i].is_dead = 1;
                printf("[Family %d]: Dead\n", families[i].family_id);
            }
        }
    }
}