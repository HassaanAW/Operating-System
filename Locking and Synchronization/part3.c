#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "part3.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;

const char *getDirectionName(enum DIRECTION d)
{
	switch (d)
	{
	case 0:
		return "NORTH";
	case 1:
		return "SOUTH";
	case 2:
		return "EAST";
	case 3:
		return "WEST";
		/* etc... */
	}
}

const char *getLaneName(enum LANE l)
{

	switch (l)
	{
	case 0:
		return "LEFT";
	case 1:
		return "RIGHT";
	}
}

sem_t *laneSem;
sem_t *laneCountSem;
sem_t *printer;
sem_t *carGone;
int counters[8];
int totalCars = 0;
sem_t *totalCarsSem;

/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here.
 */
void initializeP3()
{
	laneSem = (sem_t *)malloc(sizeof(sem_t) * 8);
	laneCountSem = (sem_t *)malloc(sizeof(sem_t) * 8);
	printer = (sem_t *)malloc(sizeof(sem_t));
	carGone = (sem_t *)malloc(sizeof(sem_t));
	totalCarsSem = (sem_t *)malloc(sizeof(sem_t));

	sem_init(printer, 0, 1);
	sem_init(carGone, 0, 0);
	sem_init(totalCarsSem, 0, 1);

	int i;
	for (i = 0; i < 8; i++)
	{
		sem_init(&laneSem[i], 0, 0);
		sem_init(&laneCountSem[i], 0, 1);
		counters[i] = 0;
	}

	return;
}

bool iWantToTurnLeft(int from, int to)
{
	//east turns left to south
	if (from == 2 && to == 1)
	{
		return true;
	}
	//south turns left to west
	if (from == 1 && to == 3)
	{
		return true;
	}
	//west turns left to north
	if (from == 3 && to == 0)
	{
		return true;
	}
	//north turns left to east
	if (from == 0 && to == 2)
	{
		return true;
	}
	return false;
}

/**
 * This is the function called for each car thread. You can check
 * how these functions are used by going over the test3 function
 * in main.c
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print (FROM TO LANE)
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 *
 * Input: *argu is of type struct argumentP3 defined in main.h
 
struct argumentP3 {
    enum DIRECTION from;
    enum DIRECTION to;
    enum LANE lane;
    int user_id;
};
 */
void *goingFromToP3(void *argu)
{

	// Some code to help in understanding argu
	struct argumentP3 *car = (struct argumentP3 *)argu;
	enum DIRECTION from = car->from;
	enum DIRECTION to = car->to;
	enum LANE lane = car->lane;

	int offset = (from * 2) + lane;

	sem_wait(totalCarsSem);
	totalCars++;
	sem_post(totalCarsSem);

	sem_wait(&laneCountSem[offset]);
	counters[offset]++;

	// if lane is left (0) and cars in lane == 1 (only me) and I want to turn left
	// then I dont wait for the light in my lane
	// just skip and print and decrease total and current lane count and leave
	bool waitForLight = true;
	if (lane == 0 && counters[offset] == 1 && (iWantToTurnLeft(from, to) == true))
	{
		sem_post(&laneCountSem[offset]);
		waitForLight = false;
	}
	else
	{
		sem_post(&laneCountSem[offset]);
		sem_wait(&laneSem[offset]);
	}

	// printf("%s %s %s\n", getDirectionName(from), getDirectionName(to), getLaneName(lane));

	// printf("Counter %d : count value: %d\n", offset, counters[offset]);

	sem_wait(printer);
	printf("%s %s %s\n", getDirectionName(from), getDirectionName(to), getLaneName(lane));
	sem_post(printer);

	sem_wait(totalCarsSem);
	totalCars--;
	sem_post(totalCarsSem);

	sem_wait(&laneCountSem[offset]);
	counters[offset]--;
	sem_post(&laneCountSem[offset]);

	if (waitForLight == true)
	{

		sem_post(carGone);
	}

	// ...
}

int allowedCountCalculator(int offset)
{
	int numCars = 0;

	sem_wait(&laneCountSem[offset]);
	numCars = counters[offset];
	sem_post(&laneCountSem[offset]);

	//we only allow maximum of 5 cars to leave from a lane in a given round
	if (numCars > 5)
	{
		numCars = 5;
	}

	return numCars;
}

void *Traffic(void *arg)
{
	/*
		we start from East end then south then west then north
		within any road we start from left lane then right

		specific unique integer is given to each lane and each end
		we can calculate an offset by combining both these ints such that each lane of the 8 possible lanes
		can be uniquely identified by an int. (2*Direction + Lane = unique int offset)

		the counters array and laneSem is indexed using this unique int as per the requirement of the assignment

	*/

	int currTotalCars = 0;
	//this would be 8 in the current test at the start
	//after 1 round completes we again check if there are any cars at the intersection
	// in order to start the round of letting cars go in an order , once again
	sem_wait(totalCarsSem);
	currTotalCars = totalCars;
	sem_post(totalCarsSem);

	while (currTotalCars > 0)
	{
		//0,2,4,6 are offsets of left lanes of different roads.

		// printf("now going to allow east left\n");

		//EAST
		int allowedToGoCarCount = allowedCountCalculator(4);

		int i;
		for (i = 0; i < allowedToGoCarCount; i++) // East Left
		{
			sem_post(&laneSem[4]);
			sem_wait(carGone);
		}

		allowedToGoCarCount = allowedCountCalculator(5);

		// printf("now going to allow east right\n");
		for (i = 0; i < allowedToGoCarCount; i++) // East Right
		{
			sem_post(&laneSem[5]);
			sem_wait(carGone);
		}

		//SOUTH

		allowedToGoCarCount = allowedCountCalculator(2);

		// printf("now going to allow south left\n");
		int j;
		for (j = 0; j < allowedToGoCarCount; j++) // South Left
		{
			sem_post(&laneSem[2]);
			sem_wait(carGone);
		}

		allowedToGoCarCount = allowedCountCalculator(3);

		// printf("now going to allow south right\n");
		for (j = 0; j < allowedToGoCarCount; j++) // South Right
		{
			sem_post(&laneSem[3]);
			sem_wait(carGone);
		}

		//WEST

		allowedToGoCarCount = allowedCountCalculator(6);

		// printf("now going to allow west left\n");
		int k;
		for (k = 0; k < allowedToGoCarCount; k++) // West Left
		{
			sem_post(&laneSem[6]);
			sem_wait(carGone);
		}

		allowedToGoCarCount = allowedCountCalculator(7);

		// printf("now going to allow west right\n");
		for (k = 0; k < allowedToGoCarCount; k++) // West Right
		{
			sem_post(&laneSem[7]);
			sem_wait(carGone);
		}

		//NORTH

		allowedToGoCarCount = allowedCountCalculator(0);

		for (i = 0; i < allowedToGoCarCount; i++) // East Left
		{
			sem_post(&laneSem[0]);
			sem_wait(carGone);
		}

		allowedToGoCarCount = allowedCountCalculator(1);

		// printf("now going to allow east right\n");
		for (i = 0; i < allowedToGoCarCount; i++) // East Right
		{
			sem_post(&laneSem[1]);
			sem_wait(carGone);
		}

		sem_wait(totalCarsSem);
		currTotalCars = totalCars;
		sem_post(totalCarsSem);
	}

	// pthread_exit(NULL);
	return NULL;
}

/**
 * startP3 is called once all cars have been initialized. The logic of the traffic signals
 * will go here
 */
void startP3()
{
	sleep(1);
	pthread_t Lights;
	// printf("Now starting traffic thread\n");
	pthread_create(&Lights, NULL, Traffic, NULL);
	pthread_join(Lights, NULL);
	//NEED TO MAKE 4 TRAFFIC LIGHTS. ONE EACH FOR TRAFFIC COMING FROM DIFFERENT DIRECTIONS
	//
}