#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// função que simula uma transação simples sem controle de concorrência no nosso "banco de dados" (arquivo)
void* transacao_inconsistente(void* arg) {
    int valor_transacao = *(int*)arg;

    // lê saldo atual do arquivo
    FILE *f = fopen("banco.txt", "r");
    if (f == NULL) {
        perror("Erro ao abrir arquivo para leitura");
        pthread_exit(NULL);
    }
    int saldo;
    fscanf(f, "%d", &saldo);
    fclose(f);

    printf("[C - Concorrente] Leu o saldo: %d\n", saldo);

    // simula atraso de processamento da aplicação (tempo ideal para ocorrer problemas de concorrência)
    usleep(100000); 

    // calcula o novo saldo
    saldo += valor_transacao;

    // grava o valor de volta (sobrescrevendo cegamente)
    f = fopen("banco.txt", "w");
    if (f == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        pthread_exit(NULL);
    }
    fprintf(f, "%d", saldo);
    fclose(f);

    printf("[C - Concorrente] Escreveu o novo saldo: %d\n", saldo);
    return NULL;
}

int main() {
    // inicializa o arquivo com saldo 100
    FILE *f = fopen("banco.txt", "w");
    if (f == NULL) {
        perror("Erro ao inicializar banco.txt");
        return 1;
    }
    fprintf(f, "100");
    fclose(f);

    pthread_t t1, t2;
    int saque = -50;
    int deposito = 30;

    // as duas threads vão ser disparadas ao mesmo tempo
    pthread_create(&t1, NULL, transacao_inconsistente, &saque);
    pthread_create(&t2, NULL, transacao_inconsistente, &deposito);
    // join pra main esperar as threads simultâneas terminarem antes de continuar
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // lê o saldo final após threads terminarem
    f = fopen("banco.txt", "r");
    if (f == NULL) {
        perror("Erro ao ler saldo final");
        return 1;
    }
    int saldo_final;
    fscanf(f, "%d", &saldo_final);
    fclose(f);

    printf("\n--- RESULTADO EM C (SEM ACID) ---\n");
    printf("Saldo Final Esperado: 80\n");
    printf("Saldo Final Obtido: %d (INCONSISTENTE!)\n", saldo_final);

    return 0;
}
