#include "codexion.h"

void	compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->state_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->sim->state_mutex);

	log_event(coder, "is compiling");
	usleep(coder->sim->time_to_compile * 1000);

	pthread_mutex_lock(&coder->sim->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->sim->state_mutex);
}

void	debug(t_coder *coder)
{
	log_event(coder, "is debugging");
	usleep(coder->sim->time_to_debug * 1000);
}

void	refactor(t_coder *coder)
{
	log_event(coder, "is refactoring");
	usleep(coder->sim->time_to_refactor * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!get_stop(coder->sim) && get_compile_count(coder->sim, coder) < coder->sim->number_of_compiles_required)	
	{
		if (!take_dongles(coder))
			return (NULL);
		compile(coder);
		drop_dongles(coder);
		if (get_stop(coder->sim))
			break;
		debug(coder);
		if (get_stop(coder->sim))
			break;
		refactor(coder);
	}
	return (NULL);
}