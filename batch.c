#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "batch.h"

int eflag = 0, vflag = 0, numcmds = 0;
pid_t PIDS[2048];
char *progs[2048][2048];

void execute(char *cmds[], int vflag);
void handler(int signum);

int main(int argc, char *argv[]) {
    int opt;
    int numExecSim = 0, prog_idx = 0;
    struct sigaction action;
    opterr = 0;
    action.sa_handler = handler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

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
    if ((numExecSim == 0) || (numExecSim > numcmds)) {
        numExecSim = numcmds;
    }

    for (int p = 0; p < numcmds; p++) {
        if (running >= numExecSim) {
            int status;
            pid_t finish_pid = wait(&status);
            running--;

            for (int j = 0; j < numcmds; j++) {
                if (PIDS[j] == finish_pid) {
                    if (eflag && WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
                        for (int k = 0; k < numcmds; k++) {
                            if (PIDS[k] > 0) {
                                kill(PIDS[k], SIGKILL);
                            }
                        }
                        exit(EXIT_FAILURE);
                    }

                    if (vflag) {
                        fprintf(stderr, "- %s", progs[j][0]);
                        for (int k = 1; progs[j][k] != '\0'; k++) {
                            fprintf(stderr, " %s", progs[j][k]);
                        }
                        fprintf(stderr, "\n");
                    }
                    break;
                }
            }
        }

        PIDS[p] = fork();
        if (PIDS[p] < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        } else {
            if (PIDS[p] == 0) {
                char *command[2048];
                int j = 0;
                while (progs[p][j] != '\0') {
                    command[j] = progs[p][j];
                    j++;
                }
                command[j] = NULL;
                execute(command, vflag);
            } else {
                running++;
            }
        }
    }

    while (running > 0) {
	int status;
        pid_t finish_pid = wait(&status);
        running--;
        for (int j = 0; j < numcmds; j++) {
            if (PIDS[j] == finish_pid) {
                if (vflag) {
		fprintf(stderr, "- %s", progs[j][0]);
                    for (int k = 1; progs[j][k] != '\0'; k++) {
                        fprintf(stderr, " %s", progs[j][k]);
                    }
                    fprintf(stderr, "\n");
                }

                if (eflag && WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
                    for (int i = 0; i < numcmds; i++) {
                        if (PIDS[i] > 0) {
                            kill(PIDS[i], SIGTERM);
                        }
                    }
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }

    for (int i = 0; i < prog_idx; i++) {
        for (int j = 0; progs[i][j] != NULL; j++) {
            free(progs[i][j]);
        }
    }
    return EXIT_SUCCESS;
}

void execute(char *cmds[], int vflag) {
    if (vflag) {
        fprintf(stderr, "+ %s", cmds[0]);
        for (int i = 1; cmds[i] != NULL; i++) {
            fprintf(stderr, " %s", cmds[i]);
        }
        fprintf(stderr, "\n");
    }

    execvp(cmds[0], cmds);
    perror(cmds[0]);
    exit(EXIT_FAILURE);
}

void handler(int signum) {
    eflag = 1;
    for (int i = 0; i < numcmds; i++) {
        if (PIDS[i] > 0) {
            kill(PIDS[i], signum);
        }
    }
}

