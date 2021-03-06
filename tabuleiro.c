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
char pos[3][3];
char jogada[1];
char player[2];
char dataHora[100];
bool ganhador = false;
bool jogadorCorreto = true;
bool coordenadasCorretas = true;
struct sigaction sinal;
sigset_t newmask, oldmask;

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
void grava_log(TJogoVelha* jv);
void PIPE_data_hora();
void initiate_handler(struct sigaction* sign);

//Método main
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

	initiate_handler(&sinal);

	while(!ganhador){

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

		TJogoVelha* jv = ((TJogoVelha*) buffer);

		valida_coordenadas(jv);
		valida_jogada(jv);

		if ((jogadorCorreto) && (coordenadasCorretas)) {
			realiza_jogada(jv);
			verifica_ganhador();
			strcpy(player, jv->playerID);
		}
		grava_log(jv);	
	}

	//Liberar descritor (mq_cdeve acumular o pesolose)
	mq_unlink(NOME_FILA);
	mq_close(queue);	
	printf("Fim!\n");

	exit(EXIT_SUCCESS);
}

//Método void
//Responsável por imprimir na tela o tabuleiro e a situação atual do jogo
void tabuleiro() {
	system("clear");
	printf("Feito por Nadine e Fernando\n\n\n");
	printf("            JOGO DA VELHA\n\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos[0], pos[1], pos[2]);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos[3], pos[4], pos[5]);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n\n\n", pos[6], pos[7], pos[8]);
}

//Método void
//Recebe o buffer da mensagem de TJogoVelha e valida se o jogador informado é o esperado
//Caso o jogador informado não seja compátivel com o esperado é emitido um alerta ao jogador
void valida_jogada(TJogoVelha* jv) {
	if (coordenadasCorretas) {
		if (strlen(player) == 0) {
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

//Método void que informa qual o jogador da vez
void mensagem_jogador() {
	if ((strlen(player) == 0) || (strcmp(player, "p2") == 0)) {
		printf("Aguardando jogada do Jogador 1:\n");
	} else if (strcmp(player, "p1") == 0){
		printf("Aguardando jogada do Jogador 2:\n");
	} 
}

//Método void, recebe por parametro o buffer TJogoVelha
//Verifica as coordenadas recebidas e faz a validação
//Caso as coordenadas estejam fora do espaço do tabuleiro ou a posição já esteja preenchida é emitido um alerta ao usuário para jogar novamente
void valida_coordenadas(TJogoVelha* jv) {
	if (((jv->coord1 > 3) || (jv->coord1 < 1)) ||
            ((jv->coord2 > 3) || (jv->coord2 < 1))) {
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else {
		coordenadasCorretas = true;
	}
	if(((jv->coord1 == 1) && (jv->coord2 == 1)) && (strlen(pos[0]) != 0)){
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 1) && (jv->coord2 == 2)) && (strlen(pos[1]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 1) && (jv->coord2 == 3)) && (strlen(pos[2]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 1)) && (strlen(pos[3]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 2)) && (strlen(pos[4]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 2) && (jv->coord2 == 3)) && (strlen(pos[5]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if(((jv->coord1 == 3) && (jv->coord2 == 1)) && (strlen(pos[6]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if((jv->coord1 == 3) && (jv->coord2 == 2) && (strlen(pos[7]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	} else if((jv->coord1 == 3) && (jv->coord2 == 3) && (strlen(pos[8]) != 0)){ 
		printf("Jogada inválida! Tente novamente.\n");
		sleep(3);
		coordenadasCorretas = false;
	}
}

//Método void, recebe por parametro o buffer TJogoVelha
//Verifica as coordenadas recebidas e executa a jogada
void realiza_jogada(TJogoVelha* jv) {
	if ((strlen(player) == 0) || (strcmp(player, "p1") == 0)) {
		strcpy(jogada, "X");
	} else {
		strcpy(jogada, "O");
	}
		
	if ((jv->coord1 == 1) && (jv->coord2 == 1)) {
		strcpy(pos[0], jogada);
	} else if ((jv->coord1 == 1) && (jv->coord2 == 2)) {	
		strcpy(pos[1], jogada);
	} else if ((jv->coord1 == 1) && (jv->coord2 == 3)) {	
		strcpy(pos[2], jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 1)) {	
		strcpy(pos[3], jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 2)) {	
		strcpy(pos[4], jogada);
	} else if ((jv->coord1 == 2) && (jv->coord2 == 3)) {	
		strcpy(pos[5], jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 1)) {	
		strcpy(pos[6], jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 2)) {	
		strcpy(pos[7], jogada);
	} else if ((jv->coord1 == 3) && (jv->coord2 == 3)) {	
		strcpy(pos[8], jogada);
	}
	printf("Jogada %d %d realizada com sucesso!\n",jv->coord1, jv->coord2);
	sleep(3);
}

//Método void responsável por verificar quem foi o vencedor do jogo
void verifica_ganhador(){
	if (((strcmp(pos[0], "X") == 0) && (strcmp(pos[1], "X") == 0) && (strcmp(pos[2], "X") == 0)) || 
	   ((strcmp(pos[3], "X") == 0) && (strcmp(pos[4], "X") == 0) && (strcmp(pos[5], "X") == 0)) || 
	   ((strcmp(pos[6], "X") == 0) && (strcmp(pos[7], "X") == 0) && (strcmp(pos[8], "X") == 0)) ||  
	   ((strcmp(pos[0], "X") == 0) && (strcmp(pos[3], "X") == 0) && (strcmp(pos[6], "X") == 0)) ||  
           ((strcmp(pos[1], "X") == 0) && (strcmp(pos[4], "X") == 0) && (strcmp(pos[7], "X") == 0)) || 
	   ((strcmp(pos[2], "X") == 0) && (strcmp(pos[5], "X") == 0) && (strcmp(pos[8], "X") == 0)) || 
	   ((strcmp(pos[0], "X") == 0) && (strcmp(pos[4], "X") == 0) && (strcmp(pos[8], "X") == 0)) || 
	   ((strcmp(pos[2], "X") == 0) && (strcmp(pos[4], "X") == 0) && (strcmp(pos[6], "X") == 0))) {
		tabuleiro();
		printf("Jogador 1 ganhou!\n");
		ganhador = true;
	} else if (((strcmp(pos[0], "O") == 0) && (strcmp(pos[1], "O") == 0) && (strcmp(pos[2], "O") == 0)) || 
	   ((strcmp(pos[3], "O") == 0) && (strcmp(pos[4], "O") == 0) && (strcmp(pos[5], "O") == 0)) || 
	   ((strcmp(pos[6], "O") == 0) && (strcmp(pos[7], "O") == 0) && (strcmp(pos[8], "O") == 0)) ||  
	   ((strcmp(pos[0], "O") == 0) && (strcmp(pos[3], "O") == 0) && (strcmp(pos[6], "O") == 0)) ||  
           ((strcmp(pos[1], "O") == 0) && (strcmp(pos[4], "O") == 0) && (strcmp(pos[7], "O") == 0)) || 
	   ((strcmp(pos[2], "O") == 0) && (strcmp(pos[5], "O") == 0) && (strcmp(pos[8], "O") == 0)) || 
	   ((strcmp(pos[0], "O") == 0) && (strcmp(pos[4], "O") == 0) && (strcmp(pos[8], "O") == 0)) || 
	   ((strcmp(pos[2], "O") == 0) && (strcmp(pos[4], "O") == 0) && (strcmp(pos[6], "O") == 0))) {
		tabuleiro();
		printf("Jogador 2 ganhou!\n");
		ganhador = true;
	} else if ((((strlen(pos[0]) != 0) && (strlen(pos[1]) != 0) && (strlen(pos[2]) != 0)) && 
	   ((strlen(pos[3]) != 0) && (strlen(pos[4]) != 0) && (strlen(pos[5]) != 0)) && 
	   ((strlen(pos[6]) != 0) && (strlen(pos[7]) != 0) && (strlen(pos[8]) != 0)) &&  
	   ((strlen(pos[0]) != 0) && (strlen(pos[3]) != 0) && (strlen(pos[6]) != 0)) &&  
           ((strlen(pos[1]) != 0) && (strlen(pos[4]) != 0) && (strlen(pos[7]) != 0)) && 
	   ((strlen(pos[2]) != 0) && (strlen(pos[5]) != 0) && (strlen(pos[8]) != 0)) && 
	   ((strlen(pos[0]) != 0) && (strlen(pos[4]) != 0) && (strlen(pos[8]) != 0)) && 
	   ((strlen(pos[2]) != 0) && (strlen(pos[4]) != 0) && (strlen(pos[6]) != 0))) && (ganhador != true)) {
		tabuleiro();
		printf("Ninguém ganhou, o jogo empatou!\n");
		ganhador = true;
	}
}

//Método responsável por capturar as jogadas e ações do jogo
//Cria dois arquivos de log com as capturas feitas ao longo do jogo, um para cada jogador
void grava_log(TJogoVelha* jv) {
	int fd;
	char nomeLog[50]= " ";
	char texto[5000] = " ";
	char status[50]= " ";
	char coord[1] = " ";

	if ((strlen(player) == 0) || (strcmp(player, "p1") == 0)) {
		strcpy(nomeLog, "log_jogadas_player1.txt");
	} else if (strcmp(player, "p2") == 0){
		strcpy(nomeLog, "log_jogadas_player2.txt");
	}
	
	fd = open(nomeLog, O_RDONLY);
        if (fd == -1) {  //Arquivo ainda não existe
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
                
		PIPE_data_hora();
		strcat(texto, dataHora);
		strcat(texto, ";");
		strcat(texto, "Jogador:");
		if (strlen(player) == 0) {
			strcat(texto, "p1");
		} else {
			strcat(texto, player);
		}
		strcat(texto, ";");
		strcat(texto, "lance:");
		sprintf(coord, "%d", jv->coord1);
		strcat(texto, coord);
		strcat(texto, " ");
		sprintf(coord, "%d", jv->coord2);
		strcat(texto, coord);
		strcat(texto, ";");
		strcat(texto, status);
		
		if (write(fd, texto, 100) != 100) perror("log");
        } else {  //Arquivo já existe
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
                
		PIPE_data_hora();
		strcat(texto, dataHora);
		strcat(texto, ";");
		strcat(texto, "Jogador:");
		if (strlen(player) == 0) {
			strcat(texto, "p1");
		} else {
			strcat(texto, player);
		}
		strcat(texto, ";");
		strcat(texto, "lance:");
		sprintf(coord, "%d", jv->coord1);
		strcat(texto, coord);
		strcat(texto, " ");
		sprintf(coord, "%d", jv->coord2);
		strcat(texto, coord);
		strcat(texto, ";");
		strcat(texto, status);
		
		if (write(fd, texto, 100) != 100) perror("log");
        }
	close (fd);
}

//Método PIPE para pegar a saída do comando date (IPC adicional)
void PIPE_data_hora() {
	int pfd[2];
	if (pipe(pfd) != 0) perror("pipe()");
	else
		switch(fork()) {
		case 0:
			close(pfd[0]);    //fecha a leitura
			dup2(pfd[1], 1);  //Envia a saída do exec para o pipe
			dup2(pfd[1], 2);  // envia stderr para o pipe
			close(pfd[1]);    //fecha a escrita
			execlp("date", "date", NULL);
			perror("exec 'wc'"); break;
		default: 
			close(pfd[1]);    //fecha a escrita
			while(read(pfd[0], dataHora, sizeof(dataHora)) != 0) {} //Grava saída do exec na variável
			
		}
}

//Método responsável por definir o tamanho do buffer
ssize_t get_msg_buffer_size(mqd_t queue) {
	struct mq_attr attr;

	/*Determina max. msg size; allocate buffer to receive msg */
	if (mq_getattr(queue, &attr) != -1) {
		return attr.mq_msgsize;
	}

	perror("aloca_msg_buffer");
	exit(3);
}

//Instanciador de sinais
//Recebe por parâmetro o sinal e bloqueia o sinal recebido.
void initiate_handler(struct sigaction* sign){
	sigfillset(&newmask);
 	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

}


