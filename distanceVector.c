/*
    Aluno: Bruno Santos Costa

    Instruções para rodar o programa:
    Para compilar use: mpicc -o distanceVector distanceVector.c
    Para executar use: mpirun --oversubscribe -np 7 ./distanceVector
*/

// Include da bibliotecas que serão usadas no código
#include <mpi.h> // Biblioteca do MPI
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INF 9999 // INF representa infinito que ser ausado no grafo para distancias desconhecidas
#define N 7 // Número de nós no grafo

// Definição das funções criadas no programa
void initialize_graph(int graph[N][N]);
void print_graph(int graph[N][N]);

// Função principal do programa
int main(int argc, char** argv) {
    int rank, size; // rank serve para guardar o identificador de cada processo(ele começa em 0), size guarda a quantidade de processos que vão participara do programa
    int graph[N][N]; // Matriz para guardar a matriz de adjacência que representa o grafo
    int distances[N]; // esta variavel vai guardar em cada linha dele o vetor de distancias do nó correspondente a linha dele

    MPI_Init(&argc, &argv); // Inicia o ambiente MPI e faz configurações para o ambiente caso o usuário passe alguma pelos argumentos na linha de comando
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Essa função vai identificar o Id de cada processo dentro do comunicador do MPI, MPI_COMM_WORLD tem todos os processos que iniciaram dentro do programa, rank vai guardar o ID do processo que está sendo executado no momento
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Essa função vai retornar a quantidade total de processos que estão participando do comunicador MPI_COMM_WORLD

    // Verifica se a quantidade de processos corresponde a quantidade de nós do grafo que neste caso é 7
    if (size != N) {
        if (rank == 0) {
            printf("Este programa requer exatamente %d processos MPI.\n", N);
        }
        MPI_Finalize(); // Finaliza o ambiente MPI
        return -1;
    }

    // Esse bloco só é executado pelo processo de rank zero
    // Ele vai inicializar a matriz de adjacência com os numeros aleatórios
    if (rank == 0) {
        initialize_graph(graph); // Inicializa o grafo
        printf("Grafo inicializado:\n");
        print_graph(graph); // Imprime a grafo inicial
    }

    // Essa parte envia o grafo para todoa os processos que participam da execução do programa para todos terem uma copia do grafo
    // Os argumentos são, o grafo que vai ser enviado
    // A quantidade de elementos que serão enviados
    // O tipo de dados que será enviado que no caso são inteiros
    // Zero é o proceso mestre que vai enviar para os outro processos
    // MPI_COMM_WORLD é o comunicados que tem todos os processos que participam da execução do programa
    MPI_Bcast(graph, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Nessa linha cada processo vai colocar dentro da matriz de distancia seu vetor de distancia na linha da matriz referente a ele
    for (int i = 0; i < N; i++) {
        distances[i] = graph[rank][i];
    }

    // Laço de repetição que vai percorrer todos os N nós do grafo
    for (int k = 0; k < N - 1; k++) {
        int temp_distances[N]; // Vetor que vai guardar o vetor de disâncias que sera recebido de outro processo

        // Envia e recebe vetores de distâncias dos vizinhos
        for (int i = 0; i < N; i++) {
            // Aqui ele verifica se o nó é diferente do no que o processo está usando, caso seja diferente ele vai executar o codigo a abaixo
            if (graph[rank][i] != INF && rank != i) {
                // Os argumentos são os seguintes
                // O vetor que será enviado a outros processos
                // O numero de elementos que nesse caso é o tamanho do vetor
                // MPI_INT é o tipo de dados que está sendo enviado, no caso números inteiros
                // i se trata do rank do processo que está sendo enviado
                // 0 é o rotulo da mensagem que está sendo enviada, poderia ser qualquer número
                // MPI_COMM_WORLD é o comunicador que tem todos os processos que estão executando no programa
                MPI_Send(distances, N, MPI_INT, i, 0, MPI_COMM_WORLD); // Envia todo o vetor de distâncias

                // Os argumentos são
                // O vetor que vai receber o vetor que foi enviado para esse processo
                // N é o tamnho do vetor
                // MPI_INT é o tipo de dado que será recebido
                // i é o processoque enviou o vetor de distância
                // 0 é o rotulo da mensagem recebida
                // MPI_COMM_WORLD é o comunicador que tem todos os processos que estão sendo executados no programa
                // MPI_STATUS_IGNORE este parametro diz ao MPI que não é necessário verificar o status da mensagem recebida
                MPI_Recv(temp_distances, N, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Recebe todo o vetor de distâncias

                // Atualiza as distâncias com base nas mensagens recebidas dos vizinhos
                for (int j = 0; j < N; j++) {
                    // Verifica se existe uma ligação entre os nós e se a distancia calculada entre eles dis é menor que a distância ataual
                    if (temp_distances[j] != INF && distances[i] + temp_distances[j] < distances[j]) {
                        distances[j] = distances[i] + temp_distances[j]; // Atualiza a distância
                    }
                }
            }
        }
    }

    int final_distances[N][N]; // Matriz que vai guardar todos os vetores de distância para cada nó

    // Os parametros são
    // Variavel na qual os os dados foram enviados
    // Número de elementos que vão ser enviados
    // Tipo de dados que serão enviados
    // final_distances é a variavel na qual os vetores vão ser guardados
    // O número de elementos a serem recebidos
    // O tipo de dado a ser recebido
    // O rank do processo que vai reunir os dados que nesse caso é o rank 0
    // Comunicaddor com todos os processos
    MPI_Gather(distances, N, MPI_INT, final_distances, N, MPI_INT, 0, MPI_COMM_WORLD); // Vai pegar todos os resultados dos processos e enviar para o processo de rank 0

    // Se o processo executando for o de rank zero ele vai imprimir a matriz com as distâncias finais
    if (rank == 0) {
        printf("\nVetores de distância calculados:\n");
        print_graph(final_distances); // Chama a função de imprimir
    }

    MPI_Finalize(); // Finaliza o ambiente MPI
    return 0; // Retorno final da função main
} // Fim main

// Função para gerar um grafo aleatório
void initialize_graph(int graph[N][N]) {
    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        for (int j = i; j < N; j++) {
            if (i == j) {
                graph[i][j] = 0; // As distâncias para o próprio nó são sempre zero
            } else {
                int distance = (rand() % 10) + 1; // Distância aleatória entre 1 e 10

                if (rand() % 3 == 0) { // Configura algumas conexões como infinito
                    distance = INF;
                }

                graph[i][j] = distance; // Preenche a posição superior
                graph[j][i] = distance; // Garante simetria na posição inferior
            }
        }
    }
} // Fim initialize_graph

// Função para imprimir a matriz de adjacência do grafo
void print_graph(int graph[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (graph[i][j] == INF) {
                printf("INF\t");
            } else {
                printf("%d\t", graph[i][j]);
            }
        }
        printf("\n");
    }
} // Fim print_graph
