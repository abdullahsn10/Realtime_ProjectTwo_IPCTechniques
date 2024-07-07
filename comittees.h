#ifndef __COLLECT_H_
#define __COLLECT_H_
#include "plane.h"
#include "header.h"

#define GND_TO_COLLECTER_KEY 'J'
#define SAVE_STORAGE_KEY 'S'
#define BAGS_SHMEM_KEY 'B'
#define DIST_POINT_TO_FAMLILIES_KEY 'D'

struct sembuf acquire = {0, -1, SEM_UNDO},
              release = {0, 1, SEM_UNDO};

enum
{
    COLLECTING_PROD,
    SPLITTING_CONS
};
enum
{
    SPLITTING_PROD,
    DIST_CONS
};
enum{
    DIST_PROD,
    FAMILY_CONS
};

struct SAVE_STORAGE_SHMEM
{
    int amount;
    int num_of_dead_worker;
} save_storage_shmem;

struct GND_TO_COLLECT_MSG
{
    long msg_type;
    struct CONTAINER container;
} gnd_to_collect_msg;

struct WORKER
{
    int energy;
    char *type;
    float color[3];
} worker;

struct COLLECTING
{
    int committee_id;
    int number_of_workers;
    struct WORKER *workers;
    float color[3];
} collecting;

struct SPLITTING
{
    int committee_id;
    int number_of_workers;
    struct WORKER *workers;
} splitting;

struct DISTRIBUTING
{
    int committee_id;
    int number_of_workers;
    struct WORKER *workers;
} distributing;

struct BAGS
{
    int number_of_bags;
    int number_of_dead_worker;
} bags;

struct DISTRIBUTION_POINT_TO_FAMILIES{
    int ready_bags;
} distribution_point_to_families;




#endif
