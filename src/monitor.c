#include <codexion.h>

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (!get_stop(sim))
	{
		if (all_coders_finished(sim))
		{
			set_stop(sim);
			break;
		}
		i = 0;
		while (i < sim->number_of_coders)
		{
			if (coder_has_burned_out(sim, &sim->coders[i]))
			{
				log_event(&sim->coders[i], "burned out");
				set_stop(sim);
				break;
			}
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}