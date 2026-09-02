#include "codexion.h"

static void	swap_requests(t_request **a, t_request **b)
{
	t_request	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	request_has_priority(t_request *a, t_request *b, int scheduler)
{
	if (scheduler == SCHED_FIFO)
		return (a->request_order < b->request_order);
	if (a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	return (a->coder->id < b->coder->id);
}

void	heap_up(t_heap *heap, int index, int scheduler)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!request_has_priority(heap->items[index],
				heap->items[parent], scheduler))
			break;
		swap_requests(&heap->items[index], &heap->items[parent]);
		index = parent;
	}
}

int	heap_push(t_heap *heap, t_request *request, int scheduler)
{
	int	index;

	if (heap->size >= heap->capacity)
		return (0);
	index = heap->size;
	heap->items[index] = request;
	heap->size++;
	heap_up(heap, index, scheduler);
	return (1);
}

void	heap_down(t_heap *heap, int index, int scheduler)
{
	int	left;
	int	right;
	int	best;

	while (1)
	{
		left = index * 2 + 1;
		right = index * 2 + 2;
		best = index;

		if (left < heap->size
			&& request_has_priority(heap->items[left],
				heap->items[best], scheduler))
			best = left;

		if (right < heap->size
			&& request_has_priority(heap->items[right],
				heap->items[best], scheduler))
			best = right;

		if (best == index)
			break;

		swap_requests(&heap->items[index], &heap->items[best]);
		index = best;
	}
}

t_request	*heap_pop(t_heap *heap, int scheduler)
{
	t_request	*request;

	if (heap->size == 0)
		return (NULL);

	request = heap->items[0];
	heap->size--;

	if (heap->size > 0)
	{
		heap->items[0] = heap->items[heap->size];
		heap_down(heap, 0, scheduler);
	}
	return (request);
}