#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "timer.h"
#include "sched.h"

#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))
#define SCHEDULE_MAX_SIZE 1000
#define INF 10000

int periods;
int timer_period;

struct Task
{
	void (*entrypoint)(void *aspace);
	void *aspace;
	int priority;
	int actual_priority;
	int timeout;
};

typedef struct Task Task;

Task schedule[SCHEDULE_MAX_SIZE];
int schedule_size = 0;
char update = 0;

Task create_task(void (*entrypoint)(void *aspace),
				 void *aspace,
				 int priority,
				 int timeout)
{
	Task buf;
	buf.entrypoint = entrypoint;
	buf.aspace = aspace;
	buf.priority = priority;
	buf.actual_priority = priority;
	buf.timeout = timeout;
	return buf;
}

void sched_sort()
{
	char more(int i, int j)
	{
		if (schedule[i].timeout <= schedule[j].timeout)
		{
			if (schedule[i].timeout < schedule[j].timeout)
			{
				return 1;
			}
			else
			{
				if (schedule[i].priority = schedule[j].priority)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}

	void swap(int i, int j)
	{
		Task buf = schedule[i];
		schedule[i] = schedule[j];
		schedule[j] = buf;
	}
	
	void quickSort(int first, int last)
	{
		int i, j, pivot, temp;

		if (first < last)
		{
			pivot = first;
			i = first;
			j = last;

			while (i < j)
			{
				while ((more(i,pivot)) && i < last)
					i++;
				while (!more(j, pivot))
					j--;
				if (i < j)
				{
					swap(i,j);
				}
			}

			swap(j, pivot);
			quickSort(first, j - 1);
			quickSort(j + 1, last);
		}
	}
	quickSort(0, schedule_size - 1);
}
void sched_update()
{
	int current_time = sched_gettime();

	for (int i = 0; i < schedule_size; i++)
	{
		if (schedule[i].timeout <= current_time)
		{
			schedule[i].timeout = 0;
		}
		printf(" %d %d ", schedule[i].actual_priority, schedule[i].timeout);
	}
	printf("\n");

	schedule[0].timeout = INF;
	sched_sort();
	schedule_size--; //delete old task

	printf("after sort \n");
	for (int i = 0; i < schedule_size; i++)
	{
		printf(" %d %d ", schedule[i].actual_priority, schedule[i].timeout);
	}
	printf("\n");
	printf("\n");
}

static void sigalrm(int sig)
{
	fprintf(stderr, "%s\n", __func__);
	periods++;
}

void sched_new(void (*entrypoint)(void *aspace),
			   void *aspace,
			   int priority)
{
	schedule[schedule_size] = create_task(entrypoint, aspace, priority, 0);
	schedule_size++;
}

void sched_cont(void (*entrypoint)(void *aspace),
				void *aspace,
				int timeout)
{
	//priority from current task
	printf("cont timeout %d time %d\n", timeout, sched_gettime());
	schedule[schedule_size] = create_task(entrypoint, aspace, schedule[0].actual_priority + 1, sched_gettime() + timeout);
	schedule_size++;
}

void sched_sleep(unsigned ms)
{

	int out_ms = sched_gettime() + ms;
	printf("sleep %d ", out_ms);
	while (sched_gettime() < out_ms)
		;
	printf("sleep-end %d %d\n", sched_gettime(), out_ms);
	update = 1;
}

int sched_gettime(void)
{
	int time = periods * timer_period + timer_cnt();
	return time;
}

void sched_run(int period_ms)
{
	timer_init_period(period_ms, sigalrm);
	periods = 0;
	timer_period = period_ms;
	while (schedule_size > 0)
	{
		if (schedule[0].timeout <= sched_gettime())
		{
			(schedule[0].entrypoint)(schedule[0].aspace);
		}
		if (update)
		{
			printf("update \n");
			sched_update();
			update = 0;
			//choose new task -> 0 indexed
		}
	}
}
