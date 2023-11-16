#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "utils/b-tree_functions.h"

FILE *abreArquivo(char *diretorio);

int main(){
    const int tamInsere = 26, tamBusca = 4;
    FILE *repositorio, *indice, *pontInsere, *pontBusca, *pontResult; // declara arquivos;
    REGISTRO vetInsere[tamInsere];
    BUSCA vetBusca[tamBusca];
    int i, opcao, promovido, repetida, aux;
    short root, promo_rrn;
    REGISTRO tempRegistro;
    INDEX tempIndice, promoKey;
    char concat[124];

    printf("\tO programa foi iniciado...\n\tProcurando arquivos...\n");

    pontInsere = abreArquivo("insere.bin");
    fread(vetInsere, sizeof(REGISTRO), tamInsere, pontInsere);
    fclose(pontInsere);

    pontBusca = abreArquivo("busca.bin");
    fread(vetBusca, sizeof(BUSCA), tamBusca, pontBusca);
    fclose(pontBusca);
    //for (i = 0; i < tamBusca; i++)
        //printf("vetBusca[%d]: %s%s\n", i, vetBusca[i].codCliente, vetBusca[i].codVeiculo);

    do{
        pontResult = abreArquivo("result.bin");
        fseek(pontResult, 0, SEEK_END);

        if (btopen())
            root = getroot();
        else
            root = create_tree();

        printf("\n-------------------------------------------------------------------------\n\n");
        printf("\tDigite:\n\t1 - INSERCAO\n\t2 - IMPRIMIR TODOS\n\t3 - PESQUISA\n\t0 - ENCERRAR\n");
        printf("\tOpcao:");
        scanf("%d", &opcao);
        printf("\n-------------------------------------------------------------------------\n\n");

        switch (opcao)
        {
            case 1:
                printf("Digite o número a ser inserido: ");
                scanf("%d", &aux);

                tempRegistro = vetInsere[aux - 1];
                sprintf(tempIndice.key, "%s%s", tempRegistro.codCliente, tempRegistro.codVeiculo);
                fseek(pontResult, 0, SEEK_END);
                tempIndice.offset = ftell(pontResult);
                repetida = 0;
                promovido = insere(root, tempIndice, &promo_rrn, &promoKey, &repetida);
                if (repetida)
                    break;

                if (promovido) {
                    root = create_root(promoKey, root, promo_rrn);
                    printf("Chave promovida: %s\n", promoKey.key);
                }

                fwrite(&tempRegistro, sizeof(REGISTRO), 1, pontResult);
                printf("Chave inserida com sucesso: %s\n", tempIndice.key);
                break;
            case 2:
                imprime(root, pontResult);
                break;
            case 3:
                printf("Digite o número a ser inserido: ");
                scanf("%d", &aux);
                BUSCA temp;
                temp = vetBusca[aux - 1];
                //imprimirBusca(temp);
                REGISTRO tempReg;
                
                tempReg = buscaBTree(temp, pontResult);
                imprimirRegistro(tempReg);
                //pesquisa(vetBusca, tamBusca, repositorio, indice);
                break;
            case 0:
                break;
        }

        fclose(pontResult);
        btclose();
    } while (opcao != 0);

    printf("\tEncerrando programa...\n");
    return 0;
}
/* FUNÇÕES */

/* Procura com a função "acces()" se o arquivo existe
        se sim ele é aberto com "r+b",
        senão é criado um arquivo com "w+b".
*/
FILE *abreArquivo(char *diretorio)
{
    FILE *pont;
    int tam = 0;

    if (access(diretorio, F_OK) == 0)
        pont = fopen(diretorio, "r+b");
    else
        pont = fopen(diretorio, "w+b");

    return pont;
}