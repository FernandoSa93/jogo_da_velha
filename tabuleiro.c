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
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

//Variáveis globais
char pos11[1], pos12[1], pos13[1],
     pos21[1], pos22[1], pos23[1],
     pos31[1], pos32[1], pos33[1];
char jogada[1];
char player[2];
bool ganhador = false;
bool jogadorCorreto = true;
bool coordenadasCorretas = true;

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
void verifica_ganhador();
void valida_jogada(TJogoVelha* jv);
void valida_coordenadas(TJogoVelha* jv);
void mensagem_jogador();
void tabuleiro();
void grava_log();

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
		//Problema: buffer está fazendo com que variaveis pos* recebam caracteres indevidos durante a jogada(RESOLVER)
		//Alocar buffer para receber msg
		tam_buffer = get_msg_buffer_size(queue);
		buffer = calloc(tam_buffer, 1);

		tabuleiro();
		mensagem_jogador();

		//Receber (mq_recv)
		nbytes = mq_receive(queue, buffer, tam_buffer, NULL);
		if (nbytes == -1) {
			perror("receive");
			exit(4);
		}

		valida_coordenadas((TJogoVelha*) buffer);
		valida_jogada((TJogoVelha*) buffer);

		if ((jogadorCorreto) && (coordenadasCorretas)) {
			realiza_jogada((TJogoVelha*) buffer);
			verifica_ganhador();
		}
		grava_log();	
	}

	//Liberar descritor (mq_cdeve acumular o pesolose)
	//
	mq_unlink(NOME_FILA);
	mq_close(queue);	
	printf("Fim!\n");

	exit(EXIT_SUCCESS);
}

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

void valida_jogada(TJogoVelha* jv) {
	if (coordenadasCorretas) {
		if (strcmp(player, "") == 0) {
			if (strcmp(jv->playerID, "p1") == 0) {
				strcpy(player, "p1");
				jogadorCorreto = true;
			} else if (strcmp(jv->playerID, "p2") == 0){
				printf("Que feio jogador p2! Aguarde a sua vez!\n");
				jogadorCorreto = false;
				sleep(3);
			} else {
				printf("Invasor detectado! Não atrapalhe o jogo!\n");
				jogadorCorreto = false;
				sleep(3);
			}
		} else if (strcmp(jv->playerID, "p1") == 0){
			if ((strcmp(player, "p1") != 0)) {
				strcpy(player, "p1");
				jogadorCorreto = true;
			} else {
				printf("Que feio jogador %s! Aguarde a sua vez!\n", player);
				jogadorCorreto = false;
				sleep(3);
			}
		} else if (strcmp(jv->playerID, "p2") == 0) {
			if (strcmp(player, "p2") != 0) {
				strcpy(player, "p2");
				jogadorCorreto = true;
			} else {
				printf("Que feio jogador %s! Aguarde a sua vez!\n", player);
				jogadorCorreto = false;
				sleep(3);
			}	
		} else { 
			printf("Invasor detectado! Não atrapalhe o jogo!\n");
			jogadorCorreto = false;
			sleep(3);
		}
	}
}

void mensagem_jogador() {
	if ((strcmp(player, "") == 0) || (strcmp(player, "p2") == 0)) {
		printf("Aguardando jogada do Jogador 1:\n");
	} else if (strcmp(player, "p1") == 0){
		printf("Aguardando jogada do Jogador 2:\n");
	} 
}
void valida_coordenadas(TJogoVelha* jv) {
	if (((jv->coord1 > 3) || (jv->coord1 < 1)) ||
            ((jv->coord2 > 3) || (jv->coord2 < 1))) {
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else {
		coordenadasCorretas = true;
	}
	if(((jv->coord1 == 1) && (jv->coord2 == 1)) && (strlen(pos11) != 0)){
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 1) && (jv->coord2 == 2)) && (strlen(pos12) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 1) && (jv->coord2 == 3)) && (strlen(pos13) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 1)) && (strlen(pos21) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 2)) && (strlen(pos22) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 3)) && (strlen(pos23) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 3) && (jv->coord2 == 1)) && (strlen(pos31) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if((jv->coord1 == 3) && (jv->coord2 == 2) && (strlen(pos32) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if((jv->coord1 == 3) && (jv->coord2 == 3) && (strlen(pos33) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	}
}

void realiza_jogada(TJogoVelha* jv) {
	//Problema: Quando jogador escolhe posição 1 1, variável player recebe um 'X', ex: p1X(RESOLVER)
	if (strcmp(player, "p1") == 0) {
		strcpy(jogada, "X");
	} else {
		strcpy(jogada, "O");
	}
		
	if ((jv->coord1 == 1) && (jv->coord2 == 1)) {
		strcpy(pos11, jogada);
	} else if ((jv->coord1 == 1) && (jv->coord2 == 2)) {	
		strcpy(pos12, jogada);
	} else if ((jv->coord1 == 1) && (jv->coord2 == 3)) {	
		strcpy(pos13, jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 1)) {	
		strcpy(pos21, jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 2)) {	
		strcpy(pos22, jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 3)) {	
		strcpy(pos23, jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 1)) {	
		strcpy(pos31, jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 2)) {	
		strcpy(pos32, jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 3)) {	
		strcpy(pos33, jogada);
	}
	printf("Jogada %d %d realizada com sucesso!\n",jv->coord1, jv->coord2);
	sleep(3);
}

void verifica_ganhador(){
	if (((strcmp(pos11, "X") == 0) && (strcmp(pos12, "X") == 0) && (strcmp(pos13, "X") == 0)) || 
	   ((strcmp(pos21, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos23, "X") == 0)) || 
	   ((strcmp(pos31, "X") == 0) && (strcmp(pos32, "X") == 0) && (strcmp(pos33, "X") == 0)) ||  
	   ((strcmp(pos11, "X") == 0) && (strcmp(pos21, "X") == 0) && (strcmp(pos31, "X") == 0)) ||  
           ((strcmp(pos12, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos32, "X") == 0)) || 
	   ((strcmp(pos13, "X") == 0) && (strcmp(pos23, "X") == 0) && (strcmp(pos33, "X") == 0)) || 
	   ((strcmp(pos11, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos33, "X") == 0)) || 
	   ((strcmp(pos13, "X") == 0) && (strcmp(pos22, "X") == 0) && (strcmp(pos31, "X") == 0))) {
		tabuleiro();
		printf("Jogador 1 ganhou!\n");
		ganhador = true;
	} else if (((strcmp(pos11, "O") == 0) && (strcmp(pos12, "O") == 0) && (strcmp(pos13, "O") == 0)) || 
	   	  ((strcmp(pos21, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos23, "O") == 0)) || 
	   	  ((strcmp(pos31, "O") == 0) && (strcmp(pos32, "O") == 0) && (strcmp(pos33, "O") == 0)) ||  
	   	  ((strcmp(pos11, "O") == 0) && (strcmp(pos21, "O") == 0) && (strcmp(pos31, "O") == 0)) ||  
           	  ((strcmp(pos12, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos32, "O") == 0)) || 
	   	  ((strcmp(pos13, "O") == 0) && (strcmp(pos23, "O") == 0) && (strcmp(pos33, "O") == 0)) || 
	   	  ((strcmp(pos11, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos33, "O") == 0)) || 
	   	  ((strcmp(pos13, "O") == 0) && (strcmp(pos22, "O") == 0) && (strcmp(pos31, "O") == 0))) {
		tabuleiro();
		printf("Jogador 2 ganhou!\n");
		ganhador = true;
	}
}

void grava_log() {
	int fd;
	char nomeLog[50]= " ";
	char texto[5000]=" ";
	char status[50]= " ";

	if (strcmp(player, "p1") == 0) {
		strcpy(nomeLog, "log_jogadas_player1.txt");
	} else if (strcmp(player, "p2") == 0){
		strcpy(nomeLog, "log_jogadas_player2.txt");
	} else {
		strcpy(nomeLog, "log_jogadas_player1.txt");
	}
	
	fd = open(nomeLog, O_RDONLY);
        if (fd == -1) {  //Arquivo não existe
                fd = open(nomeLog, O_CREAT | O_RDWR, S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR);
                if (fd == -1) {
                        perror("Problema na criação!");
                        exit(EXIT_FAILURE);
                }
		if (ganhador) {
			strcpy(status, "jogada ganhadora");
		} else if (!jogadorCorreto) {
			strcpy(status, "jogador incorreto");
		} else if (!coordenadasCorretas) {
			strcpy(status, "jogada invalida");
		} else if (coordenadasCorretas) {
			strcpy(status, "jogada valida");
		}
                
		//Falta descobrir como salvar a data e hora
		strcat(texto, "; ");
		if (strcmp(player, "") == 0) {
			strcat(texto, "p1");
		} else {
			strcat(texto, player);
		}
		strcat(texto, ";");
		strcat(texto, "lance:");
		strcat(texto, jogada);
		strcat(texto, ";");
		strcat(texto, status);
		
		if (write(fd, texto, 100) != 100) perror("log");
        } else {  //Arquivo existe
		fd = open(nomeLog, O_RDWR | O_APPEND);
		if (ganhador) {
			strcpy(status, "jogada ganhadora");
		} else if (!jogadorCorreto) {
			strcpy(status, "jogador incorreto");
		} else if (!coordenadasCorretas) {
			strcpy(status, "jogada invalida");
		} else if (coordenadasCorretas) {
			strcpy(status, "jogada valida");
		}
                
		//Falta descobrir como salvar a data e hora
		strcat(texto, "; ");
		if (strcmp(player, "") == 0) {
			strcat(texto, "p1");
		} else {
			strcat(texto, player);
		}
		strcat(texto, "; ");
		strcat(texto, "lance:");
		strcat(texto, jogada);
		strcat(texto, "; ");
		strcat(texto, status);
		
		if (write(fd, texto, 100) != 100) perror("log");
        }
	close (fd);
}

ssize_t get_msg_buffer_size(mqd_t queue) {
	struct mq_attr attr;

	/*Determina max. msg size; allocate buffer to receive msg */
	if (mq_getattr(queue, &attr) != -1) {
		return attr.mq_msgsize;
	}

	perror("aloca_msg_buffer");
	exit(3);
}
