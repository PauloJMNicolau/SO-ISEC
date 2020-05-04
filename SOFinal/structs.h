#include "map.h"
#include "users.h"


typedef struct{
  int pid;
  char mensagem[20];
	char fifo[20];
}pedido_cliente;



typedef struct{
    MAP mapa;
    int idplayer;
    int ativo;
}INFOGAMECLI;


typedef struct{
	char mensagem[20];

}resposta_servidor;


//_______________________


typedef struct {
	char fifo[20];
	int posX;
	int posY;
}Player;

//struct que envia info para o cliente

typedef struct {
int mapa[10][48];
char fifoservidor[70];
int numberLines;
int numberColumns;
}Map;
