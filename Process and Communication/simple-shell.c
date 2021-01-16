/**
 * Simple shell interface program.
 * Operating System Concepts - Ninth Edition
 * Copyright John Wiley & Sons - 2013
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE		80 /* 80 chars per line, per command */

int main(void)
{
	char* args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    	int should_run = 1;
	int counter = 0;
	int result = 1;
	int end = 1;
	int esc = 1;	
	char* start[40];
	char* later[40];
	
	int i, upper;
	char temp;
	i = 0;

	char previous[500];
	
    while (should_run){   
        printf("osh>");
        fflush(stdout);
	char str[500];
	fgets(str, 500, stdin);
	//strcpy(previous, str);
	
	esc = 1;
	int length = 0; 
	char* storage;
	storage = strtok(str, " \n");
	while(storage != NULL)
	{args[counter] = storage;
	storage = strtok(NULL, " \n");
	counter++;
	length ++;
	}
	
	int y;
	int sign =1;
	for(y=0; y<length; y++)
	{sign = strcmp(">", args[y]);
	if(sign ==0){break;} }
	// Sign now holds whether > is there or not

	int z;
	int piping = 1;
	for(z=0; z<length; z++)
	{piping = strcmp("|", args[z]);
	if(piping == 0){break;} }

	if(piping == 0){
			
		int a;
		for(a=0; a<z; a++)
		{start[a] = args[a];}
		start[z] = NULL;
		
		int b;
		int c =0;
		for(b=z+1; b<length; b++)
		{later[c] = args[b];
		c++;	}
		later[c] = NULL;
	}
		
	end = strcmp("exit", args[0]);
	esc = strcmp("!!", args[0]);

	result = strcmp(args[counter-1], "&");
	if(result == 0)
	{args[counter-1] = NULL;}
	else {args[counter] = NULL;}
	
	
	

	if(end == 0)
	{should_run = 0; 

	}
	else {
	
		if(esc == 0)// !! sent
			{ 
			if(counter == 1)
			{printf("No history available \n");
			counter = 0;}
			else
			{ counter = 0;
			char * update = strtok(previous, " \n");
			while(update != NULL)
			{args[counter] = update;
			update = strtok(NULL, " \n");
			counter++;} //end of while
			result = strcmp(args[counter-1], "&");
			if(result == 0)
			{args[counter-1] = NULL;}
			else {args[counter] = NULL;}
		
			int pid_val = fork();
			if(pid_val == 0) {execvp(args[0], args);}
			else{ if(result != 0) {wait(NULL);} }	
							} // end of inner else
			
			} // end of esc = 0
		
		else if(sign == 0)
		{	
			strcpy(previous, str);
			int fd = open(args[length-1], O_WRONLY | O_CREAT | O_TRUNC, 5000 );
			args[length-1] = NULL;
			args[length-2] = NULL;
			 			
			int pid_val = fork();
		
			if(pid_val < 0){return 0;}
			else if(pid_val == 0){
			dup2(fd, STDOUT_FILENO);
			execvp(args[0], args);}
			else{ if(result != 0) {wait(NULL);} }
			
			close(fd);
			continue;
		}
		
		else if(piping == 0)
		{	
			
			strcpy(previous, str);
			int fd[2];
			int pid_val = fork();

			pipe(fd);

			if(pid_val == 0) {
			int pid_val2 = fork();
			if(pid_val2 == 0)
				{	
					dup2(fd[0], 0);
					close(fd[1]);
					execvp(later[0], later);
				}
			else if (pid_val2 >0)
				{
					dup2(fd[1], 1);
					close(fd[0]);
					execvp(start[0], start);			
				}
			continue;
			}
			 else{if(result != 0) {wait(NULL);}}
		}
		
		else {
		
		strcpy(previous, str);
		int pid_val = fork();

		if(pid_val < 0)
		{printf("Fork Failed");
		return 0;}
		else if(pid_val == 0)
		{ execvp(args[0], args);}
	 	else{
		if(result != 0)
		{wait(NULL);}	
			}
		}	
	
	} // of else 
		       
	} // outer while
    
	return 0;
}
