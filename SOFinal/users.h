#ifndef UTILIZADORES_H
#define UTILIZADORES_H

#include "personagem.h"

typedef struct User{
  char nome[25];
	char password[25];
	char fifo[20];
} USER;

typedef struct Cmd{
  char c;
  PERSONAGEM p;
  USER u;
}CMD;
#endif /* UTILIZADORES_H */
