#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libpq-fe.h> //biblioteca postgres em C
#include <unistd.h>

// struct para passar dados para as threads
typedef struct {
    int id_transacao;
    int valor_operacao;
} ArgsTransacao;

void* executar_transacao_postgres(void* arg) {
    ArgsTransacao *args = (ArgsTransacao*) arg;

    PGconn *conn = PQconnectdb("dbname=banco_teste user=postgres password=MinhaSuperSenha123 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "[Thread %d] Erro de conexao: %s\n", args->id_transacao, PQerrorMessage(conn));
        PQfinish(conn);
        pthread_exit(NULL);
    }

    // inicia a transação e define o nível de isolamento como READ COMMITTED (padrão do postgres),
    // que garante que um commit seja realizado após o outro
    PGresult *res = PQexec(conn, "BEGIN TRANSACTION ISOLATION LEVEL READ COMMITTED;");
    PQclear(res);

    // NOTE: se usássemos SERIALIZABLE, o postgres tem uma postura defensiva, rejeitando a transação
    // que causa inconsistência completamente. já com READ COMMITTED, uma transação aguarda a outra committar,
    // pq a linha inteira fica travada até que a transação da primeira thread acabe (melhor pra nossa demonstração)

    // lendo saldo atual
    res = PQexec(conn, "SELECT saldo FROM conta WHERE id = 1;");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "[Thread %d] Erro na leitura\n", args->id_transacao);
        PQclear(res);
        PQfinish(conn);
        pthread_exit(NULL);
    }
    int saldo = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    printf("[Thread %d] Leu o saldo: %d. Aplicando operacao: %d\n",
           args->id_transacao, saldo, args->valor_operacao);

    // simula atraso de processamento da aplicação (tempo ideal para ocorrer problemas de concorrência)
    usleep(50000); 

    // prepara o update
    char query[100];
    sprintf(query, "UPDATE conta SET saldo = saldo + (%d) WHERE id = 1;", args->valor_operacao); //o proprio postgres faz o cálculo

    res = PQexec(conn, query);

    // tenta o COMMIT
    PGresult *res_commit = PQexec(conn, "COMMIT;");

    // saindo
    PQclear(res);
    PQclear(res_commit);
    PQfinish(conn);
    pthread_exit(NULL);
}

int main() {
    pthread_t t1, t2;
    ArgsTransacao arg1 = {1, -50}; //saque
    ArgsTransacao arg2 = {2, 30}; //depósito

    // dispara as duas threads ao mesmo tempo
    pthread_create(&t1, NULL, executar_transacao_postgres, &arg1);
    pthread_create(&t2, NULL, executar_transacao_postgres, &arg2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\nFim do teste de concorrência com o PostgreSQL.\n");
    return 0;
}
