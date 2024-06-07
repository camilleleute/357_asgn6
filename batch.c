#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "batch.h"

int eflag = 0, vflag = 0, die = 0, numcmds = 0;
pid_t PIDS[2048];
char *progs[2048][2048];


int main(int argc, char *argv[]) {
    int opt;
    int numExecSim = 0, prog_idx = 0;
    opterr = 0;

	signal(SIGTERM, handler);
	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
    while ((opt = getopt(argc, argv, "n:ev")) != -1) {
        switch (opt) {
            case 'n':
                numExecSim = atoi(optarg);
                if (numExecSim < 1) {
                    fprintf(stderr, "usage: ./batch [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                vflag = 1;
                break;
            case 'e':
                eflag = 1;
                break;
            default:
                fprintf(stderr, "usage: ./batch [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]\n");
                exit(EXIT_FAILURE);
                break;
        }
    }
    
    int arg_idx;
    optind--;
    while (optind < argc) {
        if ((strcmp(argv[optind], "--")) == 0) {
            optind++;
            arg_idx = 0;
            while ((strcmp(argv[optind], "--")) != 0) {
                progs[prog_idx][arg_idx] = (char *)malloc(strlen(argv[optind]) + 1);
                strcpy(progs[prog_idx][arg_idx++], argv[optind++]);
                if (optind >= argc) {
                    break;
                }
            }
            progs[prog_idx][arg_idx] = '\0';
            prog_idx++;
        } else {
            break;
        }
    }

    numcmds = prog_idx;

    if (numcmds == 0) {
        fprintf(stderr, "usage: ./batch [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]\n");
        exit(EXIT_FAILURE);
    }

    int running = 0;
    int exit_stat = EXIT_SUCCESS;
	if ((numExecSim == 0) || (numExecSim > numcmds)) {
        numExecSim = numcmds;
    }

    for (int p = 0; p < numcmds; p++) {
        //waiting
	while (running >= numExecSim && !die) {
            int status;
            pid_t finish_pid = wait(&status);
            running--;

         	if (finish_pid>0 && WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
            		exit_stat = EXIT_FAILURE;
			if (!die && eflag){
				handler(SIGKILL);
			}
		}
                    if (vflag) {
                        fprintf(stderr, "- %s", progs[p][0]);
                        for (int k = 1; progs[p][k] != '\0'; k++) {
                            fprintf(stderr, " %s", progs[p][k]);
                        }
                        fprintf(stderr, "\n");
                    }
                    break;
        }
        
	if (die) {
		break;
	}
	//do yo thing
        PIDS[p] = fork();
        
	char *command[2048];
                int j = 0;
            	while (progs[p][j] != '\0') {
                    command[j] = progs[p][j];
                    j++;
                }
                command[j] = NULL;

	if (PIDS[p] < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        } else {
            if (PIDS[p] == 0) {
                execute(command); // should be execvp(), and perror then exit
            } else {
                
		if (vflag) {
        		fprintf(stderr, "+ %s", command[0]);
        		for (int i = 1; command[i] != NULL; i++) {
            			fprintf(stderr, " %s", command[i]);
        		}
        		fprintf(stderr, "\n");
    		}
		running++;
		
            }
     		}
  	}
    	while (running > 0) {
            int status;
            pid_t finish_pid = wait(&status);
            running--;

                if (finish_pid>0 && WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
                        exit_stat = EXIT_FAILURE;
                        if (!die && eflag){
                                handler(SIGKILL);
                        }
                }


		if (vflag) {
			for (int y = 0; y < numcmds; y++){
				if (PIDS[y] == finish_pid){
					if (vflag) {
                        			fprintf(stderr, "- %s", progs[y][0]);
                        			for (int k = 1; progs[y][k] != '\0'; k++) {
                            				fprintf(stderr, " %s", progs[y][k]);
                        			}
                        			fprintf(stderr, "\n");
                    			}
                    		break;
		
				}

			}
        }
    }

    for (int i = 0; i < prog_idx; i++) {
        for (int j = 0; progs[i][j] != NULL; j++) {
            free(progs[i][j]);
        }
    }
    exit(exit_stat);
	}


void execute(char *cmds[]) {

    execvp(cmds[0], cmds);
    perror(cmds[0]);
    exit(EXIT_FAILURE);
}

void handler(int signum) {
    die = 1;
    for (int i = 0; i < numcmds; i++) {
        if (PIDS[i] > 0) {
            kill(PIDS[i], signum);
        }
    }
}

