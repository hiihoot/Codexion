#include "codexion.h"

static int	dongles_available(t_coder *coder, long now)
{
	if (coder->left->available_at > now)
		return (0);
	if (coder->sim->number_of_coders > 1
		&& coder->right->available_at > now)
		return (0);
	return (1);
}

int	scheduler_take_dongles(t_coder *coder)
{
	t_sim		*sim;
	t_request	*request;
	long		last_compile_start;
	long		now;

	sim = coder->sim;
	request = malloc(sizeof(t_request));
	if (!request)
		return (0);

	pthread_mutex_lock(&sim->state_mutex);
	last_compile_start = coder->last_compile_start;
	pthread_mutex_unlock(&sim->state_mutex);

	request->coder = coder;
	request->deadline = last_compile_start + sim->time_to_burnout;

	pthread_mutex_lock(&sim->scheduler_mutex);
	request->request_order = sim->next_request_order++;
	if (!heap_push(&sim->requests, request, sim->scheduler))
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		free(request);
		return (0);
	}
	while (!get_stop(sim))
	{
		now = get_time_ms();
		if (sim->requests.size > 0
			&& sim->requests.items[0] == request
			&& dongles_available(coder, now))
		{
			heap_pop(&sim->requests, sim->scheduler);
			pthread_mutex_unlock(&sim->scheduler_mutex);

			if (pthread_mutex_lock(&coder->left->mutex) != 0)
				return (0);
			if (sim->number_of_coders > 1
				&& pthread_mutex_lock(&coder->right->mutex) != 0)
			{
				pthread_mutex_unlock(&coder->left->mutex);
				return (0);
			}

			log_event(coder, "has taken a dongle");
			if (sim->number_of_coders > 1)
				log_event(coder, "has taken a dongle");
			free(request);
			return (1);
		}
		pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
	}
	pthread_mutex_unlock(&sim->scheduler_mutex);
	free(request);
	return (0);
}

void	scheduler_drop_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	available_at;

	sim = coder->sim;
	available_at = get_time_ms() + sim->dongle_cooldown;
	pthread_mutex_lock(&sim->scheduler_mutex);
	coder->left->available_at = available_at;
	if (sim->number_of_coders > 1)
		coder->right->available_at = available_at;
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
	pthread_mutex_unlock(&coder->left->mutex);
	if (sim->number_of_coders > 1)
		pthread_mutex_unlock(&coder->right->mutex);
}