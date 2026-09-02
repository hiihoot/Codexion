#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include <time.h>   // for clock_gettime (if used in scheduler)

# define SCHED_FIFO 1
# define SCHED_EDF 2

typedef struct s_coder	t_coder;
typedef struct s_sim	t_sim;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long			available_at;
}	t_dongle;

typedef struct s_request
{
	t_coder	*coder;
	long	request_order;
	long	deadline;
}	t_request;

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
	t_request   request;
}	t_coder;

typedef struct s_heap
{
	t_request	**items;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_sim
{
	int			number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	long		next_request_order;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	int			scheduler;

	long		start_time;
	int			stop;

	t_heap		requests;
	pthread_mutex_t	scheduler_mutex;
	pthread_cond_t	scheduler_cond;

	pthread_mutex_t	print_mutex;
	pthread_mutex_t	state_mutex;

	t_coder		*coders;
	t_dongle	*dongles;
}	t_sim;

/* Time and utility */
long	get_time_ms(void);
int		ft_atoi(const char *str);

/* Parsing */
int		parse_args(int argc, char **argv, t_sim *sim);

/* Init / Destroy */
int		init_sim(t_sim *sim);
void	destroy_sim(t_sim *sim);

/* Coder lifecycle */
void	*coder_routine(void *arg);
void	compile(t_coder *coder);
void	debug(t_coder *coder);
void	refactor(t_coder *coder);

/* Dongle management */
int		take_dongles(t_coder *coder);
void	drop_dongles(t_coder *coder);

/* Logging */
void	log_event(t_coder *coder, const char *message);

/* State */
int		get_stop(t_sim *sim);
void	set_stop(t_sim *sim);
int		get_compile_count(t_sim *sim, t_coder *coder);
int		all_coders_finished(t_sim *sim);
int		coder_has_burned_out(t_sim *sim, t_coder *coder);

/* Monitor */
void	*monitor_routine(void *arg);

/* Heap (priority queue) */
int		heap_push(t_heap *heap, t_request *request, int scheduler);
t_request	*heap_pop(t_heap *heap, int scheduler);
int		request_has_priority(t_request *a, t_request *b, int scheduler);
void	heap_up(t_heap *heap, int index, int scheduler);
void	heap_down(t_heap *heap, int index, int scheduler);

/* Scheduler */
int		scheduler_take_dongles(t_coder *coder);
void	scheduler_drop_dongles(t_coder *coder);

#endif