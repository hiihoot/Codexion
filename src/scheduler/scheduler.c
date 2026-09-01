#include "codexion.h"

static int	lock_dongles(t_coder *coder)
{
	if (pthread_mutex_lock(&coder->left->mutex) != 0)
		return (0);
	if (coder->sim->number_of_coders > 1)
	{
		if (pthread_mutex_lock(&coder->right->mutex) != 0)
		{
			pthread_mutex_unlock(&coder->left->mutex);
			return (0);
		}
	}
	return (1);
}

static void	unlock_dongles(t_coder *coder)
{
	pthread_mutex_unlock(&coder->left->mutex);
	if (coder->sim->number_of_coders > 1)
		pthread_mutex_unlock(&coder->right->mutex);
}

int	scheduler_take_dongles(t_coder *coder)
{
	t_sim		*sim;
	t_request	*request;   // single declaration
	long		now;
	int			locked;

	sim = coder->sim;
	request = &coder->request;   // no malloc, use embedded

	// Prepare request fields
	pthread_mutex_lock(&sim->state_mutex);
	request->coder = coder;
	request->deadline = coder->last_compile_start + sim->time_to_burnout;
	pthread_mutex_unlock(&sim->state_mutex);

	// Lock scheduler mutex and push request
	pthread_mutex_lock(&sim->scheduler_mutex);
	request->request_order = sim->next_request_order++;
	if (!heap_push(&sim->requests, request, sim->scheduler))
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		return (0);   // no free needed
	}

	locked = 0;
	while (!get_stop(sim))
	{
		// If not at the top, wait
		if (sim->requests.size == 0 || sim->requests.items[0] != request)
		{
			pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
			continue;
		}

		// We are at the top → try to lock both dongle mutexes
		if (!lock_dongles(coder))
		{
			pthread_mutex_unlock(&sim->scheduler_mutex);
			return (0);
		}
		locked = 1;

		// Re‑check availability (cooldown) while holding the mutexes
		now = get_time_ms();
		if (coder->left->available_at <= now &&
			(sim->number_of_coders == 1 || coder->right->available_at <= now))
		{
			// Grant: pop request, unlock scheduler, keep dongle mutexes
			heap_pop(&sim->requests, sim->scheduler);
			pthread_mutex_unlock(&sim->scheduler_mutex);

			// Log the two dongle acquisitions
			log_event(coder, "has taken a dongle");
			log_event(coder, "has taken a dongle");

			return (1);   // request is embedded, no free
		}

		// Not available → release dongle mutexes and wait
		unlock_dongles(coder);
		locked = 0;
		pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
	}

	// Simulation stopped
	if (locked)
		unlock_dongles(coder);
	pthread_mutex_unlock(&sim->scheduler_mutex);
	return (0);
}

void	scheduler_drop_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	available_at;

	sim = coder->sim;
	available_at = get_time_ms() + sim->dongle_cooldown;

	// Set available_at while still holding the dongle mutexes
	coder->left->available_at = available_at;
	if (sim->number_of_coders > 1)
		coder->right->available_at = available_at;

	// Unlock dongle mutexes before locking scheduler_mutex
	unlock_dongles(coder);

	// Broadcast to wake waiters
	pthread_mutex_lock(&sim->scheduler_mutex);
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
}