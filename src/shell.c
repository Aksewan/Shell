/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#define BUFFSIZE 100

// -> voir si pas ajouter des cas d'erreurs à verif
void execution_commande(struct cmdline *l) {	
	pid_t pid = Fork();
	int status;	
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} 
	else if (pid == 0) { // Premier Fils
		int i=0;
		while(l->seq[i+1]!=0){ //Tant qu'on a des commandes à pipe
			char buffer[BUFFSIZE];
			int pipefd[2];
			if (pipe(pipefd) < 0) {
				perror("pipe");
				exit(EXIT_FAILURE);
			}
			pid_t pid1 = Fork();
			if (pid1 == -1) {
				perror("fork");
				exit(EXIT_FAILURE);
			}
			else if (pid1 == 0) { //On veut passer dans la boucle suivante
				Close(pipefd[1]);
				read(pipefd[0], buffer, strlen(buffer) +1);
				i++;
			}
			else{ //On doit executer la commande de seq[i] et ecrire sa sortie vers le pipe
				Close(pipefd[0]);
				if (l->in) {
					int fd_in = open(l->in, O_RDONLY);
					if (fd_in == -1) {
						fprintf(stderr, "%s: Permission denied\n", l->in);
						exit(EXIT_FAILURE);
					}
					dup2(fd_in, STDIN_FILENO); //redirige l'entrée standard vers le fichier
					close(fd_in);
				}
				if (l->out) {
					int fd_out = open(l->out, O_WRONLY | O_TRUNC | O_CREAT); 
					if (fd_out == -1) {
						fprintf(stderr, "%s: Permission denied\n", l->out);
						exit(EXIT_FAILURE);
					}
					dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
					close(fd_out);
				}
				if (execvp(l->seq[i][0], l->seq[i]) < 0) { //execvp(cmd principale, liste des args)
					fprintf(stderr, "%s: command not found\n", l->seq[i][0]); //ou mettre le truc en entier?
					exit(EXIT_FAILURE);
				}
				write(pipefd[1], buffer, strlen(buffer) +1);
				pid_t w = waitpid(pid1, &status, 0); //On attend que l'enfant ait fini pour continuer
				if(w == -1){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				exit(0);
			}
		}
		//Derniere commande à effectuer
		if (l->in) {
			int fd_in = open(l->in, O_RDONLY);
			if (fd_in == -1) {
				fprintf(stderr, "%s: Permission denied\n", l->in);
				exit(EXIT_FAILURE);
			}
			dup2(fd_in, STDIN_FILENO); //redirige l'entrée standard vers le fichier
			close(fd_in);
		}
		if (l->out) {
			int fd_out = open(l->out, O_WRONLY | O_TRUNC | O_CREAT); 
			if (fd_out == -1) {
				fprintf(stderr, "%s: Permission denied\n", l->out);
				exit(EXIT_FAILURE);
			}
			dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
			close(fd_out);
		}
		if (execvp(l->seq[i][0], l->seq[i]) < 0) { //execvp(cmd principale, liste des args)
			fprintf(stderr, "%s: command not found\n", l->seq[i][0]); //ou mettre le truc en entier?
			exit(EXIT_FAILURE);
		}
		exit(0);

	}
	else { //Pere
		pid_t w = waitpid(pid, &status, 0); //On attend que l'enfant ait fini pour continuer
		if(w == -1){
			perror("waitpid");
			exit(EXIT_FAILURE);
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
			//freecmd(l);
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
