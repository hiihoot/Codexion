/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_helpers.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 16:33:15 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 16:33:16 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

void	wait_next(t_sim *sim, long now)
{
	struct timespec	ts;
	t_coder			*top_coder;
	long			wait;

	if (sim->requests.size > 0)
	{
		top_coder = sim->requests.items[0]->coder;
		wait = cooldown_left(top_coder, now);
		if (wait > 0)
			set_timeout(&ts, wait);
		else
			set_timeout(&ts, 1);
		pthread_cond_timedwait(&sim->scheduler_cond,
			&sim->scheduler_mutex, &ts);
	}
	else
		pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
}
