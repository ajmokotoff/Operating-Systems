#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER 1024

typedef struct data { // struct to hold temporary attributes about a file
	int num_bad, num_dirs, num_reg, num_special, num_text;
	unsigned long reg_size, text_size;
	char file[MAX_BUFFER];
} data;

int max_threads = 1; // default # of thread is 1
int active_workers = 0; // no active threads right now
pthread_t *workers; // list of all workers
pthread_mutex_t sys_stat, file_stats; // pthread_mutex used for mutual exclusion
sem_t sem_thread; // semaphore used for limiting number of threads
int bad_files = 0; // total number of bad files
int dirs = 0; // total number of directories
int reg_files = 0; // total number of regular files
int special_files = 0; // total number of special files
unsigned long reg_files_size = 0; // total size in bytes of regular files
int text_files = 0; // number of text files
unsigned long text_files_size = 0; // total size in bytes of text files

void BeginRegion(pthread_mutex_t mutex) { // locks the region for mutual exclusion
	pthread_mutex_lock(&mutex);
}

void EndRegion(pthread_mutex_t mutex) { // releases the region for mutual exclusion
	pthread_mutex_lock(&mutex);
}

void update_data(struct data *d) { // syncronized region for updating file information
	BeginRegion(file_stats); // start lock region
	// update variables
	bad_files += d->num_bad;
	dirs += d->num_dirs;
	reg_files += d->num_reg;
	special_files += d->num_special;
	reg_files_size += d->reg_size;
	text_files += d->num_text;
	text_files_size += d->text_size;

	EndRegion(file_stats); // release lock region so other threads can access...
}

void* perform_request(void *passed_data) { // called on each thread spawn	
	struct stat file; // for file details at a system level
	data *d = ((data*)passed_data); // cast the void pointer to our data struct
	int cnt; 
	unsigned char character;
	int infile;
	
	//initialize all variables to 0
	d->num_bad = 0;
	d->num_dirs = 0;
	d->num_reg = 0;
	d->num_special = 0;
	d->reg_size = 0;
	d->num_text = 0;
	d->text_size = 0;

	if(!(strcmp(d->file, "\n"))) { return; } // check if we are at the end of the line
	d->file[strcspn(d->file, "\n")] = 0; // remove \n and replace with null terminator
	BeginRegion(sys_stat); // start another lock region
	if(stat(d->file, &file) < 0) { // check if the file is ok
		d->num_bad++;
	} else if(S_ISDIR(file.st_mode)) { // check if the file is a directory
		d->num_dirs++;
	} else if(S_ISREG(file.st_mode)) { // check if the file is regular
		d->num_reg++;
		d->reg_size += file.st_size;
		infile = open(d->file, O_RDONLY);
		if(infile) { // then check all characters in it to see if its a text file
			do {
				cnt = read(infile, &character, sizeof(unsigned char)); // get one character from file
				if(!isprint(character) && !isspace(character)) { // check if characters are ok
					goto bypass; // if there is a non-accepted character, jump to bypass, so it is not
						     // added as a textfile, but still update and release semaphore and lock region
				}
			} while(cnt);
			
			d->num_text++; // if text file add 1
			d->text_size += file.st_size;
			close(infile);
		}
	} else { // if none of the above, its a "special file"
		d->num_special++;
	}
bypass:
	EndRegion(sys_stat); // release lock region
	update_data(d); // update variables 
	sem_post(&sem_thread);	// release thread from semaphore
}


void main(int argc, char *argv[]) {
	struct timeval start; // log start time for clock time
	int num_files = 10; // arbitrary number of files to start with
	int i = 0; 

	gettimeofday(&start, NULL); // record time in the beginning

	if(argc > 2) {
		if(strcmp(argv[1], "thread")) { return; } // check to see if user input is correct
		max_threads = atoi(argv[2]); // convert number of threads to int
		max_threads = (max_threads > 15) ? 15:max_threads; // if above 15 threads, limit it to 15.
	}

	workers = (pthread_t*)malloc(sizeof(pthread_t) * num_files); // allocate some memory for the threads
	sem_init(&sem_thread, 0, max_threads); // create the semaphore and get it ready

	if(pthread_mutex_init(&sys_stat, NULL) < 0) { // create mutex
		perror("pthread_mutex_init");
		exit(1);
	}
	if(pthread_mutex_init(&file_stats, NULL) < 0) { // create mutex
		perror("pthread_mutex_init");
		exit(1);
	}
	char buffer[MAX_BUFFER];
	
	while(fgets(buffer, MAX_BUFFER, stdin) != NULL) { // start scanning all input
		sem_wait(&sem_thread); // wait if max threads are running
		data *d = (data*)malloc(sizeof(data)); // allocate memory for data on each file
		strcpy(d->file, buffer); // load buffer into struct so it can be passed
		if(active_workers >= num_files) { // if we need more files to track, allocate more memory
			num_files += 10;
			workers = (pthread_t*)realloc(workers, sizeof(pthread_t) * num_files);
		} // create thread
		if(pthread_create(&workers[active_workers], NULL, perform_request, (void*)d)) {
			fprintf(stderr, "Error!\n");
			return;
		}
		d = NULL;
		active_workers++;
	}
	for(i=0; i<active_workers; i++)
	{ // kill all threads once done
		if(pthread_join(workers[i], NULL))
		{
			fprintf(stderr, "error!\n");
			continue;
		}
	}
	
	// print neccessary information
	printf("Bad Files: %i\n", bad_files);
	printf("Directories: %i\n", dirs);
	printf("Regular Files: %i\n", reg_files);
	printf("Special Files: %i\n", special_files);
	printf("Regular File Bytes: %lu\n", reg_files_size);
	printf("Text Files: %i\n", text_files);
	printf("Text File Bytes: %lu\n", text_files_size);
	
	// print times
	struct rusage stats;
	getrusage(RUSAGE_SELF, &stats);
	printf("User Time: %ld.%06ld seconds\n", stats.ru_utime.tv_sec, stats.ru_utime.tv_usec);
	printf("System Time: %ld.%06ld seconds\n", stats.ru_stime.tv_sec, stats.ru_stime.tv_usec);

	struct timeval end;
	gettimeofday(&end, NULL);
	printf("Clock Time: %ld.%06ld seconds\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);

	// kill mutexs
	(void) pthread_mutex_destroy(&file_stats);
	(void) pthread_mutex_destroy(&sys_stat);
}

