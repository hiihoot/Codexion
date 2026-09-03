/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 16:33:48 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 16:33:49 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	find_candidate(t_sim *sim, long now)
{
	int			i;
	int			cand;
	t_request	*r;
	t_coder		*c;

	cand = -1;
	i = 0;
	while (i < sim->requests.size)
	{
		r = sim->requests.items[i];
		c = r->coder;
		if (now - c->last_compile_start < sim->time_to_burnout)
		{
			if (dongles_ready(c, now))
			{
				if (cand == -1 || request_has_priority(r,
						sim->requests.items[cand], sim->scheduler))
					cand = i;
			}
		}
		i++;
	}
	return (cand);
}

static int	main_loop(t_sim *sim, t_coder *coder)
{
	int	cand;

	while (!get_stop(sim))
	{
		cand = find_candidate(sim, get_time_ms());
		if (cand != -1 && sim->requests.items[cand]->coder == coder)
			return (acquire_candidate(sim, coder));
		wait_next(sim, get_time_ms());
	}
	pthread_mutex_unlock(&sim->scheduler_mutex);
	return (0);
}

static int	push_request(t_sim *sim, t_coder *coder)
{
	pthread_mutex_lock(&sim->state_mutex);
	coder->request.coder = coder;
	coder->request.deadline = coder->last_compile_start
		+ sim->time_to_burnout;
	pthread_mutex_unlock(&sim->state_mutex);
	pthread_mutex_lock(&sim->scheduler_mutex);
	coder->request.request_order = sim->next_request_order++;
	if (!heap_push(&sim->requests, &coder->request, sim->scheduler))
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		return (0);
	}
	return (1);
}

int	scheduler_take_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (sim->number_of_coders == 1)
		return (0);
	if (!push_request(sim, coder))
		return (0);
	return (main_loop(sim, coder));
}

void	scheduler_drop_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	avail;

	sim = coder->sim;
	avail = get_time_ms() + sim->dongle_cooldown;
	coder->left->available_at = avail;
	if (sim->number_of_coders > 1)
		coder->right->available_at = avail;
	pthread_mutex_unlock(&coder->left->mutex);
	if (sim->number_of_coders > 1)
		pthread_mutex_unlock(&coder->right->mutex);
	pthread_mutex_lock(&sim->scheduler_mutex);
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
}
