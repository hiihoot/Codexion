#include "codexion.h"

int	take_dongles(t_coder *coder)
{
	return (scheduler_take_dongles(coder));
}

void	drop_dongles(t_coder *coder)
{
	scheduler_drop_dongles(coder);
}