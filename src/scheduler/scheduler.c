#include "codexion.h"
#include <time.h>

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

static void	prepare_request(t_coder *coder, t_request *request)
{
	pthread_mutex_lock(&coder->sim->state_mutex);
	request->coder = coder;
	request->deadline = coder->last_compile_start
		+ coder->sim->time_to_burnout;
	pthread_mutex_unlock(&coder->sim->state_mutex);
}

static int	dongles_ready(t_coder *coder, long now)
{
	if (coder->left->available_at > now)
		return (0);
	if (coder->sim->number_of_coders > 1
		&& coder->right->available_at > now)
		return (0);
	return (1);
}

static long	cooldown_left(t_coder *coder, long now)
{
	long	left, right;

	left = coder->left->available_at - now;
	if (left < 0)
		left = 0;
	if (coder->sim->number_of_coders == 1)
		return (left);
	right = coder->right->available_at - now;
	if (right < 0)
		right = 0;
	return (left > right ? left : right);
}

int	scheduler_take_dongles(t_coder *coder)
{
	t_sim		*sim;
	t_request	*request;
	int			candidate_index, our_index, i;
	long		now;
	struct timespec	ts;

	sim = coder->sim;
	request = &coder->request;

	prepare_request(coder, request);

	pthread_mutex_lock(&sim->scheduler_mutex);
	request->request_order = sim->next_request_order++;
	if (!heap_push(&sim->requests, request, sim->scheduler))
	{
		pthread_mutex_unlock(&sim->scheduler_mutex);
		return (0);
	}

	while (!get_stop(sim))
	{
		now = get_time_ms();
		candidate_index = -1;

		/* Scan the heap to find the highest‑priority ready coder */
		for (i = 0; i < sim->requests.size; i++)
		{
			t_request *r = sim->requests.items[i];
			t_coder *c = r->coder;

			/* Skip if deadline already passed (burned out) */
			if (now - c->last_compile_start >= sim->time_to_burnout)
				continue;

			if (dongles_ready(c, now))
			{
				if (candidate_index == -1 ||
					request_has_priority(r, sim->requests.items[candidate_index],
						sim->scheduler))
				{
					candidate_index = i;
				}
			}
		}

		if (candidate_index != -1)
		{
			t_coder *candidate = sim->requests.items[candidate_index]->coder;

			if (candidate == coder)
			{
				/* We are the highest‑priority ready coder – grant dongles */
				/* Find our index in the heap */
				our_index = -1;
				for (i = 0; i < sim->requests.size; i++)
				{
					if (sim->requests.items[i] == request)
					{
						our_index = i;
						break;
					}
				}
				if (our_index == -1)
				{
					pthread_mutex_unlock(&sim->scheduler_mutex);
					return (0);
				}

				/* Remove our request from the heap */
				sim->requests.size--;
				if (our_index < sim->requests.size)
				{
					sim->requests.items[our_index] = sim->requests.items[sim->requests.size];
					heap_up(&sim->requests, our_index, sim->scheduler);
					heap_down(&sim->requests, our_index, sim->scheduler);
				}

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
			else
			{
				/* A higher‑priority coder is ready – wait for it to finish */
				pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
				continue;
			}
		}

		/* No ready coder found – wait for cooldown of the root (or 1 ms) */
		if (sim->requests.size > 0)
		{
			t_coder *top_coder = sim->requests.items[0]->coder;
			long wait = cooldown_left(top_coder, now);

			if (wait > 0)
			{
				clock_gettime(CLOCK_REALTIME, &ts);
				ts.tv_sec += wait / 1000;
				ts.tv_nsec += (wait % 1000) * 1000000;
				if (ts.tv_nsec >= 1000000000)
				{
					ts.tv_sec++;
					ts.tv_nsec -= 1000000000;
				}
				pthread_cond_timedwait(&sim->scheduler_cond,
					&sim->scheduler_mutex, &ts);
			}
			else
			{
				/* Fallback: 1 ms timeout */
				clock_gettime(CLOCK_REALTIME, &ts);
				ts.tv_nsec += 1000000;
				if (ts.tv_nsec >= 1000000000)
				{
					ts.tv_sec++;
					ts.tv_nsec -= 1000000000;
				}
				pthread_cond_timedwait(&sim->scheduler_cond,
					&sim->scheduler_mutex, &ts);
			}
		}
		else
		{
			pthread_cond_wait(&sim->scheduler_cond, &sim->scheduler_mutex);
		}
	}

	pthread_mutex_unlock(&sim->scheduler_mutex);
	return (0);
}

void	scheduler_drop_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	available_at;

	sim = coder->sim;
	available_at = get_time_ms() + sim->dongle_cooldown;

	coder->left->available_at = available_at;
	if (sim->number_of_coders > 1)
		coder->right->available_at = available_at;

	unlock_dongles(coder);

	pthread_mutex_lock(&sim->scheduler_mutex);
	pthread_cond_broadcast(&sim->scheduler_cond);
	pthread_mutex_unlock(&sim->scheduler_mutex);
}