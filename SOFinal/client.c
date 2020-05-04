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
#include "personagem.h"

//#define MAX_MSG 20;
#define FSERVUSER "servuser"
#define FSERV "servcmd"
#define FCLI "cli"
#define USER_EXIST "Existe login"

int MAX_MSG = 20;
int fs,fc;
char fifoCliente[20];
pid_t pid;
WINDOW *consola;
MAP mapa;
INFOGAMECLI game;
PERSONAGEM p;


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
                    p.x=i;
                    p.y=j;
                    wattroff(consola, COLOR_PAIR(2));
                    break;
                case 44: //Pacman <
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,ACS_LARROW);
                    p.x=i;
                    p.y=j;
                    wattroff(consola, COLOR_PAIR(2));
                    break;
                case 45: //Pacman Down
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,ACS_UARROW);
                    p.x=i;
                    p.y=j;
                    wattroff(consola,COLOR_PAIR(2));
                    break;
                case 46: //Pacman UP
                    wattron(consola,COLOR_PAIR(2));
                    mvwaddch(consola,i,j,'v');
                    p.x=i;
                    p.y=j;
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

void login(){
	 int n;
   char nome[MAX_MSG];
   char password[MAX_MSG];
	 char c;
   resposta_servidor resposta;
   pedido_cliente pedido;
   USER user;

		printf("Inserir nome: ");
		scanf("%s",nome);
		printf("Inserir password: ");
		scanf("%s",password);
		strcpy(user.nome,nome);
		strcpy(user.password,password);
		strcpy(user.fifo,fifoCliente);
	  if((fs = open(FSERVUSER,O_WRONLY)) == -1){
		 	printf(" nao foi possivel abrir o fifo do servidor para escrita");
		 	exit(1);
		}
	  n = write(fs,&user,sizeof(USER));
		printf("Enviou user\n");
    close(fs);
		// abre o fifo do cliente para receber a resposta do servidor
		if((fc = open(fifoCliente,O_RDONLY)) == -1){
			printf(" nao foi possivel abrir o fifo do servidor para leitura");
			exit(1);
		}

		n = read(fc,&mapa,sizeof(MAP));
		if (n < 0){
				printf("O user nao existe! Iremos desligar a aplicacao!\n");
				exit(1);
		}
		n = read(fc,&p,sizeof(PERSONAGEM));
		if (n < 0){
				printf("O user nao existe! Iremos desligar a aplicacao!\n");
				exit(1);
		}
		close(fc);
		fflush(stdout);
}

void enviaCMD(char c, USER user){
	CMD comand;
	comand.c = c;
	comand.p = p;
	comand.u = user;
	if((fs = open(FSERV,O_WRONLY)) == -1){
		printf(" nao foi possivel abrir o fifo do servidor para escrita");
		exit(1);
	}
	int n = write(fs,&comand,sizeof(CMD));
	printf("Enviou comando\n");
	close(fs);
	// abre o fifo do cliente para receber a resposta do servidor
	if((fc = open(fifoCliente,O_RDONLY)) == -1){
		printf(" nao foi possivel abrir o fifo do servidor para leitura");
		exit(1);
	}

	n = read(fc,&mapa,sizeof(MAP));
	if (n < 0){
			printf("O user nao existe! Iremos desligar a aplicacao!\n");
			exit(1);
	}
	close(fc);
}

int main (void){
	 int pid2;
   int n,fc;
   char msg[MAX_MSG];
   char nome[MAX_MSG];
   char password[MAX_MSG];
   pedido_cliente pedido;
   resposta_servidor resposta;
   INFOGAMECLI game;
   USER user;
	 char c;

 	pid2 = (int)getpid(); // Recebe o PID e converte(cast) para inteiro
 	sprintf(fifoCliente, "%s%d", FCLI, pid2); // Converte o inteiro do pid para string para mais tarde ser usado como nome do fifo
	if (mkfifo(fifoCliente, 0600) == -1) {
		printf("Não foi possivel criar o fifo do cliente\n");
		exit(1);
	}
	login();

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
	updateMapa(mapa);
	refresh();
	do{
		updateMapa(mapa);
		refresh();
    c = wgetch(consola);
		if (c != ERR){
        if ( c == KEY_UP){
            p.dir = 'u';
        }
        else if (c == KEY_DOWN){
            p.dir = 'b';
        }
        else if (c == KEY_LEFT){
            p.dir = 'l';
        }
        else if (c == KEY_RIGHT){
            p.dir = 'r';
        }
				enviaCMD(c, user);
			}


	}while(1);

close(fs);
unlink(nome);
close(fc);
exit(0);
}
