#ifndef PART_1
#define PART_1

struct User_info{
	int ID;
	int from;
	int to;
};


void initializeP1(int numFloors, int maxNumPeople);
void *goingFromToP1(void *);
void startP1();
void* ElevFunc(void *);

#endif
