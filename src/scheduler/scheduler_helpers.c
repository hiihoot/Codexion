/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_helpers.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 20:00:00 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/05 16:10:55 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

	index = 0;
	while (index < sim->requests.size
		&& sim->requests.items[index] != &coder->request)
		index++;
	if (index == sim->requests.size)
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		return (0);
	}
	sim->requests.size--;
	if (index < sim->requests.size)
	{
		sim->requests.items[index] = sim->requests.items[sim->requests.size];
		heap_up(&sim->requests, index, sim->scheduler);
		heap_down(&sim->requests, index, sim->scheduler);
	}
	return (grant_dongles_to_coder(sim, coder));
}

static long	compute_wait(t_sim *sim, t_coder *coder, long now)
{
	long	last;
	long	deadline_ms;
	long	cool;

	pthread_mutex_lock(&sim->state_mutex);
	last = coder->last_compile_start;
	pthread_mutex_unlock(&sim->state_mutex);
	deadline_ms = last + sim->time_to_burnout;
	if (deadline_ms <= now)
		return (0);
	cool = cooldown_left(coder, now);
	if (cool < deadline_ms - now)
		return (cool);
	return (deadline_ms - now);
}

void	wait_next(t_sim *sim, t_coder *coder)
{
	struct timespec	ts;
	long			now;
	long			wait_ms;

	now = get_time_ms();
	wait_ms = compute_wait(sim, coder, now);
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
