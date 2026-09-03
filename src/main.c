/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 15:50:58 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 16:37:30 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// static void	print_config(t_sim *sim)
// {
// 	printf("coders: %d\n", sim->number_of_coders);
// 	printf("burnout: %ld\n", sim->time_to_burnout);
// 	printf("compile: %ld\n", sim->time_to_compile);
// 	printf("debug: %ld\n", sim->time_to_debug);
// 	printf("refactor: %ld\n", sim->time_to_refactor);
// 	printf("required: %d\n", sim->number_of_compiles_required);
// 	printf("cooldown: %ld\n", sim->dongle_cooldown);
// 	if (sim->scheduler == SCHED_FIFO)
// 		printf("scheduler: fifo\n");
// 	else
// 		printf("scheduler: edf\n");
// }

static int	create_coder_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			printf("Failed to create coder thread\n");
			return (0);
		}
		i++;
	}
	return (1);
}

static int	create_monitor_thread(t_sim *sim, pthread_t *monitor)
{
	if (pthread_create(monitor, NULL, monitor_routine, sim) != 0)
	{
		printf("Failed to create monitor thread\n");
		set_stop(sim);
		return (0);
	}
	return (1);
}

static void	wait_for_threads(t_sim *sim, pthread_t monitor)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor, NULL);
}

int	main(int argc, char **argv)
{
	t_sim		sim;
	pthread_t	monitor;

	if (!parse_args(argc, argv, &sim))
	{
		printf("Invalid arguments\n");
		return (1);
	}
	if (!init_sim(&sim))
	{
		printf("Initialization failed\n");
		return (1);
	}
	// print_config(&sim);
	if (!create_coder_threads(&sim))
	{
		destroy_sim(&sim);
		return (1);
	}
	if (!create_monitor_thread(&sim, &monitor))
		return (1);
	wait_for_threads(&sim, monitor);
	destroy_sim(&sim);
	return (0);
}
