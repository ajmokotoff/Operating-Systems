//Author: Andrew Mokotoff

/*
This shell is unique from a standard bash shell. It is lacking many functions built into conventional shells. The largest problem with this shell was the lack of the ability to pipe and direct output straight to a file. In regular bash, one can hit the up arrow to repeat previous commands. This does not have support for that, and it would have been very convienent if it were! Finally, it does not let me change directory to all locations that you could in a regular shell.
*/


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

/* Created a process structure to track start time for CPU wall time, pid to differentiate the process, name of the process to print, and a pointer to the next.
* Alive is also included so that if a process needs to be printed, it can.
*/
struct process
{
	long start;
	pid_t pid;
	char* name;
	struct process* next;
	int alive;
};

struct process* head; //global var that points at the head of the data structure
struct process* cur_proc; //the current process

/* Given the specific process struct, this function prints all the neccessary data about that process,
* @param p the structure containing the pid, name, and etc about the process in question
*/

void print_details(struct process* p)
{
	struct timeval t;
	long time;
	struct rusage ru;
	struct rusage *ru_p = &ru;

	getrusage(RUSAGE_SELF, ru_p);
	gettimeofday(&t, NULL);
	time = ((t.tv_sec/1000) + (t.tv_usec*1000));

	printf("Process Name: %s\n", p->name);
	printf("User Time Used: %8ld\nSystem Time Used: %8ld\nWall Clock Time Used: %8ld\nInvoluntary Context Switches: %8ld\nVoluntary Context Switches: %8ld\nMajor Page Faults: %8ld\nMinor Page Faults: %8ld\n\n", ru_p->ru_utime.tv_sec/1000 + ru_p->ru_utime.tv_usec * 1000, ru_p->ru_stime.tv_sec/1000 +  ru_p->ru_stime.tv_usec*1000, time-p->start, ru_p->ru_nivcsw, ru_p->ru_nvcsw, ru_p->ru_majflt, ru_p->ru_minflt);

}

/* Starting from the head, check all processes and look for ones that are dead, this is how we find dead background processes
* @param p the head structure, all processes can be found from traversing here
*/

void check_procs(struct process* p)
{
	while(p!=NULL){ //iterate through all processes looking for ones still alive
		if(kill(p->pid, 0)) //look to see if its alive
		{
			if(p->alive){
				p->alive = 0; // if alive, kill and then print details
				print_details(p);
			}
		}
		p=p->next;
	}
}

/* Starts by taking in the desired commands from argv, and then forks the process. Once it is forked, the child will then exec a new process.
* It also accounts for errors and handles them accordingly.
* @param **argv The user input, can be from the command line, or from the execution
* @param bg Determines if the process was a background process
*/

void exec(char **argv, int bg){
	pid_t pid;
	struct rusage ru;
	struct rusage *ru_p = &ru;
	struct timeval utime, stime, time0;
	long time;
	int i;
	struct process* p = head;
	

	if ((pid = fork()) < 0){ //forked a child process
		fprintf(stderr, "Fork error\n");
		exit(1);
	}
	else if (!pid){ //for child process, child will end up here
		if (execvp(argv[0], argv) < 0){	//exec a process to execute the command
			printf("Error\n");
			exit(1);
		}
		//exit(0);
	}
	else{
		gettimeofday(&time0, NULL); //start logging the time, so it can be compared against later for CPU wall time
		time = ((time0.tv_sec/1000) + (time0.tv_usec*1000)); //put seconds and usec into one unit and combine
		struct process* proc = malloc(sizeof(struct process)); //allocate memory for a process struct
		proc->start = time; //fill up proc struct with vars
		proc->pid = pid;
		proc->name = argv[0];
		if(bg){ //if a background process, do not wait and proceed
			proc->alive = 1;
			cur_proc->next = proc;
			cur_proc = proc;
			check_procs(p);
			return;
		}
		else{ //not a background process, thus wait for the process to be killed
			waitpid(pid, 0, 0);
			print_details(proc);
		}
	}
}

/* Parses argv correctly using the user input, so that execvp will work correctly. This requires removing spaces, \n, \t, with null terminators.
* In addition, the next blocks are setup for the next input.
* @param input The user input from the continuous command line
* @param input The global argv value, we can simply modify this variable to make everything else work correcty.
*/

void parse_argv(char *input, char **argv){
	while(*input != '\0'){ //traverse if not at the end of hte line
		while(*input == ' ' || *input == '\n' || *input == '\t'){
			*input++ = '\0'; //replace all the white space with null terminators
		}
		*argv++ = input; //record the argument position
		while(*input != '\0' && *input != ' ' && *input != '\n' && *input != '\t'){
			input++; //skip the argument
		}
	}
	*argv = '\0'; //mark the end of an argument list
}


/* argc -- number of arguments */
/* argv -- an array of strings */
void main(int argc, char* argv[32])
{
	char input[128]; //variable to read in teh command line
	int bg; //variable to designate background vs forgeround processes
	int i = 0; //variable used as an iterator
	head = malloc(sizeof(struct process)); //allocate memory for the head of the process struct
	head->pid = 0;
	head->alive = 0;
	struct process* p = head;
	cur_proc = head; //use head as the initial current process

	if (argc > 1){ //if arguments are entered upon launch exec them here
		if (!strcmp(argv[1], "exit")){ //check for exit
			printf("Exiting...\n");
			exit(0);
		}
		exec(argv+1, 0); //exec the commands
	}
	else{ //if no arguments were entered upon launch, thus entering continuous prompting
		while(1){ //infinite loop until exit entered
			i = 0; 
			bg = 0;
			printf("Shell: "); //display the prompt
			fgets(input, 128, stdin); //read in command line
			strtok(input, "\n"); //remove \n from input
			parse_argv(input, argv);
			if (!strcmp(argv[0], "exit")){ //check for exit
				exit(0);
			}
			if (!strcmp(argv[0], "cd")){ //check for change directory, if done, change it, don't exec the change directory
				chdir(argv[1]);	
			}
			else if(!strcmp(argv[0], "jobs")){ //check for jobs, and then print all background jobs accordingly
				while(p!= NULL) //iterate through all processes looking for ones that are still running
				{
					if(p->alive) // if one is alive, print below
					{
						printf("Process: %s, PID: %i\n", p->name, p->pid);
					}
					p = p->next; //go to the next one
				}
			}
			else{ //if normal inputs are entered proceed
				while(argv[i] != NULL){ //check for nulls, to avoid segfault
					if (!strcmp(argv[i], "&")){ //look for &, if there, run background and make bg=1
						bg = 1;
						argv[i] = NULL; //remove & from being exec'd
					}
					i++;
				}
				exec(argv, bg); 
				check_procs(head); //check for finished processes again
			}
		}
	}
}
