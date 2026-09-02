#include "codexion.h"

void	log_event(t_coder *coder, const char *message)
{
	long	timestamp;

	pthread_mutex_lock(&coder->sim->print_mutex);
	/* Double-check stop condition before printing */
	if (!get_stop(coder->sim))
	{
		timestamp = get_time_ms() - coder->sim->start_time;
		printf("%ld %d %s\n", timestamp, coder->id, message);
	}
	pthread_mutex_unlock(&coder->sim->print_mutex);
}