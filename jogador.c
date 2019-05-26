/*****************************************************************************/
/*                  Laborátorio de Sistemas Operacionais                     */
/*                        Trabalho do Grau B (TGB)                           */
/*                       Autores: Nadine e Fernando                          */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

//Nome da fila
const char* NOME_FILA = "/jogoVelha";

//Estrutura de dados para a mensagem
typedef struct JogoVelha {
	char playerID[2];
	int coord1;
	int coord2;
} TJogoVelha;

int main(int argc, char* argv[]) {
	//Declaração da fila
	mqd_t queue;
	//Declaração da mensagem
	TJogoVelha j;
	char* jogador = argv[2];

	//Obter descritor (mq_open+O_WRONLY+O_CREAT)
	queue = mq_open(NOME_FILA, O_WRONLY | O_CREAT, 0770, NULL);
	if (queue == (mqd_t) -1) {
		perror("mq_open");
		exit(2);
	}

	//Montar a mensagem
	strncpy(j.playerID, jogador, 29);
	j.coord1 = atoi(argv[2]);
	j.coord2 = atoi(argv[3]);

	//Enviar (mq_send)
	if (mq_send(queue, (const char*) &j, sizeof(TJogoVelha), 29) != 0) {
		perror("send #29");
	}

	//Liberar descritor (mq_close)
	mq_close(queue);

	printf("Mensagem enviada!\n");
	exit(EXIT_SUCCESS);
}
