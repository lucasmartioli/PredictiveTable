//
// Created by Lucas on 11/26/2016.
//

#include <mem.h>
#include <ctype.h>
#include <malloc.h>
#include "table.h"

void addList(char *List, const char NT){

    int i = 0;

    for (; i < strlen(List); ++i) {
        if (List[i] == '\0'){
            break;
        }

        if (NT == List[i]){
            return;
        }

    }

    if (NT != CARACTER_VAZIO) {
        List[i] = NT;
        List[i + 1] = '\0';
    }

    return;

}

void addProducao(Producao* head, Producao* novaproducao) {

    while(head->next != NULL){
        head = head->next;
    }

    head->next = novaproducao;
    novaproducao->next = NULL;

    return;

}

int foundIndex(const char* List, const char c) {

    for (int i = 0; i < strlen(List); ++i) {
        if (List[i] == '\0'){
            break;
        }

        if (c == List[i]){
            return i;
        }

    }

    return -1;
}

Producao *makeNewProducao(char NT, char* producao) {

    Producao* novaProducao = (Producao*) malloc(sizeof(Producao));
    novaProducao->p = (char*) malloc(TAMANHO_MAXIMO_DA_PRODUCAO);
    novaProducao->p[0] = NT;
    novaProducao->p[1] = '-';
    novaProducao->p[2] = '\0';
    strcat(novaProducao->p, producao);
    novaProducao->next = NULL;

    return novaProducao;
}

int table(GrammarRule *gramaticalRules, FollowSet* followSet, FirstSet* firstset) {

    char* NTList = (char*) malloc(100);
    char* TList = (char*) malloc(100);

    NTList[0] = '\0';
    TList[0] = '$';
    TList[1] = '\0';

    GrammarRule *g = gramaticalRules;
    while (g != NULL){

        addList(NTList, g->key);
        for (int i = 0; i < strlen(g->rule); ++i) {
            if (isupper(g->rule[i])){
                addList(NTList, g->rule[i]);
            }else {
                addList(TList, g->rule[i]);
            }

        }

        g = g->next;
    }



    Producao* matrix[strlen(NTList)][strlen(TList)];


    for (int j = 0; j < strlen(NTList); ++j) {
        for (int i = 0; i < strlen(TList); ++i) {
            matrix[j][i] = NULL;

        }

    }

    g = gramaticalRules;
    while (g != NULL) {
        char *set = (char*) malloc(TAMANHO_MAXIMO_DA_REGRA);
        if (isupper(g->rule[0])) {
            FirstSet *f = seekkeyfirst(firstset, g->rule[0]);
            set = f->set;
        } else {
            set[0] = g->rule[0];
            set[1] = '\0';
        }


        for (int i = 0; i < strlen(set); ++i) {
            if (isupper(set[i])){
                continue;
            }

            if (set[i] == CARACTER_VAZIO) {
                FollowSet *ff = pesquisaConjuntoFollow(followSet, g->key);

                for (int j = 0; j < strlen(ff->set); ++j) {
                    if (isupper(ff->set[j])){
                        continue;
                    }

                    int NTIndex = foundIndex(NTList, g->key);
                    int TIndex = foundIndex(TList, ff->set[j]);

                    if (matrix[NTIndex][TIndex] == NULL){
                        matrix[NTIndex][TIndex] = makeNewProducao(g->key,g->rule);
                    } else {
                        addProducao(matrix[NTIndex][TIndex], makeNewProducao(g->key, g->rule));
                    }
                    
                }


                continue;
            }

            int NTIndex = foundIndex(NTList, g->key);
            int TIndex = foundIndex(TList, set[i]);

            if (matrix[NTIndex][TIndex] == NULL){
                matrix[NTIndex][TIndex] = makeNewProducao(g->key,g->rule);;
            } else {
                addProducao(matrix[NTIndex][TIndex], makeNewProducao(g->key, g->rule));
            }
        }

        g = g->next;
    }

    if (!writetable(NTList, TList, matrix))
        return 0;

    return 1;


}

int writetable(char* NTList, char* TList, Producao* t[strlen(NTList)][strlen(TList)])
{
    FILE *tableFile = fopen("table.csv", "w");

    if (tableFile == NULL)
    {
        printf("Problemas na criação do arquivo table\n");
        return 0;
    }

    for (int k = 0; k < strlen(TList); ++k) {
        char* c = (char*) malloc(2);
        c[0] = TList[k];
        c[1] = '\0';

        if(fwrite(",", 1, 1, tableFile) != 1 ||
           fwrite(c, 1, 1, tableFile) != 1)
        {
            printf("Erro na escrita do arquivo");
            return 0;
        }

    }

    if (fwrite("\n", 1, 1, tableFile) != 1)
    {
        printf("Erro na escrita do arquivo");
        return 0;
    }

    for (int i = 0; i < strlen(NTList); ++i) {
        char* c = (char*) malloc(2);
        c[0] = NTList[i];
        c[1] = '\0';

        if(fwrite(c, 1, 1, tableFile) != 1 ||
           fwrite(",", 1, 1, tableFile) != 1)
        {
            printf("Erro na escrita do arquivo");
            return 0;
        }

        for (int j = 0; j < strlen(TList); ++j) {
            Producao* head = t[i][j];

            if (head == NULL && (fwrite("ERRO", 1, 4, tableFile) != 4))
            {
                printf("Erro na escrita do arquivo");
                return 0;
            }

            while ( head != NULL)
            {

                if(fwrite(head->p, 1, strlen(head->p), tableFile) != strlen(head->p))
                {
                    printf("Erro na escrita do arquivo");
                    return 0;
                }

                head = head->next;

                if (head != NULL && (fwrite(" | ", 1, 3, tableFile) != 3))
                {
                    printf("Erro na escrita do arquivo");
                    return 0;
                }
            }

            if(fwrite(",", 1, 1, tableFile) != 1)
            {
                printf("Erro na escrita do arquivo");
                return 0;
            }

        }

        if(fwrite("\n", 1, 1, tableFile) != 1)
        {
            printf("Erro na escrita do arquivo");
            return 0;
        }


    }



    fclose(tableFile);
    return 1;

}



