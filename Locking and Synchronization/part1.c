#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "part1.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */


/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;
const int MAX_NUM_FLOORS = 20;
sem_t* Outside_elev;
sem_t* Inside_elev;
sem_t* Printer; 

sem_t* Press_button;
//sem_t* Press_button2;
//sem_t* Shift;

//int turn; 
int Max_Capacity;
int pick_from[20]; // Checks the number of people waiting at each floor 
int where_to[20]; // Checks the number of people that want to step out at a particular floor
int in_lift; // total number of people in the lift at any instance. To ccompare with _elevator capacity
int totalPeopleWaitingForLiftAtAnyFloor; // waiting outside floor
//int num_waiting;
//int num_inside;
//int Counter[MAX_NUM_FLOORS] = {0};


/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numFloors: Total number of floors elevator can go to. numFloors will be smaller or equal to MAX_NUM_FLOORS
 * maxNumPeople: The maximum capacity of the elevator
 *
 */
void initializeP1(int numFloors, int maxNumPeople) {
	// example_semaphore = (sem_t*) malloc(sizeof(sem_t)); 
	Max_Capacity = maxNumPeople;
	in_lift = 0;
	totalPeopleWaitingForLiftAtAnyFloor = 0;
	
	Outside_elev = malloc(sizeof(sem_t) * numFloors);
	Inside_elev = malloc(sizeof(sem_t) * numFloors);
	Printer = malloc(sizeof(sem_t));
	Press_button = malloc(sizeof(sem_t));
	//Press_button2 = malloc(sizeof(sem_t));

	sem_init(Press_button, 0,1);
	//sem_init(Press_button2, 0,1);
	sem_init(Printer,0,1);

	int i;
	for(i=0; i<MAX_NUM_FLOORS; i++){
		sem_init(&Outside_elev[i],0,0); 
		sem_init(&Inside_elev[i],0,0);
		pick_from[i] = 0;
		where_to[i] = 0;
	}


	return;
}



/**
 * Every passenger will call this function when 
 * he/she wants to take the elevator. (Already
 * called in main.c)
 * 
 * This function should print info "id from to" without quotes,
 * where:
 * 	id = id of the user (would be 0 for the first user)
 * 	from = source floor (from where the passenger is taking the elevator)
 * 	to = destination floor (floor where the passenger is going)
 * 
 * info of a user x_1 getting off the elevator before a user x_2
 * should be printed before.
 * 
 * Suppose a user 1 from floor 1 wants to go to floor 4 and
 * a user 2 from floor 2 wants to go to floor 3 then the final print statements
 * will be 
 * 2 2 3
 * 1 1 4
 *
 */
void* goingFromToP1(void *arg) {
	struct argument *user = (struct argument *) arg;
	int U_id = user->id;
	int U_from = user->from;
	int U_to = user->to;


	//Counter[U_from] = Counter[U_from] + 1;
	sem_wait(Press_button);
		pick_from[user->from]++;
		totalPeopleWaitingForLiftAtAnyFloor++;
	sem_post(Press_button);

	sem_wait(&Outside_elev[user->from]);
	
	sem_wait(Press_button);
		pick_from[user->from]--;
		where_to[user->to]++;
		totalPeopleWaitingForLiftAtAnyFloor--;
		in_lift++;
	sem_post(Press_button);

	sem_post(Printer);

	sem_wait(&Inside_elev[user->to]);
	
	sem_wait(Press_button);
		where_to[user->to]--;
		in_lift--;
	sem_post(Press_button);

	printf("%d %d %d\n", U_from, U_from, U_to);

	// int z;
	// for(z=0; z< MAX_NUM_FLOORS; z++){
	// 		if(z!= user->to)
	// 		{
	// 		sem_post(&Inside_elev[z]); 
	// 		}
	// 	}
	sem_post(Printer);
	

	return NULL;
}

/*If you see the main file, you will get to 
know that this function is called after setting every
passenger.

So use this function for starting your elevator. In 
this way, you will be sure that all passengers are already
waiting for the elevator.
*/


void* ElevFunc(void *arg){
	int x;
	
	while(1){	
	
	for(x=0; x<MAX_NUM_FLOORS; x++)
	{	
		while(where_to[x] > 0){	// only stop when a person want to move out of the floor
			sem_post(&Inside_elev[x]);
			sem_wait(Printer); 
		}
		
		while(pick_from[x] > 0){ // only stop if a person is waiting at the floor
			if (in_lift < Max_Capacity) // check if lesser than capacity
			{
				sem_post(&Outside_elev[x]);
				sem_wait(Printer);
			}
			else
			{
				break;
			}
		}
	}

	int j;
	for(j=MAX_NUM_FLOORS-2; j>=0; j--)
	{
		while(where_to[j] > 0){ // only stop when a person want to move out of the floor
			sem_post(&Inside_elev[j]);
			sem_wait(Printer); 
		}
		
		while(pick_from[j] > 0){ // only stop if a person is waiting at the floor
			if (in_lift < Max_Capacity) // check if lesser than capacity
			{
				sem_post(&Outside_elev[j]);
				sem_wait(Printer);
			}
			else
			{
				break;
			}
		}
	}

	sem_wait(Press_button);
	if (totalPeopleWaitingForLiftAtAnyFloor == 0 && in_lift == 0) // check if all users have been picked and dropped
	{
		// sem_post(Press_button);
		break; // end 
	}
	sem_post(Press_button);

	x = 1;

	}

pthread_exit(NULL);
return NULL;

}


void startP1(){
	sleep(1); // This is the only place where you are allowed to use sleep
	pthread_t Elevator;
	pthread_create(&Elevator, NULL, ElevFunc, NULL);
	pthread_join(Elevator, NULL);
}
