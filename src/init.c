#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
			return (0);
		sim->dongles[i].available_at = 0;
		i++;
	}
	return (1);
}

static int	allocate_resources(t_sim *sim)
{
	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->coders)
		return (0);
	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!sim->dongles)
	{
		free(sim->coders);
		sim->coders = NULL;
		return (0);
	}
	return (1);
}

static void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].request_order = 0;
		sim->coders[i].thread = 0;
		sim->coders[i].sim = sim;
		i++;
	}
}

static void	assign_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].left = &sim->dongles[i];
		if (i == sim->number_of_coders - 1)
			sim->coders[i].right = &sim->dongles[0];
		else
			sim->coders[i].right = &sim->dongles[i + 1];
		i++;
	}
}

int	init_sim(t_sim *sim)
{
    sim->start_time = get_time_ms();
    sim->stop = 0;
    
	if (!allocate_resources(sim))
		return (0);
	if (!init_dongles(sim))
	{
		free(sim->coders);
		free(sim->dongles);
		sim->coders = NULL;
		sim->dongles = NULL;
		return (0);
	}
	init_coders(sim);
	assign_dongles(sim);
	if (pthread_mutex_init(&sim->print_mutex, NULL) != 0)
	{
		destroy_sim(sim);
		return (0);
	}
	if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->print_mutex);
		destroy_sim(sim);
		return (0);
	}
	return (1);
}

void	destroy_sim(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->state_mutex);
	free(sim->coders);
	free(sim->dongles);
	sim->coders = NULL;
	sim->dongles = NULL;
}