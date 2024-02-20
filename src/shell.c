/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#define PIPESIZE 10


void handler(){
	pid_t pid;
	while((pid=waitpid(-1,NULL,WNOHANG))>0){}
	return;
}


void execution_commande(struct cmdline *l) {
    pid_t pid;
    Signal(SIGCHLD, handler);
    int pipefd[PIPESIZE][2];
    int pipenumber = 0;
	while(l->seq[pipenumber+1]!=0){ //Tant qu'on a des commandes à pipe
		pipe(pipefd[pipenumber]); //On place le pipe associé aux commandes pipenumber et pipenumber+1 dans le tableau pipefd	
		pipenumber+=1;
	}
    for (int i = 0; i <= pipenumber; i++) {
        pid = Fork(); 
        if (pid == 0) { //Fils
            if (i > 0) { //On ferme les descripteurs du pipe precedent et on redirige l'entrée standard
                for(int j = 0; j<i-1; j++){
					Close(pipefd[j][0]);
					Close(pipefd[j][1]);
				}
				Dup2(pipefd[i - 1][0], 0);
                Close(pipefd[i - 1][1]);
            } 
			else if (l->in) { //On redirige l'entrée standard
                int fd_in = Open(l->in, O_RDONLY, S_IRUSR);
                Dup2(fd_in, 0);
                Close(fd_in);
            }

            if (i < pipenumber) { //On redirige la sortie vers le pipe suivant et on ferme ses descripteurs
                for(int j = i+1; j<pipenumber; j++){
					Close(pipefd[j][0]);
					Close(pipefd[j][1]);
				}
				Dup2(pipefd[i][1], 1);
                Close(pipefd[i][0]);
            } 
			else if (l->out) { //On redirige la sortie standard
                int fd_out = Open(l->out, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                Dup2(fd_out, 1);
                Close(fd_out);
            }
            Execvp(l->seq[i][0], l->seq[i]); //Gestion d'erreur dans csapp.c
            exit(1);
        }
    }

    //On ferme tous les descripteurs du père
    for (int i = 0; i < pipenumber; i++) {
        Close(pipefd[i][0]);
        Close(pipefd[i][1]);
    }

	if(!l->background){ //si pas en arriere plan alors on attend
		while(kill(pid, 0)==0){ //On attend que la seule ou la derniere commande s'effectue
			sleep(1);
		}
	}
}


int main()
{
	while (1) {
		struct cmdline *l;
		int i, j;

		printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		//Etape 2 : la commande quit
		if((strcmp(l->seq[0][0], "quit") == 0) || (strcmp(l->seq[0][0], "q") == 0)) {
			printf("Exiting shell\n");
			exit(0);
		}


		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);

		execution_commande(l);


		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}
			printf("\n");
		}

	}
}