/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sait-mou <sait-mou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 11:11:26 by sait-mou          #+#    #+#             */
/*   Updated: 2026/09/03 11:11:27 by sait-mou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	take_dongles(t_coder *coder)
{
	return (scheduler_take_dongles(coder));
}

void	drop_dongles(t_coder *coder)
{
	scheduler_drop_dongles(coder);
}
