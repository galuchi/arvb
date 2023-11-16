#include "bt.h"
#include <stdint.h>

/* STRUCTS */
typedef struct REGISTRO
{
    char codCliente[12];
    char codVeiculo[8];
    char nomeCliente[50];
    char nomeVeiculos[50];
    char dias[4];
} REGISTRO;

typedef struct
{
    char codCliente[12];
    char codVeiculo[8];
    //int offset;
} BUSCA;

/* PROTOTIPAÇÂO*/
void imprime(short rrn, FILE *pontResult);
void imprimeDadosPagina(BTPAGE *p_page, int * pos, FILE *pontResult);

/*Função: insere*/
int insere(short rrn, INDEX chave, short * promoRrnFilho, INDEX * promoChave, int * repetida){
    BTPAGE page, newPage; // page: página atual; newpage: página criada caso ocorra um split;
    int encontrado, promovido;
    short pos, rrnPromovido;
    INDEX chavePromovido;

    if (rrn == NIL){
        *promoChave = chave;
        *promoRrnFilho = NIL;
        return YES;
    }

    btread(rrn, &page);
    encontrado = search_node(chave, &page, &pos);
    if (encontrado){
        printf("Chave duplicada: %s \n", chave.key);
        *repetida = 1;
        return NO;
    }

    promovido = insere(page.child[pos], chave, &rrnPromovido, &chavePromovido, repetida);
    if (!promovido)
        return NO;

    if (page.keyCount < MAXKEYS){
        ins_in_page(chavePromovido, rrnPromovido, &page);
        btwrite(rrn, &page);
        return NO;
    }
    else{
        split(chavePromovido, rrnPromovido, &page, promoChave, promoRrnFilho, &newPage);
        printf("Divisão de nó\n");
        btwrite(rrn, &page);
        btwrite(*promoRrnFilho, &newPage);
        return YES;
    }
}

void imprimeDadosPagina(BTPAGE * page, int * pos, FILE * pontResult) {
    int i;

    // Adicione a impressão das páginas filhas
    for (i = 0; i < page->keyCount; i++) {
        imprime(page->child[i], pontResult);

        if (strcmp(page->keys[i].key, "@@@@@@@@@@@@@@@@@") != 0) {
            printf("Chave: %s\n", page->keys[i].key);

            // Ajuste para a leitura correta do registro no arquivo
            fseek(pontResult, page->keys[i].offset, SEEK_SET);
            REGISTRO tempRegistro;
            fread(&tempRegistro, sizeof(REGISTRO), 1, pontResult);

            printf("Cod Cliente:%s\nCod Veiculo:%s\nNome Cliente:%s\nNome Veiculo:%s\nDias:%s\n", tempRegistro.codCliente, tempRegistro.codVeiculo, tempRegistro.nomeCliente, tempRegistro.nomeVeiculos, tempRegistro.dias);
        }
        (*pos)++;
    }

    // Verifique a última página filha
    imprime(page->child[i], pontResult);
}

void imprime(short rrn, FILE * pontResult) {
    BTPAGE page;
    int pos = 0;

    if (rrn == NIL)
        return;

    btread(rrn, &page);
    imprimeDadosPagina(&page, &pos, pontResult);
}


REGISTRO buscaRecursivaBTree(BUSCA chave, short rrn, BTPAGE page, FILE *pontResult) {
    int i;

    if (rrn == -1){
        printf("Chave não encontrada: %s%s\n", chave.codCliente, chave.codVeiculo);
        // Retorna um registro inválido para indicar que a busca falhou
        REGISTRO registroInvalido;
        registroInvalido.codCliente[0] = '\0';
        return registroInvalido;  // Retorna um registro inválido para indicar que a busca falhou
    }
    btread(rrn, &page);
    char concat[19];
    sprintf(concat, "%s%s", chave.codCliente, chave.codVeiculo);
    for (i = 0; i < page.keyCount && strcmp(concat, page.keys[i].key) > 0; i++);

    int pos = i;

    if (pos < page.keyCount && strcmp(concat, page.keys[pos].key) == 0) {
        // Encontrou a chave na página
        fseek(pontResult, page.keys[pos].offset, SEEK_SET);
        REGISTRO resultado;
        fread(&resultado, sizeof(REGISTRO), 1, pontResult);
        printf("Chave encontrada: %s%s | pag: %d | pos: %d\n", chave.codCliente, chave.codVeiculo, rrn, pos);
        return resultado;
    } else {
        // A chave pode estar em uma subárvore
        return buscaRecursivaBTree(chave, page.child[pos], page, pontResult);
    }
}
REGISTRO buscaBTree(BUSCA chaveBusca, FILE *pontResult) {
    short root;
    BTPAGE page;

    if (btopen() == 0) {
        printf("Arquivo vazio\n");
        // Retorna um registro inválido para indicar que a busca falhou
        REGISTRO registroInvalido;
        registroInvalido.codCliente[0] = '\0';
        return registroInvalido;
    }

    root = getroot();
    return buscaRecursivaBTree(chaveBusca, root, page, pontResult);
}

void imprimirBusca(BUSCA busca) {
    printf("Código do Cliente: %s\n", busca.codCliente);
    printf("Código do Veículo: %s\n", busca.codVeiculo);
}

void imprimirRegistro(REGISTRO registro) {
    printf("Código do Cliente: %s\n", registro.codCliente);
    printf("Código do Veículo: %s\n", registro.codVeiculo);
    printf("Nome do Cliente: %s\n", registro.nomeCliente);
    printf("Nome do Veículo: %s\n", registro.nomeVeiculos);
    printf("Dias: %s\n", registro.dias);
    printf("\n");
}
