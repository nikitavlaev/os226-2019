#include "sched.h"
#include "stdio.h"
#include "heap.h"
#include "heap.c"

#define __FIFO 0
#define __PRIO 1
#define __DDLN 2

#define HEAP_SIZE 20
#define INF 100000;

static int mode;

int __SCHED_CURRENT_TIME = 0;

Heap *schedule;


Node current_task;

void sched_init()
{
	schedule = create_heap(HEAP_SIZE, mode);
}

void sched_free()
{
	free(schedule->arr);
	free(schedule);
}

int prios_and_deadlines[HEAP_SIZE][2];

void sched_new(void (*entrypoint)(void *aspace),
			   void *aspace,
			   int priority,
			   int deadline)
{
	if (deadline < 0)
	{
		deadline = INF;
	}
	insert(schedule, createNode(entrypoint, aspace, priority, deadline, __SCHED_CURRENT_TIME));
	prios_and_deadlines[*((int *)aspace)][0] = priority;
	prios_and_deadlines[*((int *)aspace)][1] = deadline;
}

void sched_cont(void (*entrypoint)(void *aspace),
				void *aspace,
				int timeout)
{
	printf("cont\n");
	insert(schedule, createNode(entrypoint, aspace, prios_and_deadlines[*((int *)aspace)][0],
								prios_and_deadlines[*((int *)aspace)][1], __SCHED_CURRENT_TIME + timeout));
}

void sched_time_elapsed(unsigned amount)
{
	__SCHED_CURRENT_TIME += amount;
}

void sched_set_policy(const char *name)
{
	switch (name[0])
	{
	case 'f':
	{
		mode = __FIFO;
		break;
	}
	case 'p':
	{
		mode = __PRIO;
		break;
	}
	case 'd':
	{
		mode = __DDLN;
		break;
	}
	default:
	{
		printf("Wrong policy %s", name);
		break;
	}
	}
	sched_init();
}

void sched_run(void)
{

	while (schedule->count > 0)
	{
		current_task = pop_min(schedule);
		if (__SCHED_CURRENT_TIME >= current_task.timeout)
		{
			(current_task.entrypoint)(current_task.aspace);
		}
		else
		{
			sched_time_elapsed(current_task.timeout - __SCHED_CURRENT_TIME);	
			(current_task.entrypoint)(current_task.aspace);
		}
	}
	sched_free();
}
