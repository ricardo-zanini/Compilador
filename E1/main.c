/*
  Programa principal de impressão de tokens.
  Este arquivo será posteriormente substituído.
*/
#include <stdio.h>
#define _(s) #s // https://gcc.gnu.org/onlinedocs/gcc-12.2.0/cpp/Stringizing.html
#include "tokens.h"
#define RET_SUCESSO 0
#define RET_TKNERRO 1
#define RET_DESCONH 2

extern int yylex(void);
extern int yylex_destroy(void);

extern FILE *yyin;
extern char *yytext;
extern int get_line_number (void);

/* protótipos deste módulo - as implementações estão após a main */
void print_token_normal (char *token);
void print_token_especial (int token);
int print_token (int token);

int main (int argc, char **argv) {
  int token = 0, retorno = 0;
  while (retorno == 0 && (token = yylex())) {
    retorno = print_token(token);
  }
  yylex_destroy();
  return retorno;
}

void print_nome(char *token) {
  printf("%d %s [%s]\n", get_line_number(), token, yytext);
}
void print_nome2(int token) {
  printf("%d TK_ESPECIAL [%c]\n", get_line_number(), token);
}
/* A função retorna RET_SUCESSO se o token é conhecido. Caso contrário:
   - retorna RET_TKNERRO se o token é de erro
   - retorna RET_DESCONH se o token é desconhecido */
int print_token(int token) {
  switch (token){
  case '-':
  case '!':
  case '*':
  case '/':
  case '%':
  case '+':
  case '<':
  case '>':
  case '[':
  case ']':
  case '(':
  case ')':
  case '=':
  case ',':
  case ';':
  case '&':
  case '|':   print_nome2 (token);               break;
  case TK_TIPO: print_nome(_(TK_TIPO)); break;
  case TK_VAR: print_nome(_(TK_VAR)); break;
  case TK_SENAO: print_nome(_(TK_SENAO)); break;
  case TK_DECIMAL: print_nome(_(TK_DECIMAL)); break;
  case TK_SE: print_nome(_(TK_SE)); break;
  case TK_INTEIRO: print_nome(_(TK_INTEIRO)); break;
  case TK_ATRIB: print_nome(_(TK_ATRIB)); break;
  case TK_RETORNA: print_nome(_(TK_RETORNA)); break;
  case TK_SETA: print_nome(_(TK_SETA)); break;
  case TK_ENQUANTO: print_nome(_(TK_ENQUANTO)); break;
  case TK_COM: print_nome(_(TK_COM)); break;     
  case TK_OC_LE: print_nome (_(TK_OC_LE)); break;
  case TK_OC_GE: print_nome (_(TK_OC_GE)); break;
  case TK_OC_EQ: print_nome (_(TK_OC_EQ)); break;
  case TK_OC_NE: print_nome (_(TK_OC_NE)); break;
  case TK_LI_INTEIRO: print_nome (_(TK_LI_INTEIRO)); break;
  case TK_LI_DECIMAL: print_nome (_(TK_LI_DECIMAL)); break;
  case TK_ID: print_nome (_(TK_ID));  break;
  case TK_ER: print_nome (_(TK_ER)); return RET_TKNERRO; break;
  default: printf ("<Token inválido com o código %d (%c)>\n", token, token); return RET_DESCONH; break;
  }
  return RET_SUCESSO;
}