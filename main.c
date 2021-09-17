#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

char buffer[sizeof(int *)]; // buffer usado para transmitir a variavel ao pai
//int tamanho; // variavel p passar o tamanho do número p leitura do pai --> retirado
pid_t child_pid; // copiado da aula e/s
int evita_loop_errado = 0; // explicação na funçao main
int comandoParaExecutar = 0;
int descritoresPIPE[2]; // descritor que sera do pai e herdado pelo filho
//descritoresPipe[0] - leitura do pipe
//descritoresPipe[1] - escrita no pipe

void signal_handler(int sigNumber){

  if (sigNumber == SIGTERM){
    printf("Finalizando o disparador...\n");
    evita_loop_errado = 15; //para o loop
  }

  if (sigNumber == SIGUSR1){
    printf("SIGUSR1\n");
    evita_loop_errado = 10;
    pipe(descritoresPIPE); // por chamar pipe, não precisa chamar o open
    child_pid = fork();
    if (child_pid == 0) { //ai é o filho

    // trecho tirado de http://www.cplusplus.com/forum/beginner/230838/

      // Get the system time.
      unsigned seed = time(0);
   
      // Seed the random number generator.
      srand(seed);
    //###########################################

      int aleatorio = 1 + (rand() % 100); // aleatorio de 1 a 100
      printf("Olha, pai. Meu número aleatório é: %d\n",aleatorio);
      close(descritoresPIPE[0]); // fecha leitura, se for necessário
      sprintf( buffer, "%d", aleatorio ); // adaptado de https://pt.stackoverflow.com/questions/336172/para-que-serve-o-sscanf-e-sprintf
					  // não funcionaria colocando direto a variavel, preciso do buffer. (é, eu tentei...)
      write(descritoresPIPE[1], buffer, sizeof(int *));
      //tamanho = sizeof(aleatorio);
      close(descritoresPIPE[1]); 
    }else{
      close(descritoresPIPE[1]); // fecha escrita, se for necessário
      evita_loop_errado= 0; // volta a esperar sinal
      wait(NULL);
      read(descritoresPIPE[0], buffer, sizeof(int *));
      sscanf(buffer, "%d", &comandoParaExecutar); // adaptado de https://pt.stackoverflow.com/questions/336172/para-que-serve-o-sscanf-e-sprintf
      printf("Oi, filho. Legal ter sorteado o número %d\n", comandoParaExecutar);
      close(descritoresPIPE[0]);

    }
  }

  if (sigNumber == SIGUSR2){
    printf("SIGUSR2\n");
    evita_loop_errado = 12;
    child_pid = fork(); // filho recebe tudo que o pai tem acesso, inclusive a variavel comandoParaExecutar
    if (child_pid == 0) { //ai é o filho
      if (comandoParaExecutar == 0){
        printf("Não há comando a executar\n");
        
      }else if (comandoParaExecutar % 2 == 0){
        execlp("/bin/ping", "ping", "-c 5", "8.8.8.8", NULL); // adaptado da aula e  https://stackoverflow.com/questions/55282827/execl-using-an-integer-in-arguments-ping
      
      }else{
        execlp("/bin/ping", "ping","-c 5", "-i 2", "paris.testdebit.info", NULL);
      }
    
    }else{
      evita_loop_errado = 0;
      wait(NULL);
    }
  }
}


int main(void) {
  //int pid = getpid();
  printf("%d\n",getpid());

  while(evita_loop_errado == 0) { // essa variavel vai ser usada para controlar 
  //quando ela deve ficar em loop ou nao. se não for usada e o sinal 
  //sigusr1 for chamado, o filho vai entrar em execução e ficar num loop infinito
    if(signal(SIGTERM, signal_handler)==SIG_ERR)
      printf("deu caô no SIGTERM");
    if(signal(SIGUSR1, signal_handler)==SIG_ERR)
      printf("deu caô no SIGUSR1");
    if(signal(SIGUSR2, signal_handler)==SIG_ERR)
      printf("deu caô no SIGUSR2");
  }

return 0;
}

/*
O programa foi desenvolvido num desenvolvimento de programação online (https://replit.com/)
com o sistema operacional Ubuntu, mas testado no Debian também.
Foram feitas algumas adaptações de códigos vistos em aula, assim como links na internet.
*/