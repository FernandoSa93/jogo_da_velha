/*****************************************************************************/
/*                  Laborátorio de Sistemas Operacionais                     */
/*                        Trabalho do Grau B (TGB)                           */
/*                       Autores: Nadine e Fernando                          */
/*****************************************************************************/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

//Variáveis globais
char pos11[1], pos12[1], pos13[1],
     pos21[1], pos22[1], pos23[1],
     pos31[1], pos32[1], pos33[1];
char jogada[1];
bool ganhador = false;

//Nome da fila
const char* NOME_FILA = "/jogoVelha";

//Estrutura de dados para a mensagem
typedef struct JogoVelha {
	char playerID[2];
	int coord1;
	int coord2;
} TJogoVelha;

//Declarações das funções
ssize_t get_msg_buffer_size(mqd_t queue);
void realiza_jogada(TJogoVelha* jv);
void verifica_ganhador(TJogoVelha* jv);

void tabuleiro() {
	system("clear");
	printf("Feito por Nadine e Fernando\n\n\n");
	printf("            JOGO DA VELHA\n\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos11, pos12, pos13);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos21, pos22, pos23);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n\n\n", pos31, pos32, pos33);
}

int main(void) {
	//Declaração da fila
	mqd_t queue;
	//Declaração do buffer
	char* buffer = NULL;
	//Declaração do tamanho do buffer
	ssize_t tam_buffer;
	ssize_t nbytes;

	//Obter descritor (mq_open+O_RDONLY)
	queue = mq_open(NOME_FILA, O_RDONLY);
	if (queue == (mqd_t) -1) {
		perror("mq_open");
		exit(2);
	}

	while(!ganhador){
		//Alocar buffer para receber msg
		tam_buffer = get_msg_buffer_size(queue);
		buffer = calloc(tam_buffer, 1);

		//Receber (mq_recv)
		nbytes = mq_receive(queue, buffer, tam_buffer, NULL);
		if (nbytes == -1) {
			perror("receive");
			exit(4);
		}

		tabuleiro();

		realiza_jogada((TJogoVelha*) buffer);
		verifica_ganhador((TJogoVelha*) buffer);	
	}

	//Liberar descritor (mq_cdeve acumular o pesolose)
	//
	mq_unlink(NOME_FILA);
	mq_close(queue);	
	printf("Fim!\n");

	exit(EXIT_SUCCESS);
}

void realiza_jogada(TJogoVelha* jv) {
	if ((strcmp(jv->playerID, " ") != 0) || (strcmp(jv->playerID, "p2") != 0)) {
		printf("Aguardando jogada do Jogador 1:\n");
	} else {
		printf("Aguardando jogada do Jogador 2:\n");
	}

	if ((strcmp(jv->playerID, "p1") != 0) || (strcmp(jv->playerID, "p2") != 0)) {
		printf("Jogador inválido!\n");
	} else {
		if (strcmp(jv->playerID, "p1") == 0) {
			sprintf(jogada, "%s", "X");
		} else {
			sprintf(jogada, "%s", "O");
		}

		while (((jv->coord1 > 3) || (jv->coord1 < 1)) ||
		      ((jv->coord2 > 3) || (jv->coord2 < 1))) {
			printf("Jogada inválida! Tente novamente.\n");
		}
		
		if ((jv->coord1 == 1) && (jv->coord2 == 1)) {
			sprintf(pos11, "%s", jogada);
		} else if ((jv->coord1 == 1) && (jv->coord2 == 2)) {	
			sprintf(pos12, "%s", jogada);
		} else if ((jv->coord1 == 1) && (jv->coord2 == 3)) {	
			sprintf(pos13, "%s", jogada);
		} else if ((jv->coord1 == 2) && (jv->coord2 == 1)) {	
			sprintf(pos21, "%s", jogada);
		} else if ((jv->coord1 == 2) && (jv->coord2 == 2)) {	
			sprintf(pos22, "%s", jogada);
		} else if ((jv->coord1 == 2) && (jv->coord2 == 3)) {	
			sprintf(pos23, "%s", jogada);
		} else if ((jv->coord1 == 3) && (jv->coord2 == 1)) {	
			sprintf(pos31, "%s", jogada);
		} else if ((jv->coord1 == 3) && (jv->coord2 == 2)) {	
			sprintf(pos32, "%s", jogada);
		} else if ((jv->coord1 == 3) && (jv->coord2 == 3)) {	
			sprintf(pos33, "%s", jogada);
		}
		printf("Jogada %d %d realizada com sucesso!",jv->coord1, jv->coord2);
	}
	sleep(3);
}

void verifica_ganhador(TJogoVelha* jv){
	if (((strcmp(pos11, "X") == 0) && (strcmp(pos12, "X") == 0) && (strcmp(pos13, "X") == 0)) || 
	   ((strcmp(pos21, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos23, "X") == 0)) || 
	   ((strcmp(pos31, "X") == 0) && (strcmp(pos32, "X") == 0) && (strcmp(pos33, "X") == 0)) ||  
	   ((strcmp(pos11, "X") == 0) && (strcmp(pos21, "X") == 0) && (strcmp(pos31, "X") == 0)) ||  
           ((strcmp(pos12, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos32, "X") == 0)) || 
	   ((strcmp(pos13, "X") == 0) && (strcmp(pos23, "X") == 0) && (strcmp(pos33, "X") == 0)) || 
	   ((strcmp(pos11, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos33, "X") == 0)) || 
	   ((strcmp(pos13, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos31, "X") == 0))) {
		printf("Jogador 1 ganhou!");
		ganhador = true;
	} else if (((strcmp(pos11, "O") == 0) && (strcmp(pos12, "O") == 0) && (strcmp(pos13, "O") == 0)) || 
	   	  ((strcmp(pos21, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos23, "O") == 0)) || 
	   	  ((strcmp(pos31, "O") == 0) && (strcmp(pos32, "O") == 0) && (strcmp(pos33, "O") == 0)) ||  
	   	  ((strcmp(pos11, "O") == 0) && (strcmp(pos21, "O") == 0) && (strcmp(pos31, "O") == 0)) ||  
           	  ((strcmp(pos12, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos32, "O") == 0)) || 
	   	  ((strcmp(pos13, "O") == 0) && (strcmp(pos23, "O") == 0) && (strcmp(pos33, "O") == 0)) || 
	   	  ((strcmp(pos11, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos33, "O") == 0)) || 
	   	  ((strcmp(pos13, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos31, "O") == 0))) {
		printf("Jogador 2 ganhou!");
		ganhador = true;
	}
}

ssize_t get_msg_buffer_size(mqd_t queue) {
	struct mq_attr attr;

	/*Determina max. msg size; allocate buffer to receive msg */
	if (mq_getattr(queue, &attr) != -1) {
		printf("max msg size: %ld\n", attr.mq_msgsize);
		return attr.mq_msgsize;
	}

	perror("aloca_msg_buffer");
	exit(3);
}
