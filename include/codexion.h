/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 15:50:58 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/05 13:25:47 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include <time.h>   /* for clock_gettime */

# define SCHED_FIFO 1
# define SCHED_EDF 2

typedef struct s_coder	t_coder;
typedef struct s_sim	t_sim;

/* Dongle (mutex + availability timestamp) */
typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long			available_at;
}	t_dongle;

/* Request for the heap (scheduling) */
typedef struct s_request
{
	t_coder	*coder;
	long	request_order;
	long	deadline;
}	t_request;

/* Coder thread state */
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
	t_request	request;
}	t_coder;

/* Heap (priority queue) for FIFO/EDF */
typedef struct s_heap
{
	t_request	**items;
	int			size;
	int			capacity;
}	t_heap;

/* Main simulation state */
typedef struct s_sim
{
	int				number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	long			next_request_order;   /* for true FIFO */
	int				number_of_compiles_required;
	long			dongle_cooldown;
	int				scheduler;

	long			start_time;
	int				stop;

	t_heap			requests;
	pthread_mutex_t	scheduler_mutex;
	pthread_cond_t	scheduler_cond;

	pthread_mutex_t	print_mutex;
	pthread_mutex_t	state_mutex;

	t_coder			*coders;
	t_dongle		*dongles;
}	t_sim;

/* ---- Time & Utilities ---- */
long	get_time_ms(void);
void	set_timeout(struct timespec *ts, long wait_ms);
int		ft_atoi(const char *str);

/* ---- Dongle Mutex Helpers ---- */
int		lock_dongles(t_coder *coder);
int		dongles_ready(t_coder *coder, long now);
long	cooldown_left(t_coder *coder, long now);

/* ---- Parsing ---- */
int		parse_args(int argc, char **argv, t_sim *sim);

/* ---- Init / Destroy ---- */
int		init_sim(t_sim *sim);
void	destroy_sim(t_sim *sim);

/* ---- Coder Lifecycle ---- */
void	*coder_routine(void *arg);
void	compile(t_coder *coder);
void	debug(t_coder *coder);
void	refactor(t_coder *coder);

/* ---- Dongle Interface (wrappers) ---- */
int		take_dongles(t_coder *coder);
void	drop_dongles(t_coder *coder);

/* ---- Scheduler Core ---- */
int		scheduler_take_dongles(t_coder *coder);
void	scheduler_drop_dongles(t_coder *coder);

/* ---- Scheduler Helpers (shared between scheduler.c & helpers) ---- */
int		acquire_candidate(t_sim *sim, t_coder *coder);
void	wait_next(t_sim *sim, t_coder *coder);

/* ---- Heap (Priority Queue) ---- */
int		heap_push(t_heap *heap, t_request *request, int scheduler);
t_request	*heap_pop(t_heap *heap, int scheduler);
int		request_has_priority(t_request *a, t_request *b, int scheduler);
void	heap_up(t_heap *heap, int index, int scheduler);
void	heap_down(t_heap *heap, int index, int scheduler);

/* ---- Logging ---- */
void	log_event(t_coder *coder, const char *message);

/* ---- Global State ---- */
int		get_stop(t_sim *sim);
void	set_stop(t_sim *sim);
int		get_compile_count(t_sim *sim, t_coder *coder);
int		all_coders_finished(t_sim *sim);
int		coder_has_burned_out(t_sim *sim, t_coder *coder);

/* ---- Monitor ---- */
void	*monitor_routine(void *arg);

#endif
