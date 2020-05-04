#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <curses.h>
#include <locale.h>
#include <ncurses.h>

#include "structs.h"
#include "users.h"
#include "map.h"
#include "personagem.h"

#define FSERVUSER "servuser"
#define FSERV "servcmd"
#define LOGIN "users.txt"

//Tipo de comandos
#define UP "up"
#define DOWN "down"
#define LEFT "left"
#define RIGHT "right"

Player player[5];


int numberPlayers = -1;

pthread_t threadUser, threadCmd; //Threads
WINDOW *consola;
PERSONAGEM pac;
USER users[30];
int fs, fsc;
MAP mapa, labirinto;




/* ----------------------------Estrutura
	respondeCliente -> trata de enviar a informação para o cliente
	Loading -> trata de invocar as funções que fazem load para memoria a informação
	loadLogin ->  trata de ler o ficheiro e carregar para memoria o user e pass

*/

/* Encerra Servidor */
void encerraServidor(int s){
    endwin();
    printf("Vou Encerrar");
    pthread_cancel(threadUser);
    pthread_cancel(threadCmd);
    printf("\nAdeus");
    close(fs);
    close(fsc);
    unlink(FSERVUSER);
    unlink(FSERV);
    exit(0);
}

/* Mostra Mapa*/
void updateMapa(MAP m){
    int i,j;
    for (i=0; i< m.maxl; i++){
        for (j=0; j<m.maxv; j++){
            int c= m.posicao[i][j];
            switch (c){
                case 0: // Espaço Branco
                    wattron(consola, COLOR_PAIR(1));
                    mvwaddch(consola,i,j,' ');
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 43: //Pacman >
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,ACS_RARROW);
                    pac.x=i;
                    pac.y=j;
                    wattroff(consola, COLOR_PAIR(2));
                    break;
                case 44: //Pacman <
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,ACS_LARROW);
                    pac.x=i;
                    pac.y=j;
                    wattroff(consola, COLOR_PAIR(2));
                    break;
                case 45: //Pacman Down
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,ACS_UARROW);
                    pac.x=i;
                    pac.y=j;
                    wattroff(consola,COLOR_PAIR(2));
                    break;
                case 46: //Pacman UP
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,'v');
                    pac.x=i;
                    pac.y=j;
                    wattroff(consola,COLOR_PAIR(2));
                    break;
                case 97: // Porta
                    wattron(consola,COLOR_PAIR(4));
                    mvwaddch(consola,i,j,ACS_CKBOARD);
                    wattroff(consola,COLOR_PAIR(4));
                    break;
                case 106: //Canto BR
                    wattron(consola,COLOR_PAIR(1));
                    mvwaddch(consola,i,j,ACS_LRCORNER);
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 107: // Canto SR
                    wattron(consola,COLOR_PAIR(1));
                    mvwaddch(consola,i,j,ACS_URCORNER);
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 108: //Canto SL
                    wattron(consola,COLOR_PAIR(1));
                    mvwaddch(consola,i,j,ACS_ULCORNER);
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 109: //Canto BL
                    wattron(consola,COLOR_PAIR(1));
                    mvwaddch(consola,i,j,ACS_LLCORNER);
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 112: //Linha H
                    wattron(consola,COLOR_PAIR(1));
                    mvwaddch(consola,i,j,ACS_HLINE);
                    wattroff(consola,COLOR_PAIR(1));
                    break;
                case 120:
                    wattron(consola, COLOR_PAIR(1));
                    mvwaddch(consola, i,j,ACS_VLINE);
                    wattroff(consola, COLOR_PAIR(1));
                    break;
                case 183:
                    wattron(consola, COLOR_PAIR(3));
                    mvwaddch(consola,i,j,ACS_BULLET);
                    wattroff(consola, COLOR_PAIR(3));
                    break;
                default:
                    break;

            }
        }
    }
    mvwprintw(consola,m.maxl,2, "PONTOS: ");
    mvwprintw(consola,m.maxl,20, "VIDAS: ");
    int z;
    for( z=26; z <26+5; z++){
        wattron(consola, COLOR_PAIR(5));
        mvwaddch(consola, m.maxl,z, ACS_DIAMOND);
        wattroff(consola, COLOR_PAIR(5));
    }
    wnoutrefresh(consola);
}

/* Ler Ficheiro Mapa */
MAP lerFicheiroMapa(char file[],FILE * fmid){
    MAP m;
    int i = 0,j;
    char aux[300];
    char *token;
    fmid = fopen(file, "r");
    if (fmid == NULL){
        printf("Erro: Ficheiro Inexistente");
        exit(5);
    }
    fgets(aux,4,fmid);
    m.maxv = atoi(aux);
    fgets(aux,4,fmid);
    m.maxl = atoi(aux);
    while (fgets(aux,300, fmid)) {
        token=(char *)strtok(aux,";");
        j=0;
        while (token != NULL) {
            m.posicao[i][j] = atoi(token);
            token=(char*)strtok(NULL, ";");
            j++;
        }
        i++;
    }
    return m;
}

/* Copiar Mapa */
void copiaMapa(MAP base, MAP * copia){
    (*copia).maxl = base.maxl;
    (*copia).maxv = base.maxv;
    int i,j;
    for ( i=0; i< base.maxl ;i++){
        for ( j =0; j< base.maxv; j++){
            (*copia).posicao[i][j]= base.posicao[i][j];
        }
    }
}

/* Ler Ficheiro Utilizadores */
void lerFicheiroUser(char file[], FILE * fuid, USER * users){
    int i = 0;
    char aux[50];
    char aux2[20];
    fuid = fopen(file, "r");
    if (fuid == NULL){
        printf("Erro: Ficheiro Inexistente");
        exit(2);
    }
    while (fgets(aux,50, fuid)) {
        strncpy(users[i].nome,strtok(aux, ":"), 20);
        strncpy(aux2,strtok(NULL, aux),20);
        strcpy(users[i].password, strtok(aux2, "\n"));
        i++;
    }
}

/* Verifica Utilizador */
int verifyLogin(USER login){
	int i;
	int len = sizeof(users)/sizeof(users[0]);
	for(i = 0; i < len;i++){
		if( strcmp(users[i].nome,login.nome) == 0){
      if (strcmp(users[i].password, login.password) == 0){
        printf("ok");
			  return 1;
      }
		}
	}
	return 0;
}

void enviaMapa(USER user){
	int fs = open(user.fifo,O_WRONLY);
	if(fs == -1){
	   printf("Responde Cliente: nao foi possivel abrir o fifo do servidor para escrita");
		 exit(1);
	}
	int n = write(fs,&labirinto,sizeof(MAP));
  if (n > 0){
    printf("Mapa Enviado");
  }
  n = write(fs,&pac,sizeof(PERSONAGEM));
  if (n > 0){
    printf("Personagem Enviado");
  }
  close(fs);
}


/* Testa Colisões */
int colisonDetect(int x, int y,char direction, MAP * m){
    switch(direction){
        case 'u':
            if((*m).posicao[x-1][y] != 106 && (*m).posicao[x-1][y] != 107 && (*m).posicao[x-1][y] != 108 &&
               (*m).posicao[x-1][y] != 109 && (*m).posicao[x-1][y] != 112 && (*m).posicao[x-1][y] != 120 ){
                return 1;
            }
            break;
        case 'd':
            if((*m).posicao[x+1][y] != 106 && (*m).posicao[x+1][y] != 107 && (*m).posicao[x+1][y] != 108 &&
               (*m).posicao[x+1][y] != 109 && (*m).posicao[x+1][y] != 112 && (*m).posicao[x+1][y] != 120 ){
                return 1;
            }
            break;
        case 'l':
            if((*m).posicao[x][y-1] != 106 && (*m).posicao[x][y-1] != 107 && (*m).posicao[x][y-1] != 108 &&
               (*m).posicao[x][y-1] != 109 && (*m).posicao[x][y-1] != 112 && (*m).posicao[x][y-1] != 120 ){
                return 1;
            }
            break;
        case 'r':
            if((*m).posicao[x][y+1] != 106 && (*m).posicao[x][y+1] != 107 && (*m).posicao[x][y+1] != 108 &&
               (*m).posicao[x][y+1] != 109 && (*m).posicao[x][y+1] != 112 && (*m).posicao[x][y+1] != 120 ){
                return 1;
            }
            break;
    }
    return -1;
}

/* Mover Personagem */
void movePersonagem(int c, PERSONAGEM p, MAP * m){
    int res;
        switch (c){
            case KEY_UP:
                res = colisonDetect(p.x, p.y,'u', m);
                if (res == 1){
                    (*m).posicao[p.x-1][p.y]= 46;
                    (*m).posicao[p.x][p.y]= 0;
                }
                break;
            case KEY_DOWN:
                res = colisonDetect(p.x, p.y, 'd', m);
                if (res == 1){
                    (*m).posicao[p.x+1][p.y]= 45;
                    (*m).posicao[p.x][p.y]= 0;
                }
                break;
            case KEY_LEFT:
                res = colisonDetect(p.x, p.y, 'l', m);
                if (res == 1){
                    if(p.x == 11 && p.y-1 == 1){
                        (*m).posicao[p.x][31]= 43;
                        (*m).posicao[p.x][p.y]= 0;
                    } else {
                        (*m).posicao[p.x][p.y-1]= 43;
                        (*m).posicao[p.x][p.y]= 0;
                    }
                }
                break;
            case KEY_RIGHT:
                res =colisonDetect(p.x, p.y, 'r', m);
                if (res == 1){
                    if(p.x == 11 && p.y+1== 32){
                        (*m).posicao[p.x][2]= 44;
                        (*m).posicao[p.x][p.y]= 0;
                    } else {
                        (*m).posicao[p.x][p.y+1]= 44;
                        (*m).posicao[p.x][p.y]= 0;
                    }
                }
                break;
        }
}


/* Thread Recebe Cliente */
void* recebeUser(void * arg){
    int aux;
    int n;
    char tmp[70];
    USER user;
    do{
        fs = open(FSERVUSER, O_RDONLY);//ABRIR FIFO DO SERVIDOR PARA LEITURA
        if (fs == -1){
            printf("Não consegui abrir o Pipe do servidor para escrita");
            exit(1);
        }
		    n = read(fs, &user, sizeof(USER)); // lê o pedido
        printf("Pedido: %s\n",user.nome);
        aux = verifyLogin(user);
        if ( aux == 1){ // verifica se ele existe
	  	      enviaMapa(user);
        }
        fflush(stdout);
    }while(1);
}

/* Thread Recebe Cliente */
void* recebeCmd(void * arg){
  int n;
  CMD comand;
  do{
      fs = open(FSERV, O_RDONLY);//ABRIR FIFO DO SERVIDOR PARA LEITURA
      if (fs == -1){
          printf("Não consegui abrir o Pipe do servidor para escrita");
          exit(1);
      }
      n = read(fs, &comand, sizeof(CMD)); // lê o pedido
      close(fs);
      printf("Comando: %c\n",comand.c);
      movePersonagem(comand.c,comand.p, &labirinto);
      enviaMapa(comand.u);
  }while(1);
}

int main(int argc, char * argv[]){
    /* Ponteiros de Ficheiros */
    FILE *fuid = NULL, *fmid = NULL;
    int pth_ID, pth_Cmd; //Threads
    char map[30]= "map.txt";
    signal(SIGINT, encerraServidor);
    setlocale(LC_CTYPE, "en_US.UTF-8");
    /*NCurses Inicio */
    initscr();
    consola = newwin(40,40,0,0);
    keypad(consola,TRUE);
    nonl();
    cbreak();
    nodelay(consola, TRUE);
    curs_set(FALSE);

    if (has_colors()){
      start_color();
      init_pair(1, COLOR_BLUE, COLOR_BLACK);
      init_pair(2, COLOR_YELLOW, COLOR_BLACK);
      init_pair(3, COLOR_GREEN, COLOR_BLACK);
      init_pair(4, COLOR_WHITE,COLOR_BLACK);
      init_pair(5, COLOR_RED,COLOR_BLACK);
      init_pair(6, COLOR_CYAN,COLOR_BLACK);
    }


/*Hack*/
    argc = 2;
    argv[1] = "users.txt";
/**/



    if (argc != 2){
        printf("Erro: Servidor deve receber um ficheiro de texto como argumento\n");
        printf("Sintaxe: server nome_ficheiro\n");
        exit(1);
    }
    lerFicheiroUser(argv[1], fuid, users);
	  if (access(FSERVUSER, F_OK) == 0){  	//Verificar se já existe um servidor aberto
        printf("Já existe um servidor\n");
        exit(1);
    }
    if ((mkfifo(FSERVUSER, 0600)) == -1){  	//Criar FIFO do servidor
        printf("Não foi possível criar o Pipe do servidor");
        exit(1);
    }

    if (access(FSERV, F_OK) == 0){  	//Verificar se já existe um servidor aberto
        printf("Já existe um servidor\n");
        exit(1);
    }
    if ((mkfifo(FSERV, 0600)) == -1){  	//Criar FIFO do servidor
        printf("Não foi possível criar o Pipe do servidor");
        exit(1);
    }



    mapa = lerFicheiroMapa(map,fmid);
    copiaMapa(mapa, &labirinto);
    /* Criação das Thread */
    pth_ID = pthread_create(&threadUser, NULL ,recebeUser ,NULL);
    if (pth_ID != 0){
        printf("Erro: Servidor não consegue receber Utilizadores");
        exit(4);
    }
    pth_Cmd = pthread_create(&threadCmd, NULL ,recebeCmd ,NULL);
    if (pth_Cmd != 0){
        printf("Erro: Servidor não consegue receber Comandos");
        exit(4);
    }
    while(1){
      updateMapa(labirinto);
      refresh();

    }



   	exit(0);
//}

}
