#include "codexion.h"

int	get_stop(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->state_mutex);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->state_mutex);
	return (stop);
}

void	set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->state_mutex);
}

int	get_compile_count(t_sim *sim, t_coder *coder)
{
	int	count;

	pthread_mutex_lock(&sim->state_mutex);
	count = coder->compile_count;
	pthread_mutex_unlock(&sim->state_mutex);
	return (count);
}

int	all_coders_finished(t_sim *sim)
{
	int	i;
	int	finished;

	pthread_mutex_lock(&sim->state_mutex);
	i = 0;
	finished = 1;
	while (i < sim->number_of_coders)
	{
		if (sim->coders[i].compile_count
			< sim->number_of_compiles_required)
		{
			finished = 0;
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&sim->state_mutex);
	return (finished);
}

int	coder_has_burned_out(t_sim *sim, t_coder *coder)
{
	long	last_compile_start;
	long	now;

	pthread_mutex_lock(&sim->state_mutex);
	last_compile_start = coder->last_compile_start;
	pthread_mutex_unlock(&sim->state_mutex);

	now = get_time_ms();
	return (now - last_compile_start >= sim->time_to_burnout);
}