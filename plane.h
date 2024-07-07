#ifndef __PLANE_H_
#define __PLANE_H_


#define INITIAL_HEIGHT 100
#define PLANE_GND_MSG_KEY 'C'
#define GROUND_CAPACITY 20

struct PLANE{
    int number_of_containers;
    int drop_time;
    int refill_period;
    int my_id;
    struct CONTAINER* containers;
} plane;


struct CONTAINER{
    int flour_amount_kg;
    int height;

}container;

struct PLANE_TO_GND_MSG{
    long msg_type;
    struct CONTAINER container;
} plane_to_gnd_msg;


#endif