/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 10:46:08 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 10:46:09 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_event(t_coder *coder, const char *message)
{
	long	timestamp;

	pthread_mutex_lock(&coder->sim->print_mutex);
	if (!get_stop(coder->sim))
	{
		timestamp = get_time_ms() - coder->sim->start_time;
		printf("%ld %d %s\n", timestamp, coder->id, message);
	}
	pthread_mutex_unlock(&coder->sim->print_mutex);
}
