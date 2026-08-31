#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (!parse_args(argc, argv, &sim))
	{
		printf("Invalid arguments\n");
		return (1);
	}

    if (!init_sim(&sim))
    {
	    printf("Initialization failed\n");
	    return (1);
    }

	printf("coders: %d\n", sim.number_of_coders);
	printf("burnout: %ld\n", sim.time_to_burnout);
	printf("compile: %ld\n", sim.time_to_compile);
	printf("debug: %ld\n", sim.time_to_debug);
	printf("refactor: %ld\n", sim.time_to_refactor);
	printf("required: %d\n", sim.number_of_compiles_required);
	printf("cooldown: %ld\n", sim.dongle_cooldown);
    printf("scheduler: %s\n", sim.scheduler == SCHED_FIFO ? "fifo" : "edf");

    int	i;

    i = 0;
    while (i < sim.number_of_coders)
    {
        if (pthread_create(&sim.coders[i].thread,
			NULL, coder_routine, &sim.coders[i]) != 0)
	    {
		    printf("Failed to create coder thread\n");
		    destroy_sim(&sim);
		    return (1);
	    }
	    i++;
    }

    i = 0;
    
    while (i < sim.number_of_coders)
    {
        pthread_join(sim.coders[i].thread, NULL);
        i++;
    }

	return (0);
}