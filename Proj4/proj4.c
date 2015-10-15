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

typedef struct data {
	int num_bad, num_dirs, num_reg, num_special, num_text;
	unsigned long reg_size, text_size;
	char file[MAX_BUFFER];
} data;

int max_threads = 1;
int active_workers = 0;
pthread_t *workers;
pthread_mutex_t sys_stat, file_stats;
sem_t sem_thread;
int bad_files = 0;
int dirs = 0;
int reg_files = 0;
int special_files = 0;
unsigned long reg_files_size = 0;
int text_files = 0;
unsigned long text_files_size = 0;

void BeginRegion(pthread_mutex_t mutex) {
	pthread_mutex_lock(&mutex);
}

void EndRegion(pthread_mutex_t mutex) {
	pthread_mutex_lock(&mutex);
}

void update_data(struct data *d) {
	BeginRegion(file_stats);
	bad_files += d->num_bad;
	dirs += d->num_dirs;
	reg_files += d->num_reg;
	special_files += d->num_special;
	reg_files_size += d->reg_size;
	text_files += d->num_text;
	text_files_size += d->text_size;
	EndRegion(file_stats);
}

void* perform_request(void *passed_data)
{	
	struct stat file;
	data *d = ((data*)passed_data);
	int cnt;
	unsigned char character;
	int infile;
	
	d->num_bad = 0;
	d->num_dirs = 0;
	d->num_reg = 0;
	d->num_special = 0;
	d->reg_size = 0;
	d->num_text = 0;
	d->text_size = 0;

	if(!(strcmp(d->file, "\n"))) { return; }
	d->file[strcspn(d->file, "\n")] = 0;
	BeginRegion(sys_stat);
	if(stat(d->file, &file) < 0) {
		d->num_bad++;
	} else if(S_ISDIR(file.st_mode)) {
		d->num_dirs++;
	} else if(S_ISREG(file.st_mode)) {
		d->num_reg++;
		d->reg_size += file.st_size;
		infile = open(d->file, O_RDONLY);
		if(infile) {
			do {
				cnt = read(infile, &character, sizeof(unsigned char));
				if(!isprint(character) && !isspace(character)) { 
					goto bypass;
				}
			} while(cnt);
			
			d->num_text++;
			d->text_size += file.st_size;
			close(infile);
		}
	} else {
		d->num_special++;
	}
bypass:
	EndRegion(sys_stat);
	update_data(d);
	sem_post(&sem_thread);	
}


void main(int argc, char *argv[]) {
	struct timeval start;
	int num_files = 10;
	int i = 0;

	gettimeofday(&start, NULL);

	if(argc > 2) {
		if(strcmp(argv[1], "thread")) { return; }
		max_threads = atoi(argv[2]);
	}

	workers = (pthread_t*)malloc(sizeof(pthread_t) * num_files);
	sem_init(&sem_thread, 0, max_threads);

	if(pthread_mutex_init(&sys_stat, NULL) < 0) {
		perror("pthread_mutex_init");
		exit(1);
	}
	if(pthread_mutex_init(&file_stats, NULL) < 0) {
		perror("pthread_mutex_init");
		exit(1);
	}
	char buffer[MAX_BUFFER];
	
	while(fgets(buffer, MAX_BUFFER, stdin) != NULL)
	{
		sem_wait(&sem_thread);
		data *d = (data*)malloc(sizeof(data));
		strcpy(d->file, buffer);
		if(active_workers >= num_files) {
			num_files += 10;
			workers = (pthread_t*)realloc(workers, sizeof(pthread_t) * num_files);
		}
		if(pthread_create(&workers[active_workers], NULL, perform_request, (void*)d)) {
			fprintf(stderr, "Error!\n");
			return;
		}
		d = NULL;
		active_workers++;
	}
	for(i=0; i<active_workers; i++)
	{
		if(pthread_join(workers[i], NULL))
		{
			fprintf(stderr, "error!\n");
			continue;
		}
	}
	
	printf("Bad Files: %i\n", bad_files);
	printf("Directories: %i\n", dirs);
	printf("Regular Files: %i\n", reg_files);
	printf("Special Files: %i\n", special_files);
	printf("Regular File Bytes: %lu\n", reg_files_size);
	printf("Text Files: %i\n", text_files);
	printf("Text File Bytes: %lu\n", text_files_size);
	
	struct rusage stats;
	getrusage(RUSAGE_SELF, &stats);
	printf("User Time: %ld.%06ld seconds\n", stats.ru_utime.tv_sec, stats.ru_utime.tv_usec);
	printf("System Time: %ld.%06ld seconds\n", stats.ru_stime.tv_sec, stats.ru_stime.tv_usec);

	struct timeval end;
	gettimeofday(&end, NULL);
	printf("Clock Time: %ld.%06ld seconds\n", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);

	(void) pthread_mutex_destroy(&file_stats);
	(void) pthread_mutex_destroy(&sys_stat);
}

