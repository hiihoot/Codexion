#include "codexion.h"

void	log_event(t_coder *coder, const char *message)
{
	long	timestamp;

	timestamp = get_time_ms() - coder->sim->start_time;
	pthread_mutex_lock(&coder->sim->print_mutex);
	printf("%ld %d %s\n", timestamp, coder->id, message);
	pthread_mutex_unlock(&coder->sim->print_mutex);
}