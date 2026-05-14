#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#define MAX_LINE 1024

void tratador_sigint(int sig) {
    (void)sig;
    printf("\n"); 
    rl_on_new_line(); 
    
    rl_replace_line("", 0); 
}

int main(){
    char linha[MAX_LINE];
    char *args[64];

    signal(SIGINT, tratador_sigint);

    while(1){
        while (waitpid(-1, NULL, WNOHANG) > 0);

        char *linha_comando = readline("meu_shell> ");

        if (linha_comando == NULL) {
            printf("\n");
            break;
        }

        if (strlen(linha_comando) > 0) {
            add_history(linha_comando);
        }

        strncpy(linha, linha_comando, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        free(linha_comando);

        int i = 0;

        char *token = strtok(linha, " \t\r\n\a");

        while(token != NULL){
            args[i] = token;
            i++;
            token = strtok(NULL, " \t\r\n\a");
        }

        args[i] = NULL;

        int pos_pipe = -1;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "|") == 0) {
                pos_pipe = j;
                args[j] = NULL; //substitui | por NULL
                break;
            }
        }

        //encontra pipe
        if (pos_pipe != -1) {
            //os comandos são divididos em dois
            char **cmd1 = args; 
            char **cmd2 = &args[pos_pipe + 1];

            int fd[2];
            if (pipe(fd) == -1) {
                perror("Erro ao criar o pipe");
                continue;
            }

            //cria o primeiro filho, que pega o file descriptor 1
            pid_t pid1 = fork();
            if (pid1 == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                
                signal(SIGINT, SIG_DFL);
                execvp(cmd1[0], cmd1);
                perror("Erro no cmd1 do pipe");
                exit(EXIT_FAILURE);
            }

            //cria o segundo filho, que pega o file descriptor 0
            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
                
                signal(SIGINT, SIG_DFL);
                execvp(cmd2[0], cmd2);
                perror("Erro no cmd2 do pipe");
                exit(EXIT_FAILURE);
            }

            close(fd[0]);
            close(fd[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            
            continue; 
        }

        int redirecionar_background = 0;
        
        //procura o simbolo & entre os tokens
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], "&") == 0) {
                args[k] = NULL; //esconde o & para o execvp não ver
                redirecionar_background = 1;
                break; 
            }
        }

        int redirecionar_saida = 0;
        char *arquivo_saida = NULL;

        //procura o simbolo > entre os tokens
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], ">") == 0) {
                args[j] = NULL; //esconde o > para o execvp não ver
                arquivo_saida = args[j+1]; //nome do arquivo
                redirecionar_saida = 1;
                break;
            }
        }

        int redirecionar_entrada = 0;
        char *arquivo_entrada = NULL;

        //procura o simbolo < entre os tokens
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "<") == 0) {
                args[j] = NULL; //esconde o < para o execvp não ver
                arquivo_entrada = args[j+1]; //nome do arquivo
                redirecionar_entrada = 1;
                break; 
            }
        }

        if(args[0] != NULL && strcmp(args[0], "exit") == 0){
            break;
        }

        //comando cd
        if (args[0] != NULL && strcmp(args[0], "cd") == 0) {
            //verifica se existe um destino
            if (args[1] == NULL) {
                fprintf(stderr, "my_shell: esperado argumento para \"cd\"\n");
            } else {
                //muda o current working directory
                if (chdir(args[1]) != 0) {
                    perror("my_shell"); //erro caso a pasta não exista
                }
            }
            continue; //ignora o fork nessa iteração
        }

        //caso usuario aperte enter sem escrever nada
        if (args[0] == NULL) {
            continue;
        }
     
        pid_t pid = fork(); // 1. O Sistema clona o seu programa neste exato milissegundo

        //se pid for menor que 0 o fork falhou
        if (pid < 0) {
            perror("Falha no fork");
        
        //se pid for igual a 0 este é o processo filho
        } else if (pid == 0) {
            //verifica tag redirecionar saida
            if (redirecionar_saida == 1 && arquivo_saida != NULL) {
                //abre ou cria o arquivo 
                //write only, create caso não exista ainda, truncate para sobrescrever caso já exista 
                //0644 - read e write para o user e read p outros users
                int fd_out = open(arquivo_saida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("Erro ao abrir/criar o arquivo de saída");
                    exit(EXIT_FAILURE);
                }
                
                //substitui o file descriptor 1 (STDOUT) pelo file descriptor do arquivo do comando
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out); //fecha o file descriptor original
            }
            //verifica tag redirecionar entrada
            if (redirecionar_entrada == 1 && arquivo_entrada != NULL) {
                //abre o arquivo em read only
                int fd_in = open(arquivo_entrada, O_RDONLY);
                if (fd_in < 0) {
                    perror("Erro ao abrir o arquivo de entrada");
                    exit(EXIT_FAILURE);
                }
                
                //substitui o file descriptor 0 (STDIN) pelo file descriptor do arquivo do comando
                dup2(fd_in, STDIN_FILENO);
                close(fd_in); //fecha o file descriptor original
            }

            //se execvp retornar -1 o comando não existe no linux
            signal(SIGINT, SIG_DFL);
            if (execvp(args[0], args) == -1) {
                perror("Erro");
            }
            //como o comando não existe o filho deve deixar de existir tambem
            exit(EXIT_FAILURE);
        
        //se pid não é menor nem igual a 0 este é o processo pai, apenas aguarda o filho
        } else {
            int status;
            if(redirecionar_background == 0){
                waitpid(pid, &status, 0);
            }
        }
    }

    return 0;
}