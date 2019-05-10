/*****************************************************************************/
/*                  Laborátorio de Sistemas Operacionais                     */
/*                        Trabalho do Grau B (TGB)                           */
/*                       Autores: Fernando e Nadine                          */
/*****************************************************************************/

//includes
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

//Variáveis globais
char pos11[1], pos12[1], pos13[1],
     pos21[1], pos22[1], pos23[1],
     pos31[1], pos32[1], pos33[1];

void tabuleiro(void)
{
	system("clear");
	printf("Feito por Fernando e Nadine\n\n\n");
	printf("            JOGO DA VELHA\n\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos11, pos12, pos13);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n", pos21, pos22, pos23);
	printf("          -----------------\n");
	printf("            %-1s  |  %-1s  |  %-1s  \n\n\n", pos31, pos32, pos33);
}

int main(void)
{
	tabuleiro();
}
