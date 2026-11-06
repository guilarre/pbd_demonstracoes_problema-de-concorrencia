# Visão geral

- Criamos dois experimentos em C:

    1. Demonstração de banco de dados sem controle de concorrência de um SGBD (usa um arquivo .txt simples como banco) → simula problema de LOST UPDATE

    2. Demonstração usando o PostgreSQL (banco de dados) → usa READ COMMITTED pra controlar concorrência

- Nos dois experimentos, temos uma conta contendo inicialmente **100 reais**.

- Disparamos duas threads simultaneamente:

    - Thread 1: valor_total = valor_total - 50 **(saque de 50 reais)**

    - Thread 2: valor_total = valor_total + 30 **(depósito de 30 reais)**

> Valor final esperado na conta: **80 reais**

# Resultados

## Demonstração em C puro (sem controle de concorrência):

> Valor final: indeterminado (no nosso experimento, variou entre 50, 500, 130, entre outros)

## Demonstração usando PostgreSQL (usando READ COMMITTED):

> Valor final: sempre 80 reais

# Passo a passo para reproduzir o experimento

Para o experimento, utilizou-se o arch linux, então seguem os passos para esse SO:

## Setup

1. Clonar repositório e entrar nele:
```bash
git clone https://github.com/guilarre/pbd_demonstracoes_problema-de-concorrencia.git
cd pbd_demonstracoes_problema-de-concorrencia
```
2. Instalar o postgresql: `sudo pacman -S postgresql`
3. Inicializar pasta do postgres: `sudo -u postgres initdb -D /var/lib/postgres/data`
4. Habilitar o serviço do postgres no systemd: `sudo systemctl enable postgresql --now`
5. Entrar na CLI do postgres: `sudo -u postgres psql`
6. Alterar senha do usuário postgres para ficar igual ao do nosso código:
```sql
ALTER USER postgres WITH PASSWORD 'MinhaSuperSenha123';
```
   OU alterar senha no código para a sua própria senha.
7. Criar banco de dados, conectar, criar tabela 'conta' e inserir 100 reais:
```sql
CREATE DATABASE banco_teste;
\c banco_teste
CREATE TABLE conta (id INT PRIMARY KEY, saldo INT);
INSERT INTO conta VALUES (1, 100);
```
8. Conectar à tabela e printar valor atual:
```sql
SELECT * FROM conta;
```

## Experimentos

1. Compilar exemplos:
```bash
gcc exemplo_em_c.c -o exemplo_em_c
gcc exemplo_postgres.c -o exemplo_postgres -lpq -lpthread
```
2. Rodar o exemplo inconsistente (sem SGBD):
```bash
./exemplo_em_c
```
2. Rodar o exemplo com controle de concorrência e verificar valor no postgres:
```bash
./exemplo_postgres
sudo pacman -S postgresql
```
```sql
\c banco_teste
SELECT * FROM conta;
```
