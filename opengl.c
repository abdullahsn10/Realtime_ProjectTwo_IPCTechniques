#include <GL/glut.h>
#include "plane.h"
#include "constants.h"
#include "comittees.h"
#include "msg_queue.h"
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "opengl.h"

#ifdef __LINUX__
union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};
#endif


// initialize time
int currentTime = 0;
// initialize global variables for public fifo
int n, done, dummyfifo, publicfifo, privatefifo;

// get numberof planes
int numberOfPlanes;

// Structure to represent a container
typedef struct
{
    int planeId;
    float x, y;   // Position of the container
    int dropped;  // Flag indicating if the container is dropped
    int dropTime; // Time when the container was dropped
    float color[3];
} Container;

// Structure to represent a plane
typedef struct
{
    int planeId;
    float x, y;         // Position of the plane
    float xSpeed;       // Horizontal speed of the plane
    int dropTime;       // Time to drop the container
    int containerIndex; // Index of the container to drop
    int number_of_containers;
    Container *containers;
} Plane;

Plane *planes;

struct COLLECTING *collecting_commities;

// global variable for total amount
int total_amount;

// global varible for door color
float door_color[8][3];

// global count for timing colors
int global_count = 0;

// global variable for total number of slplit workers
int split_workers_number = 0;

// create distributing commite
struct DISTRIBUTING distributing_commite;

//global bags to distribute
int global_bags_to_distribute = 0;


void init()
{
    read_settings_from_a_file("settings.txt");
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Blue background (sky color)
    glClearColor(0.196f, 0.598f, 0.824f, 1.0f); // More intense blue background (sky color)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100.0f, 100.0f, -100.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // make opengl create the main process
    int pid;
    // create the main function using opengl
    if ((pid = fork()) == -1)
    {
        perror("error in fork\n");
        exit(-1);
    }
    else if (pid == 0)
    {
        execlp("./main", "main", NULL);
        perror("Error in exec\n");
        exit(-1);
    }
    else if (pid > 0)
    {

        FILE *pf;
        /*char command[20];
        char data[512];
        // remove public fifo using the command
        sprintf(command, "rm /tmp/PUBLIC");
        pf = popen(command, "r");
        fgets(data, 512, pf);
        */
        printf("first\n");

        if (access(PUBLIC, F_OK) != -1)
        {
            // File exists, remove it
            if (unlink(PUBLIC) == -1)
            {
                perror("Error removing existing FIFO");
                exit(EXIT_FAILURE);
            }
        }
        // create the PUBLIC fifo in opengl
        if ((mknod(PUBLIC, __S_IFIFO | 0777, 0)) == -1)
        {
            perror("Error");
            exit(-1);
        }
        printf("Second\n");
        //  create public fifo as read only in (opengl)
        if ((publicfifo = open(PUBLIC, O_RDONLY | O_NONBLOCK)) == -1)
        {

            perror(PUBLIC);
            exit(1);
        }
        printf("End fifo\n");
    }

    printf("intiated\n");

    // define planes
    planes = malloc(sizeof(Plane) * NUMBER_OF_CARGO_PLANES);
    collecting_commities = malloc(sizeof(struct COLLECTING) * COLLECTING_RELIEF_COMMITTEES);
    for (int i = 0; i < COLLECTING_RELIEF_COMMITTEES; i++)
    {
        collecting_commities[i].number_of_workers = -1;
        collecting_commities[i].workers = malloc(sizeof(struct WORKER) * COLLECTING_RELIEF_WORKERS);
    }

    for (int i = 0; i < NUMBER_OF_CARGO_PLANES; i++)
    {
        planes[i].planeId = -1;
    }

    // intiate doors colors
    for (int i = 0; i < 8; i++)
    {
        door_color[i][0] = 0.0;
        door_color[i][1] = 0.0;
        door_color[i][2] = 0.0;
    }
}
void timer(int value)
{
    currentTime++;

    glutPostRedisplay();
    glutTimerFunc(1000, timer, value + 1); // Increase time by 1 second
}
// Function to draw a rectangle
void drawRectangle(float x, float y, float width, float height)
{

    glColor3f(0.0f, 0.0f, 0.0f); // Black color
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void createBuilding()
{

    // Set color to white for the building
    glColor3f(1.0f, 1.0f, 1.0f);

    // Specify the vertices of the building
    float x = -25.0f;      // Left x-coordinate
    float y = -400.0f;     // Bottom y-coordinate
    float width = 50.0f;   // Width of the building
    float height = 400.0f; // Height of the building

    // Draw the building
    glBegin(GL_QUADS);
    glVertex2f(x, y);                  // Bottom-left corner
    glVertex2f(x + width, y);          // Bottom-right corner
    glVertex2f(x + width, y + height); // Top-right corner
    glVertex2f(x, y + height);         // Top-left corner
    glEnd();

    // Draw windows
    float windowWidth = 10.0f;   // Width of the window
    float windowHeight = 10.0f;  // Height of the window
    float windowSpacing = 20.0f; // Spacing between windows

    // Draw the windows on the building
    for (float windowY = y + windowHeight; windowY < y + height - windowHeight; windowY += windowSpacing)
    {
        for (float windowX = x + windowWidth; windowX < x + width - windowWidth; windowX += windowSpacing)
        {

            glBegin(GL_QUADS);
            // Set color to black for the windows
            glColor3fv(door_color);
            glVertex2f(windowX, windowY);                              // Bottom-left corner
            glVertex2f(windowX + windowWidth, windowY);                // Bottom-right corner
            glVertex2f(windowX + windowWidth, windowY + windowHeight); // Top-right corner
            glVertex2f(windowX, windowY + windowHeight);               // Top-left corner
            glEnd();
        }
    }
}

// Function to display text at a specific position
void drawText(float x, float y, const char *text)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y); // Set the position for the text
    // Display each character of the text
    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]); // Change font if needed
    }
}

// Function to draw a square
void drawSquare(float x, float y, float size, float color[3])
{
    glColor3fv(color);

    drawRectangle(x, y, size, size);
}

void drawGround()
{
    glColor3f(0.545f, 0.271f, 0.075f); // Brown color
    glBegin(GL_QUADS);
    glVertex2f(-100.0f, -100.0f); // Bottom-left corner
    glVertex2f(100.0f, -100.0f);  // Bottom-right corner
    glVertex2f(100.0f, -90.0f);   // Top-right corner
    glVertex2f(-100.0f, -90.0f);  // Top-left corner
    glEnd();
}

void drawTree(float x, float y)
{
    // Draw trunk (brown rectangle)
    glColor3f(0.545f, 0.271f, 0.075f); // Brown color
    glBegin(GL_QUADS);
    glVertex2f(x - 1.0f, y);     // Bottom-left corner
    glVertex2f(x + 1.0f, y);     // Bottom-right corner
    glVertex2f(x + 1.0f, y + 8); // Top-right corner
    glVertex2f(x - 1.0f, y + 8); // Top-left corner
    glEnd();

    // Draw foliage (green triangle)
    glColor3f(0.0f, 0.392f, 0.0f); // Green color
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 8);     // Top corner
    glVertex2f(x + 5.0f, y);  // Bottom-right corner
    glVertex2f(x - 5.0f, y);  // Bottom-left corner
    glEnd();
}

void drawTrees()
{
    // Draw some trees at different positions on the ground
    drawTree(-80.0f, -90.0f);
    drawTree(-60.0f, -90.0f);
    drawTree(-40.0f, -90.0f);
    drawTree(-20.0f, -90.0f);
    drawTree(0.0f, -90.0f);
    drawTree(20.0f, -90.0f);
    drawTree(40.0f, -90.0f);
    drawTree(60.0f, -90.0f);
    drawTree(80.0f, -90.0f);
}

void drawCircle(float cx, float cy, float radius)
{
    int num_segments = 100; // Number of segments to approximate the circle
    glColor3f(1.0f, 0.87f, 0.77f); // Skin color
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * i / num_segments; // Get the current angle
        float x = radius * cosf(theta);                     // Calculate the x component
        float y = radius * sinf(theta);                     // Calculate the y component
        glVertex2f(x + cx, y + cy);                         // Output vertex
    }
    glEnd();
}




void drawFamily(float x, float y)
{
    // Draw head (circle)
    glColor3f(1.0f, 0.87f, 0.77f); // Skin color
    drawCircle(x, y + 4.0f, 3.0f); // Head

    // Draw body (rectangle)
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color
    drawRectangle(x - 1.0f, y - 2.0f, 2.0f, 4.0f); // Body

    // Draw legs (rectangles)
    drawRectangle(x - 1.0f, y - 6.0f, 1.0f, 4.0f); // Left leg
    drawRectangle(x, y - 6.0f, 1.0f, 4.0f);       // Right leg
}




// define current planes initialized
int current_planes = 0;
void display()
{

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGround();
    drawTrees();
    // Draw first family
    drawFamily(90.0f, -80.0f);
    // Draw second family
    drawFamily(85.0f, -80.0f);
    // Draw third family
    drawFamily(80.0f, -80.0f);
    // Draw fourth family
    drawFamily(75.0f, -80.0f);

    // begin display of opengl
    struct message msg;

    if (read(publicfifo, (char *)&msg, sizeof(msg)) > 0)
    {
        printf("Read:%s\n", msg.cmd_line);

        // read message and divide each part
        char message[100];
        strcpy(message, msg.cmd_line);
        printf("%s\n", msg.cmd_line);
        char firstLetter = msg.cmd_line[0];

        char *allString;
        allString = strtok(msg.cmd_line, ",");
        char letter = allString[0];

        // if first letter p --> plane created
        if (firstLetter == 'p')
        {

            printf("plane read\n");
            allString = strtok(NULL, ",");
            int planeId = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_containers = atoi(allString);
            allString = strtok(NULL, ",");
            int refill_period = atoi(allString);
            allString = strtok(NULL, ",");
            int drop_time = atoi(allString);
            // get parameter of plane

            planes[planeId].planeId = planeId;
            planes[planeId].x = -80;
            planes[planeId].xSpeed = (NUMBER_OF_CARGO_PLANES- planeId) + 3;
            planes[planeId].y = 50;
            planes[planeId].dropTime = drop_time;
            planes[planeId].number_of_containers = number_of_containers;
            // create containers;

            planes[planeId].containers = malloc(sizeof(Container) * sizeof(number_of_containers));
            printf("NumofCont = %d\n", number_of_containers);
            for (int i = 0; i < number_of_containers; i++)
            {
                planes[planeId].containers[i].dropped = -1;
                planes[planeId].containers[i].x = planes[planeId].x;
                planes[planeId].containers[i].y = planes[planeId].y;
                planes[planeId].containers[i].dropTime = currentTime;
            }
        }
        else if (firstLetter == 'c')
        {

            allString = strtok(NULL, ",");
            int planeId = atoi(allString);
            allString = strtok(NULL, ",");
            int containerId = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_kgs = atoi(allString);

            planes[planeId].containers[containerId].dropped = 1;
            planes[planeId].containers[containerId].x = planes[planeId].x;
            planes[planeId].containers[containerId].y = planes[planeId].y;

            // if drop red color
            planes[planeId].containers[containerId].color[0] = 1.0;
            planes[planeId].containers[containerId].color[1] = 0.0;
            planes[planeId].containers[containerId].color[2] = 0.0;

            planes[planeId].containers[containerId].dropTime = planes[planeId].dropTime;

            glColor3fv(planes[planeId].containers[containerId].color);
        }
        else if (firstLetter == 's')
        {
            allString = strtok(NULL, ",");
            int planeId = atoi(allString);
            allString = strtok(NULL, ",");
            int containerId = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_kgs = atoi(allString);

            // planes[planeId].containers[containerId].dropped = 1;
            // planes[planeId].containers[containerId].x = planes[planeId].x;
            // planes[planeId].containers[containerId].y = planes[planeId].y;

            // if it is shot will got yellow color
            planes[planeId].containers[containerId].color[0] = 1.0;
            planes[planeId].containers[containerId].color[1] = 1.0;
            planes[planeId].containers[containerId].color[2] = 0.0;
        }
        else if (firstLetter == 'r')
        {
            allString = strtok(NULL, ",");
            int planeId = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_containers = atoi(allString);

            planes[planeId].planeId = -1;
            printf("REFILL\n");
        }
        else if (firstLetter == 'e')
        {
            // begin refill again

            allString = strtok(NULL, ",");
            int planeId = atoi(allString);
            allString = strtok(NULL, ",");

            int number_of_containers = atoi(allString);
            allString = strtok(NULL, ",");
            int drop_time = atoi(allString);
            free(planes[planeId].containers);
            planes[planeId].planeId = planeId;
            planes[planeId].x = -80 ;
            planes[planeId].xSpeed = (NUMBER_OF_CARGO_PLANES- planeId) + 3;
            planes[planeId].y = 50;
            planes[planeId].dropTime = drop_time;
            planes[planeId].number_of_containers = number_of_containers;
            // create containers;

            planes[planeId].containers = malloc(sizeof(Container) * sizeof(number_of_containers));
            printf("NumofCont = %d\n", number_of_containers);
            for (int i = 0; i < number_of_containers; i++)
            {
                planes[planeId].containers[i].dropped = -1;
                planes[planeId].containers[i].x = planes[planeId].x;
                planes[planeId].containers[i].y = planes[planeId].y;
                planes[planeId].containers[i].dropTime = currentTime;
            }
        }
        else if (firstLetter == 'o')
        {
            // creating collecting team commitie
            allString = strtok(NULL, ",");
            int commitie_id = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_workers = atoi(allString);

            collecting_commities[commitie_id].number_of_workers = number_of_workers;

            // determine collecting team color
            // give it green color in normal case
            collecting_commities[commitie_id].color[0] = 0.0;
            collecting_commities[commitie_id].color[1] = 1.0;
            collecting_commities[commitie_id].color[2] = 0.0;
        }
        else if (firstLetter == 'd')
        {
            // dead worker in collecting team
            allString = strtok(NULL, ",");
            int commitie_id = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_workers = atoi(allString);

            collecting_commities[commitie_id].number_of_workers = number_of_workers;
        }
        else if (firstLetter == 'C')
        {
            // change color of collecting commitie when recive container
            allString = strtok(NULL, ",");
            int commitie_id = atoi(allString);
            allString = strtok(NULL, ",");
            int number_of_workers = atoi(allString);
            collecting_commities[commitie_id].number_of_workers = number_of_workers;

            // change color to red when recive
            collecting_commities[commitie_id].color[0] = 1.0;
            collecting_commities[commitie_id].color[1] = 0.0;
            collecting_commities[commitie_id].color[2] = 0.0;
        }
        else if (firstLetter == 'S')
        {
            allString = strtok(NULL, ",");
            int commitie_id = atoi(allString);
            allString = strtok(NULL, ",");
            int amount = atoi(allString);

            // rechange color of collecting commitie when store container
            collecting_commities[commitie_id].color[0] = 0.0;
            collecting_commities[commitie_id].color[1] = 1.0;
            collecting_commities[commitie_id].color[2] = 0.0;

            // and print number of total amount in storage
            total_amount = amount;

            // change door color -->green
            door_color[0][0] = 0.0;
            door_color[0][1] = 1.0;
            door_color[0][2] = 0.0;
            global_count = 0;
        }
        else if (firstLetter == 'T')
        {
            // get splitting team informations
            allString = strtok(NULL, ",");
            int number_of_workers = atoi(allString);
            allString = strtok(NULL, ",");
            int splitting_team_id = atoi(allString);
            split_workers_number = number_of_workers;
        }
        else if (firstLetter == 'D')
        {
            // get distributing team informations
            allString = strtok(NULL, ",");
            int number_of_workers = atoi(allString);
            allString = strtok(NULL, ",");
            int distributing_team_id = atoi(allString);

            distributing_commite.workers = malloc(sizeof(struct WORKER) * number_of_workers);

            for (int i = 0; i < number_of_workers; i++)
            {

                // initialize color to yellow
                distributing_commite.workers[i].color[0] = 1.0;
                distributing_commite.workers[i].color[1] = 1.0;
                distributing_commite.workers[i].color[1] = 0.0;
            }
        }
        else if(firstLetter == 'b'){
            allString = strtok(NULL , ",");
            int number_of_bags = atoi(allString);
            
            global_bags_to_distribute = number_of_bags;

        }
        else
        {
            //   break;
        }

        //  break;
    }
    global_count += 1;
    if (global_count >= 4)
    {
        global_count = 0;
        // change door color -->black
        door_color[0][0] = 0.0;
        door_color[0][1] = 0.0;
        door_color[0][2] = 0.0;
    }

    for (int i = 0; i < NUMBER_OF_CARGO_PLANES; i++)
    {

        if (planes[i].planeId != -1)
        {

            drawRectangle(planes[i].x, planes[i].y, 7, 3);
            // Update the plane's position
            planes[i].x += planes[i].xSpeed;
            if (planes[i].x > 100)
            {
                planes[i].x = -100;
            }
            for (int j = 0; j < planes[i].number_of_containers; j++)
            {
                if (planes[i].containers[j].dropped == 1)
                {
                    float newY = planes[i].containers[j].y - 17;
                    if (newY > -100)
                    {
                        planes[i].containers[j].y = newY;
                        glColor3fv(planes[i].containers[j].color);

                        glBegin(GL_QUADS);
                        glVertex2f(planes[i].containers[j].x - 2, planes[i].containers[j].y - 1);
                        glVertex2f(planes[i].containers[j].x + 2, planes[i].containers[j].y - 1);
                        glVertex2f(planes[i].containers[j].x + 2, planes[i].containers[j].y - 5);
                        glVertex2f(planes[i].containers[j].x - 2, planes[i].containers[j].y - 5);
                        glEnd();
                    }
                    /*else
                    {
                        planes[i].containers[j].dropped = 2;
                    }*/
                }
            }
        }
    }
    char amount_string[20];
    sprintf(amount_string, "Amount=%d kg", total_amount);
    char total_splitting_workers[20];

    char bags_to_distribute[20];
    sprintf(bags_to_distribute , "BagsTo Distribute=%d" , global_bags_to_distribute );
    sprintf(total_splitting_workers, "Splitting Workers=%d", split_workers_number);
    drawText(-10.0, 10.0, "Storage Building");
    drawText(-10.0, 15.0, amount_string);
    drawText(-10.0, 20.0, total_splitting_workers);
    drawText(20.0 ,5.0 , bags_to_distribute );

    createBuilding();

    // Draw the collecting committees
    for (int i = 0; i < COLLECTING_RELIEF_COMMITTEES; i++)
    {
        if (collecting_commities[i].number_of_workers != -1)
        {
            printf("Print collecting team\n");

            glBegin(GL_QUADS);
            // Calculate the position and size of the square for the current committee
            float x = -90.0f + (i * 18); // Adjust the x position to be to the right of the building
            float y = -95.0f;            // Adjust the y position according to the number of committees
            int size = 5;

            // Draw the square for the collecting team
            // Draw the square for the collecting team
            glColor3fv(collecting_commities[i].color);

            glVertex2f(x - size / 2, y - size / 2);
            glVertex2f(x + size / 2, y - size / 2);
            glVertex2f(x + size / 2, y + size / 2);
            glVertex2f(x - size / 2, y + size / 2);
            glEnd();

            // Draw the number of workers text
            char number_of_workers_string[20];
            sprintf(number_of_workers_string, "CWorkers=%d", collecting_commities[i].number_of_workers);
            drawText(x, y + 12.0f, number_of_workers_string);
        }
    }

    glutSwapBuffers();
}

int main(int argc, char **argv)
{

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("North Gaza(Resistance is a continuous feasibility)");

    printf("Before init\n");
    // Initialize OpenGL
    init();
    printf("After init\n");

    // Set up the timer
    glutTimerFunc(500, timer, 0);

    // Register callbacks
    glutDisplayFunc(display);

    // Start the main loop
    glutMainLoop();
}
