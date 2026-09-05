#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

void	set_timeout(struct timespec *ts, long wait_ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += wait_ms / 1000;
	ts->tv_nsec += (wait_ms % 1000) * 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

int	lock_dongles(t_coder *coder)
{
	pthread_mutex_t	*m1 = &coder->left->mutex;
	pthread_mutex_t	*m2 = &coder->right->mutex;

	if (m1 == m2)   /* only one dongle (n == 1) */
		return (pthread_mutex_lock(m1) == 0);

	if (m1 > m2)
	{
		pthread_mutex_t *tmp = m1;
		m1 = m2;
		m2 = tmp;
	}
	if (pthread_mutex_lock(m1) != 0)
		return (0);
	if (pthread_mutex_lock(m2) != 0)
	{
		pthread_mutex_unlock(m1);
		return (0);
	}
	return (1);
}

int	dongles_ready(t_coder *coder, long now)
{
	long	left_avail, right_avail;

	if (coder->sim->number_of_coders == 1)
	{
		pthread_mutex_lock(&coder->left->mutex);
		left_avail = coder->left->available_at;
		pthread_mutex_unlock(&coder->left->mutex);
		return (left_avail <= now);
	}

	pthread_mutex_lock(&coder->left->mutex);
	left_avail = coder->left->available_at;
	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_lock(&coder->right->mutex);
	right_avail = coder->right->available_at;
	pthread_mutex_unlock(&coder->right->mutex);
	return (left_avail <= now && right_avail <= now);
}

long	cooldown_left(t_coder *coder, long now)
{
	long	left, right;

	if (coder->sim->number_of_coders == 1)
	{
		pthread_mutex_lock(&coder->left->mutex);
		left = coder->left->available_at - now;
		if (left < 0)
			left = 0;
		pthread_mutex_unlock(&coder->left->mutex);
		return (left);
	}

	pthread_mutex_lock(&coder->left->mutex);
	left = coder->left->available_at - now;
	if (left < 0)
		left = 0;
	pthread_mutex_unlock(&coder->left->mutex);

	pthread_mutex_lock(&coder->right->mutex);
	right = coder->right->available_at - now;
	if (right < 0)
		right = 0;
	pthread_mutex_unlock(&coder->right->mutex);

	return (left > right ? left : right);
}