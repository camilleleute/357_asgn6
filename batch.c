#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int opt, eflag, vflag; 
	int numExecSim = 0;
	int prog_idx = 0;
	char * progs[2048][3];
	while ((opt = getopt(argc, argv, "n:ev")) != -1){
		switch (opt) {
			case 'n':
				numExecSim = atoi(optarg);
				if (numExecSim < 1) {
					fprintf(stderr, "usage: ./batch [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]");
					exit(EXIT_FAILURE);
				}
				break;
			case 'v':
				vflag = 1;	
				break;
			case 'e':
				eflag = 1;
				break;
			case '?':
				//ERROR
				break;
			default:
				break;
			}
	}
	while (optind < argc) {
		if ((strcmp(argv[optind], "--")) == 0) {
			optind++;
			progs[prog_idx][0] = argv[optind++]; //should be ./exec
			if ((strcmp(argv[optind], "--")) == 0) {
				progs[prog_idx][1] = NULL;
				progs[prog_idx][2] = NULL;
			} else {
            	   		progs[index][1] = atoi(argv[optind]);
              	      		progs[index][2] = atoi(argv[optind + 1]);
              			optind += 2;
			}
			index++;
       			
      	 		} else {
			break;
		}
	}
	
	

}
	
