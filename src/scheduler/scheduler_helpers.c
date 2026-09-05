#include "codexion.h"

static int	find_request_index(t_sim *sim, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < sim->requests.size
		&& sim->requests.items[i] != &coder->request)
		i++;
	return (i);
}

static void	remove_request(t_sim *sim, int index)
{
	sim->requests.size--;
	if (index < sim->requests.size)
	{
		sim->requests.items[index] = sim->requests.items[sim->requests.size];
		heap_up(&sim->requests, index, sim->scheduler);
		heap_down(&sim->requests, index, sim->scheduler);
	}
}

static int	grant_dongles_to_coder(t_sim *sim, t_coder *coder)
{
	pthread_mutex_unlock(&sim->scheduler_mutex);
	if (!lock_dongles(coder))
		return (0);
	if (!get_stop(sim))
	{
		log_event(coder, "has taken a dongle");
		log_event(coder, "has taken a dongle");
	}
	return (1);
}

int	acquire_candidate(t_sim *sim, t_coder *coder)
{
	int	index;

	index = find_request_index(sim, coder);
	if (index == sim->requests.size)
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		return (0);
	}
	remove_request(sim, index);
	return (grant_dongles_to_coder(sim, coder));
}

void	wait_next(t_sim *sim, t_coder *coder)
{
	struct timespec	ts;
	long		now, last, deadline_ms, cool, wait_ms;

	now = get_time_ms();
	pthread_mutex_lock(&sim->state_mutex);
	last = coder->last_compile_start;
	pthread_mutex_unlock(&sim->state_mutex);

	deadline_ms = last + sim->time_to_burnout;
	wait_ms = deadline_ms - now;
	if (wait_ms < 0)
		wait_ms = 0;

	/* Wake early when a dongle becomes available */
	cool = cooldown_left(coder, now);
	if (cool < wait_ms)
		wait_ms = cool;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += wait_ms / 1000;
	ts.tv_nsec += (wait_ms % 1000) * 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&sim->scheduler_cond,
		&sim->scheduler_mutex, &ts);
}
