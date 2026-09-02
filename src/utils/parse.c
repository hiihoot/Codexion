#include "codexion.h"

static int	parse_scheduler(const char *str)
{
	if (strcmp(str, "fifo") == 0)
		return (SCHED_FIFO);
	if (strcmp(str, "edf") == 0)
		return (SCHED_EDF);
	return (0);
}

static int	is_digit_string(const char *str)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	if (argc != 9)
		return (0);
	if (!is_digit_string(argv[1])
		|| !is_digit_string(argv[2])
		|| !is_digit_string(argv[3])
		|| !is_digit_string(argv[4])
		|| !is_digit_string(argv[5])
		|| !is_digit_string(argv[6])
		|| !is_digit_string(argv[7]))
		return (0);
	sim->number_of_coders = ft_atoi(argv[1]);
	sim->time_to_burnout = ft_atoi(argv[2]);
	sim->time_to_compile = ft_atoi(argv[3]);
	sim->time_to_debug = ft_atoi(argv[4]);
	sim->time_to_refactor = ft_atoi(argv[5]);
	sim->number_of_compiles_required = ft_atoi(argv[6]);
	sim->dongle_cooldown = ft_atoi(argv[7]);
	sim->scheduler = parse_scheduler(argv[8]);
	if (sim->scheduler == 0)
		return (0);
	return (1);
}