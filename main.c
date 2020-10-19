#include<stdio.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

struct package
{
	char buff[100];
	int pid[100], pcnt;
	int pcurr;
};

struct package *ptr;
int my_pid, shmid, i;

void join_room()
{
	ptr->pid[ptr->pcnt] = my_pid;
	ptr->pcnt += 1;
	printf("__PID %d has joined__\n", my_pid);
}

void out_room(int signum)
{
	int i = 0;
	for(; i < ptr->pcnt; i++)
	{
		if(ptr->pid[i] == my_pid)
			break;
	}
	for(; i < ptr->pcnt - 1; i++)
	{
		ptr->pid[i] = ptr->pid[i + 1];
	}
	ptr->pcnt -= 1;
	printf("__PID %d has left__\n", my_pid);
	
	if(0 == ptr->pcnt)
	{
		shmctl(shmid, IPC_RMID, NULL);
	}
	exit(0);
}

void broadcast()
{
	for(i = 0; i < ptr->pcnt; i++)
	{
		if(ptr->pid[i] == my_pid)
			continue;

		kill(ptr->pid[i], SIGUSR1); /* send SIGUSR1 to process id */
	}
}

void set_buffer()
{
	fgets(ptr->buff, 100, stdin);
	ptr->pcurr = my_pid;
}

void handler(int sig)
{
	if(sig == SIGUSR1)
	{
		printf("%d: %s", ptr->pcurr, ptr->buff);
	}
}

int main()
{
	/* Create shared memory */
	shmid = shmget(123, sizeof(struct package), IPC_CREAT | 0666);
	if(shmid < 0)
	{
		perror("Can't shmget");
	}
	/* Attach shared memory */
	ptr = (struct package*)shmat(shmid, NULL, 0);

	my_pid = getpid();
	join_room();
	
	/* listening from SIGUSER signal */
	signal(SIGUSR1, handler);
	signal(SIGINT, out_room);
	signal(SIGHUP, out_room);
	signal(SIGQUIT, out_room);

	printf("---------------Process--------------\n");
	printf("Process Count: %d\n", ptr->pcnt);
	for(i = 0; i < ptr->pcnt; i++)
		printf("Process Id: %d\n", ptr->pid[i]);

	while(1)
	{
		set_buffer();
		
		broadcast();
	}

	shmdt((void *)ptr);
	free(ptr);
	return 0;
}
