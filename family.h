#ifndef __FAMILY_H_
#define __FAMILY_H_

#define NUMBER_OF_FAMILIES 10

struct FAMILY
{
    int family_id;
    int starvation_rate;
    int is_dead;
} family;

void initialize_families(struct FAMILY *families);
void increase_starvation_and_check_dead(struct FAMILY *families);
#endif