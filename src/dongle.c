#include "codexion.h"

int	take_dongles(t_coder *coder)
{
	if (coder->sim->number_of_coders == 1)
	{
		if (pthread_mutex_lock(&coder->left->mutex) != 0)
			return (0);
		return (1);
	}
	if (pthread_mutex_lock(&coder->left->mutex) != 0)
		return (0);
	if (pthread_mutex_lock(&coder->right->mutex) != 0)
	{
		pthread_mutex_unlock(&coder->left->mutex);
		return (0);
	}
	return (1);
}

void	drop_dongles(t_coder *coder)
{
	pthread_mutex_unlock(&coder->left->mutex);
	if (coder->sim->number_of_coders > 1)
		pthread_mutex_unlock(&coder->right->mutex);
}