#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "part2.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>

/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;
const int INTER_ARRIVAL_TIME = 5;
const int NUM_TRAINS = 5;

sem_t *stationSem; //array of semaphores. one for each station
sem_t *allTrainsInsideWaitingSem;
int totalStations;
int perTrainMaxPeople;
/**
 * numStations: Total number of stations. Will be >= 5. Assume that initially
 * the first train is at station 1, the second at 2 and so on. (=6)
 * maxNumPeople: The maximum number of people in a train (=10)
 */
void initializeP2(int numStations, int maxNumPeople)
{
	totalStations = numStations;
	perTrainMaxPeople = maxNumPeople;

	int totalSemForTrains = numStations * NUM_TRAINS;
	allTrainsInsideWaitingSem = (sem_t *)malloc(sizeof(sem_t) * totalSemForTrains);

	//for each train, allocate 1 semaphore for each station
	for (int i = 0; i < totalSemForTrains; i++)
	{
		//for iTh train,allocate numStations semaphores for people to wait in corresponding to which station they want to get off at
		sem_init(&(allTrainsInsideWaitingSem[i]), 0, 0);
	}
	//allocate memory of all semaphores for stations. 1 per station. User waits in station's own semaphore before boarding train
	stationSem = (sem_t *)malloc(sizeof(sem_t) * numStations);
	for (int i = 0; i < numStations; i++)
	{
		sem_init(&stationSem[i], 0, 0);
	}
}

/**
	This function is called by each user.

 * Print in the following format:
 * If a user borads on train 0, from station 0 to station 1, and another boards
 * train 2 from station 2 to station 4, then the output will be
 * 0 0 1
 * 2 2 4
 */
void *goingFromToP2(void *user_data)
{
	struct argument *user = (struct argument *)user_data;
	int trainID = user->id; //ID of train boarded by user
	int fromStation = user->from;
	int toStation = user->to;

	//wait in boarding area of station fromStation. a train comes to this station and calls signal i.e. user can board the train
	sem_wait(&(stationSem[fromStation]));
	//user now waits inside the train which it boarded and waits in queue to reach toStation
	int offset = totalStations * trainID + toStation;

	sem_wait(&(allTrainsInsideWaitingSem[offset]));
	printf("%d %d %d\n", trainID, fromStation, toStation);
}

void *startTrain(void *param)
{
	struct argument *trainInfo = (struct argument *)param;

	int myID = trainInfo->id;

	for (int i = 0; i < totalStations; i++)
	{
		int currStation = (i + myID) % totalStations;
		int offset = totalStations * myID + currStation;
		sem_post(&allTrainsInsideWaitingSem[offset]);
		sem_post(&stationSem[currStation]);
	}
	return NULL;
}

/* Use this function to start threads for your trains */
void *startP2()
{
	sleep(1); // This is the only place where you are allowed to use sleep

	for (int i = 0; i < NUM_TRAINS; i++)
	{
		pthread_t tid;
		struct argument *arg = (struct argument *)malloc(sizeof(struct argument));
		arg->from = -1;
		arg->to = -1;
		arg->id = i; //train starts at iTh station
		pthread_create(&tid, NULL, startTrain, (void *)arg);
	}

	return NULL;
}