/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#define PIPESIZE 10


////////////////////////////////////////////////////////////////////
//Pour cd c'est une commande integrée au shell
//Donc il faut changer le path
//chdir()  remplace  le répertoire de travail courant du processus 
//appelant par celui indiqué dans le chemin path.
////////////////////////////////////////////////////////////////////


//si en arrière plan, détache le processus du groupe???
// -> voir si pas ajouter des cas d'erreurs à verif
void execution_commande(struct cmdline *l) {	
	pid_t pid;
	int status;	
	pid_t pid_tab[PIPSIZE+1];
	int pipefd[PIPESIZE][2];
	int pipenumber = 0;
	while(l->seq[pipenumber+1]!=0){ //Tant qu'on a des commandes à pipe
		pipe(pipefd[pipenumber]); //On place le pipe associé aux commandes pipenumber et pipenumber+1 dans le tableau pipefd	
		pipenumber+=1;
	}
	for(int i=0; i<pipenumber; i++){
		if(i==0){ // Premiere commande a effectuer 
			if((pid = Fork()) == 0){
				if (l->in) {
					int fd_in = Open(l->in, O_RDONLY, S_IRUSR);
					Dup2(fd_in, STDIN_FILENO); //redirige l'entrée standard vers le fichier
					Close(fd_in);
				}
				for(int j=0; j<pipenumber; j++){
					if(j==0){
						Close(pipefd[j][0]);
					}
					else{
						Close(pipefd[j][1]);
						Close(pipefd[j][0]);
					}
					Dup2(pipefd[0][1], 1);
					execvp(l->seq[0][0], l->seq[0]);
					exit(0);
				}
			}
			else{
				pid_tab[0]=pid;
			}
		}
		else{ //On est dans une commande entre la premiere et la derniere
			if(Fork()==0){ // On est bien dans un processus fils 
				for(int j=0; j<pipenumber; j++){
					if(j==i){
						Close(pipefd[j][0]);
						Close(pipefd[j+1][1];
								}
								else if(j!=i+1){
								Close(pipefd[j][1]);
								Close(pipefd[j][0]);
								}
								}
								Dup2(pipefd[j][0], 0);
								Dup2(pipefd[j+1][1], 1);
								execvp(l->seq[j][0], l->seq[j]);
								exit(0);

								}

								else { //Pere
								Waitpid(pid, &status, 0); //On attend que l'enfant ait fini pour continuer
								}
								}
								}
								}
								//commande à effectuer
void exec_cmd
if (l->out) {
	int fd_out = Open(l->out, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); 
	Dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
	Close(fd_out);
}

Execvp(l->seq[i][0], l->seq[i]);
exit(0);

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
}
if (l->out) {
	int fd_out = Open(l->out, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); 
	Dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
	Close(fd_out);
}

Execvp(l->seq[i][0], l->seq[i]);
exit(0);
}
else { //Pere
	Waitpid(pid, &status, 0); //On attend que l'enfant ait fini pour continuer
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
