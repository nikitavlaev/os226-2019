<<<<<<< HEAD
=======
#include <limits.h>
#include <string.h>
#include <stdio.h>

>>>>>>> 29e2c1eda58e3822bb504138e6576b9f4d91a210
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

struct task {
	void (*entry)(void *as);
	void *as;
	int priority;
	int deadline;

	// timeout support
	int waketime;

	// policy support
	struct task *next;
};

static int time;

static struct task *current;
static struct task *runq;
static struct task *waitq;

static int (*policy_cmp)(struct task *t1, struct task *t2);

static struct task taskpool[16];
static int taskpool_n;

static void policy_run(struct task *t) {
	struct task **c = &runq;

	while (*c && policy_cmp(*c, t) <= 0) {
		c = &(*c)->next;
	}
	t->next = *c;
	*c = t;
}

void sched_new(void (*entrypoint)(void *aspace),
<<<<<<< HEAD
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
=======
		void *aspace,
	       	int priority,
		int deadline) {

	if (ARRAY_SIZE(taskpool) <= taskpool_n) {
		fprintf(stderr, "No mem for new task\n");
		return;
	}
	struct task *t = &taskpool[taskpool_n++];

	t->entry = entrypoint;
	t->as = aspace;
	t->priority = priority;
	t->deadline = 0 <= deadline ? deadline : INT_MAX;

	policy_run(t);
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {

	if (current->next != current) {
		fprintf(stderr, "Mulitiple sched_cont\n");
		return;
	}

	if (!timeout) {
		policy_run(current);
		return;
	}

	current->waketime = time + timeout;

	struct task **c = &waitq;
	while (*c && (*c)->waketime < current->waketime) {
		c = &(*c)->next;
	}
	current->next = *c;
	*c = current;
}

void sched_time_elapsed(unsigned amount) {
	time += amount;

	while (waitq && waitq->waketime <= time) {
		struct task *t = waitq;
		waitq = waitq->next;
		policy_run(t);
	}
}

static int fifo_cmp(struct task *t1, struct task *t2) {
	return -1;
}

static int prio_cmp(struct task *t1, struct task *t2) {
	return t1->priority - t2->priority;
}

static int deadline_cmp(struct task *t1, struct task *t2) {
	int d = t1->deadline - t2->deadline;
	if (d) {
		return d;
	}
	return prio_cmp(t1, t2);
}

void sched_set_policy(const char *name) {
	if (!strcmp(name, "fifo")) {
		policy_cmp = fifo_cmp;
	} else if (!strcmp(name, "priority")) {
		policy_cmp = prio_cmp;
	} else if (!strcmp(name, "deadline")) {
		policy_cmp = deadline_cmp;
	} else {
		fprintf(stderr, "Unknown policy: %s\n", name);
	}
}

void sched_run(void) {
	if (!policy_cmp) {
		fprintf(stderr, "Policy unset\n");
		return;
	}

	while (runq) {
		current = runq;
		runq = current->next;
		current->next = current;

		current->entry(current->as);
	}
>>>>>>> 29e2c1eda58e3822bb504138e6576b9f4d91a210
}
