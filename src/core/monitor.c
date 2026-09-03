/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 10:59:06 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 10:59:07 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wake_coders(t_sim *sim)
{
	pthread_mutex_lock(&sim->scheduler_mutex);
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
}

static int	check_burnout(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (coder_has_burned_out(sim, &sim->coders[i]))
		{
			log_event(&sim->coders[i], "burned out");
			set_stop(sim);
			return (1);
		}
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
		usleep(1000);
		wake_coders(sim);
	}
	wake_coders(sim);
	return (NULL);
}
