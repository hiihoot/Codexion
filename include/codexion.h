#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
#include <unistd.h>

# define SCHED_FIFO 1
# define SCHED_EDF 2

typedef struct s_sim	t_sim;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long			available_at;
}	t_dongle;

typedef struct s_coder
{
	int			id;
	int			compile_count;
	long		last_compile_start;
	long		request_order;
	pthread_t	thread;
	t_dongle	*left;
	t_dongle	*right;
	t_sim		*sim;
}	t_coder;

typedef struct s_sim
{
	int			number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	int			scheduler;

	long		start_time;
	int			stop;

	pthread_mutex_t	print_mutex;
	pthread_mutex_t	state_mutex;

	t_coder		*coders;
	t_dongle	*dongles;
}	t_sim;

int		ft_atoi(const char *str);
int		parse_args(int argc, char **argv, t_sim *sim);
int		init_sim(t_sim *sim);
void	destroy_sim(t_sim *sim);
void	*coder_routine(void *arg);
long	get_time_ms(void);
void	log_event(t_coder *coder, const char *message);
int		take_dongles(t_coder *coder);
void	drop_dongles(t_coder *coder);
void	compile(t_coder *coder);
void	debug(t_coder *coder);
void	refactor(t_coder *coder);


#endif