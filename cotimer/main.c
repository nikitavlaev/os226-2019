#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "sched.h"

struct app1_aspace {
	int id;
	int duration;
	int period;
	int cnt;
};

static long long reftime(void) {
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void app1(void *aspace) {
	struct app1_aspace *as = aspace;
	--as->cnt;

	printf("%s id %d cnt %d time1 %d reference %lld\n", __func__, as->id, as->cnt, sched_gettime(), reftime());

	sched_sleep(as->duration);

	printf("%s id %d cnt %d time2 %d reference %lld\n", __func__, as->id, as->cnt, sched_gettime(), reftime());

	if (0 < as->cnt) {
		sched_cont(app1, aspace, as->period);
	}
}

int main(int argc, char *argv[]) {
	char name[64];
	int prio;
	int duration;
	int period;
	int cnt;

	printf("reftime %lld\n", reftime());

	struct app1_aspace a1as[16];
	int a1as_n = 0;
	while (EOF != scanf("%s %d %d %d %d", name, &prio, &duration, &period, &cnt)) {
		struct app1_aspace *as = &a1as[a1as_n++];

		as->id = as - a1as;
		as->duration = duration;
		as->period = period;
		as->cnt = cnt;

		void (*entry)(void*);
		if (!strcmp("app1", name)) {
			entry = app1;
		} else {
			fprintf(stderr, "Unknown app: %s\n", name);
			return 1;
		}

		sched_new(entry, as, prio);
	}

	sched_run(100);
	return 0;
}
