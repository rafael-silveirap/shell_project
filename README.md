<h1 align="center">⚙️ Custom C Shell</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c" alt="C">
  <img src="https://img.shields.io/badge/OS-Linux%20%2F%20POSIX-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux">
  <img src="https://img.shields.io/badge/Concepts-OS%20Theory%20%7C%20IPC-blueviolet?style=for-the-badge" alt="OS Concepts">
</p>

<p align="center">
  Um interpretador de linha de comando (Shell) UNIX-like construído do zero em C. Desenvolvido para explorar os fundamentos de Sistemas Operacionais, abstração de hardware e a API POSIX.
</p>

---

## Visão Geral

Este projeto é uma implementação funcional de um shell de terminal que interage diretamente com o Kernel do Linux. Ele não utiliza bibliotecas de alto nível para o gerenciamento de tarefas; em vez disso, lida de forma "bare-metal" com a criação de processos, roteamento de memória e manipulação de interrupções de hardware.


## 🛠️ Arquitetura e "Under the Hood"

O grande diferencial deste projeto é a aplicação prática de conceitos clássicos de Sistemas Operacionais. A arquitetura foi construída sobre quatro pilares principais da API POSIX:

### 1. Process API (Gerenciamento de Processos)
* **Execução:** Utilização do padrão `fork()` seguido de `execvp()` para clonar o shell e substituir a imagem de memória do processo filho pelo binário desejado.
* **Tarefas Assíncronas (`&`):** Implementação de concorrência. O shell (processo pai) não bloqueia aguardando o filho.
* **Reaping de Zumbis:** Uso de `waitpid(..., WNOHANG)` para varrer e colher (reap) processos filhos que terminaram em background, evitando vazamento de recursos na tabela de processos do Kernel.

### 2. File Descriptors e Redirecionamento de I/O
* Manipulação direta da tabela de File Descriptors (FDs) do processo.
* Uso da syscall `dup2()` para fechar o `STDOUT (1)` ou `STDIN (0)` padrão e redirecioná-los para FDs de arquivos abertos com a syscall `open()`, permitindo o redirecionamento com `<` e `>`.

### 3. Comunicação Interprocessos (IPC) com Pipes
* Construção de barramentos de memória unidirecionais usando a syscall `pipe()`.
* Orquestração complexa de múltiplos `fork()`, onde as portas de Leitura e Escrita do pipe são meticulosamente roteadas via `dup2()` entre processos produtores e consumidores para suportar comandos como `ls -l | grep main`.

### 4. Tratamento de Sinais (Hardware Interrupts)
* Blindagem do processo pai contra o sinal `SIGINT` (Ctrl+C) usando `signal(SIGINT, handler)`.
* Restauração do comportamento padrão nos processos filhos (`SIG_DFL`) para garantir que apenas o comando em execução seja abortado, mantendo a integridade do prompt.

## Funcionalidades

* [x] Execução de binários do sistema (`ls`, `cat`, `grep`, etc.)
* [x] Redirecionamento de saída de arquivos (`>`)
* [x] Redirecionamento de entrada de arquivos (`<`)
* [x] Execução de processos em segundo plano / background (`&`)
* [x] Comunicação via Pipe (`|`)
* [x] Blindagem contra interrupções de teclado (`Ctrl+C`)
* [x] Histórico de comandos e navegação direcional (via GNU Readline)

## Como Compilar e Rodar

### Pré-requisitos
Certifique-se de estar em um ambiente Linux (ou WSL) com o compilador `gcc` e a ferramenta `make` instalados, além da biblioteca GNU Readline.

```bash
# No Ubuntu/Debian:
sudo apt update
sudo apt install build-essential libreadline-dev
```
Instalação

Clone o repositório:

```bash
git clone [https://github.com/rafael-silveirap/shell_project.git](https://github.com/rafael-silveirap/shell_project.git)
```
Compile usando o Makefile fornecido:

```bash
make
```
Execute o shell:

```bash
make run
```
