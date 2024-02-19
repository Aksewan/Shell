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

void handler(){
	pid_t pid;
	while((pid=waitpid(-1,NULL,WNOHANG))>0){}
	return;
}

//si en arrière plan, détache le processus du groupe???
// -> voir si pas ajouter des cas d'erreurs à verif
void execution_commande(struct cmdline *l) {	
	pid_t pid;
	Signal(SIGCHLD, handler);
	int pipefd[PIPESIZE][2];
	int pipenumber = 0;
	while(l->seq[pipenumber+1]!=0){ //Tant qu'on a des commandes à pipe
		pipe(pipefd[pipenumber]); //On place le pipe associé aux commandes pipenumber et pipenumber+1 dans le tableau pipefd	
		pipenumber+=1;
	}
	if(pipenumber>0){
	for(int i=0; i<pipenumber; i++){
		if(i==0){ // Premiere commande a effectuer 
			for(int j=0; j<pipenumber; j++){
				Close(pipefd[j][1]);
				Close(pipefd[j][0]);
			}
			if(Fork() == 0){

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
					exit(1);
				}
			}
		}
		else{ //On est dans une commande entre la premiere et la derniere
			if(Fork()==0){ // On est bien dans un processus fils 
				for(int j=0; j<pipenumber; j++){ //On ouvre et ferme les entrées-sorties des pipes qui nous intéressent
					if(j==i-1){
						Close(pipefd[j][1]);
					}
					else if(j==i){ 
						Close(pipefd[j][0]);
					}
					else{
						Close(pipefd[j][0]);
						Close(pipefd[j][1]);
					}
				}
				Dup2(pipefd[i-1][0], 0);
				Dup2(pipefd[i][1], 1);
				execvp(l->seq[i][0], l->seq[i]);
				exit(1);
			}
		}
	} //Il reste la derniere commande à effecuter
	if((pid = Fork())==0){	
		if (l->out) {
			int fd_out = Open(l->out, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); 
			Dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
			Close(fd_out);
		}
		for(int j=0; j<pipenumber; j++){
			if(j==pipenumber-1){
				Close(pipefd[j][1]);
			}
			else{
				Close(pipefd[j][0]);
				Close(pipefd[j][1]);
			}
		}
		Dup2(pipefd[pipenumber-1][1], 1);
		execvp(l->seq[0][0], l->seq[0]);
		exit(1);
	}
	}
	else{
		if((pid = Fork()) ==0){ 
		if (l->out) {
			int fd_out = Open(l->out, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); 
			Dup2(fd_out, STDOUT_FILENO); //redirige la sortie standard vers le fichier
			Close(fd_out);
		}
		if (l->in) {
			int fd_in = Open(l->in, O_RDONLY, S_IRUSR);
			Dup2(fd_in, STDIN_FILENO); //redirige l'entrée standard vers le fichier
			Close(fd_in);
		}
		execvp(l->seq[0][0], l->seq[0]);
		exit(1);
	}
	}
	while(kill(pid, 0)==0){ //On attend que la seule ou la derniere commande s'effectue
		sleep(1);
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
