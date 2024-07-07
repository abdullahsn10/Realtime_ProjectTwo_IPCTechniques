#ifndef __FUNCTIONS__
#define __FUNCTIONS__

#include "header.h"
#include "constants.h"

// reading from a file and updating values directly from constants.h
void read_settings_from_a_file(char* filename) {
    char line[255];
    char label[50];

    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL){
        perror("Failed to open the file\n");
        exit(-2); // return failure status code
    }

    // separator of the file
    char separator[] = "=";

    // read the file line by line until reaching null
    while(fgets(line, sizeof(line), file) != NULL){
        // split line 
        char *str = strtok(line, separator);
        strncpy(label, str, sizeof(label));
        str = strtok(NULL, separator);
        
        // update arguments according to the label
        if(strcmp(label, "MIN_FLOUR_CONTAINERS") == 0){
            MIN_FLOUR_CONTAINERS = atoi(str);
        }
        else if(strcmp(label, "NUMBER_OF_CARGO_PLANES") == 0){
            NUMBER_OF_CARGO_PLANES = atoi(str);
        }
        else if(strcmp(label, "MAX_FLOUR_CONTAINERS") == 0){
            MAX_FLOUR_CONTAINERS = atoi(str);
        }
        else if(strcmp(label, "MIN_DROP_TIME") == 0){
            MIN_DROP_TIME = atoi(str);
        }
        else if(strcmp(label, "MAX_DROP_TIME") == 0){
            MAX_DROP_TIME = atoi(str);
        }
        else if(strcmp(label, "REFILL_PERIOD_MIN") == 0){
            REFILL_PERIOD_MIN = atoi(str);
        }
        else if(strcmp(label, "REFILL_PERIOD_MAX") == 0){
            REFILL_PERIOD_MAX = atoi(str);
        }
        else if(strcmp(label, "COLLECTING_RELIEF_COMMITTEES") == 0){
            COLLECTING_RELIEF_COMMITTEES = atoi(str);
        }
        else if(strcmp(label, "COLLECTING_RELIEF_WORKERS") == 0){
            COLLECTING_RELIEF_WORKERS = atoi(str);
        }
        else if(strcmp(label, "SPLITTING_RELIEF_WORKERS") == 0){
            SPLITTING_RELIEF_WORKERS = atoi(str);
        }
        else if(strcmp(label, "DISTRIBUTING_RELIEF_WORKERS") == 0){
            DISTRIBUTING_RELIEF_WORKERS = atoi(str);
        }
        else if(strcmp(label, "NUMBER_OF_BAGS_PER_DISTRIBUTION") == 0){
            NUMBER_OF_BAGS_PER_DISTRIBUTION = atoi(str);
        }
        else if(strcmp(label, "LIMIT_DISTRIBUTE_DROP") == 0){
            LIMIT_DISTRIBUTE_DROP = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_TIME") == 0){
            THRESHOLD_TIME = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_DEATH_RATE") == 0){
            THRESHOLD_DEATH_RATE = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_CARGO_PLANES_CRASHED") == 0){
            THRESHOLD_CARGO_PLANES_CRASHED = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_CONTAINERS_DOWN") == 0){
            THRESHOLD_CONTAINERS_DOWN = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_COLLECTING_WORKERS_DOWN") == 0){
            THRESHOLD_COLLECTING_WORKERS_DOWN = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_DISTRIBUTING_WORKERS_DOWN") == 0){
            THRESHOLD_DISTRIBUTING_WORKERS_DOWN = atoi(str);
        }
        else if(strcmp(label, "THRESHOLD_FAMILIES_DOWN") == 0){
            THRESHOLD_FAMILIES_DOWN = atoi(str);
        }
    }

    fclose(file);
}

#endif
