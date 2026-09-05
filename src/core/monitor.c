/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 16:05:38 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/05 16:06:21 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_sim *sim)
{
	long	now;
	int		i;
	t_coder	*c;

	now = get_time_ms();
	i = 0;
	while (i < sim->number_of_coders)
	{
		c = &sim->coders[i];
		pthread_mutex_lock(&sim->state_mutex);
		if (now - c->last_compile_start >= sim->time_to_burnout)
		{
			pthread_mutex_unlock(&sim->state_mutex);
			pthread_mutex_lock(&sim->print_mutex);
			printf("%ld %d burned out\n", now - sim->start_time, c->id);
			pthread_mutex_unlock(&sim->print_mutex);
			set_stop(sim);
			return (1);
		}
		pthread_mutex_unlock(&sim->state_mutex);
		i++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (!get_stop(sim))
	{
		if (all_coders_finished(sim))
		{
			set_stop(sim);
			break ;
		}
		if (check_burnout(sim))
			break ;
		usleep(200);
	}
	pthread_mutex_lock(&sim->scheduler_mutex);
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
	return (NULL);
}
