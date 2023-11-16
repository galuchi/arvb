#define MAXKEYS 4
#define MINKEYS MAXKEYS / 2
#define NIL (-1)
#define NOKEY '@'
#define NO 0
#define YES 1

/* VARIÁVEIS GLOBAIS */
FILE * btfd; // ponteiro para "btree.dat"

/* STRUCTS */
typedef struct index{
    char key[18];
    int offset;
} INDEX;

typedef struct btPage{
    int keyCount;
    INDEX keys[MAXKEYS];
    int child[MAXKEYS + 1];
} BTPAGE;

#define PAGESIZE sizeof(BTPAGE)

/* PROTOTIPAÇÃO */
int btclose();
int btopen();
int btread(short rrn, BTPAGE *page_ptr);
int btwrite(short rrn, BTPAGE *page_ptr);
short create_root(INDEX key, short left, short right);
short create_tree();
short getpage();
short getroot();
void ins_in_page(INDEX key, short r_child, BTPAGE *p_page);
int pageinit(BTPAGE *p_page);
int putroot(short root);
int search_node(INDEX key, BTPAGE *p_page, short *pos);
int split(INDEX key, short r_child, BTPAGE *p_oldpage, INDEX *promo_key, short *promo_r_child, BTPAGE *p_newpage);

/* FUNÇÕES (Disponibilizadas no material de aula) */
/* Realiza a leitura de uma página */
int btread(short rrn, BTPAGE *page_ptr){
    long addr;
    addr = (long)rrn * (long)PAGESIZE + 2L;      // 2L (cabeçalho)
    fseek(btfd, addr, SEEK_SET);
    return (fread(page_ptr, PAGESIZE, 1, btfd)); // trocar por fread()
}

/* Realiza uma busca na página para verificar se a chave já existe */
int search_node(INDEX key, BTPAGE *p_page, short *pos){
    int i;
    for (i = 0; i < p_page->keyCount && strcmp(key.key, p_page->keys[i].key) > 0; i++)
        ;
    (*pos) = i;
    if ((*pos) < p_page->keyCount && strcmp(key.key, p_page->keys[(*pos)].key) == 0){
        return (YES);
    }
    else{
        return (NO);
    }
}

/* Realiza inserção ordenada na página */
void ins_in_page(INDEX key, short r_child, BTPAGE *p_page){
    int j;
    for (j = p_page->keyCount; strcmp(key.key, p_page->keys[j - 1].key) < 0 && j > 0; j--)
    {
        p_page->keys[j] = p_page->keys[j - 1];
        p_page->child[j + 1] = p_page->child[j];
    }
    p_page->keyCount++;
    p_page->keys[j] = key;
    p_page->child[j + 1] = r_child;
}

/* Escreve uma página */
int btwrite(short rrn, BTPAGE *page_ptr)
{
    long addr;
    addr = (long)rrn * (long)PAGESIZE + 2L;       // 2L (cabeçalho)!
    fseek(btfd, addr, 0);
    return (fwrite(page_ptr, PAGESIZE, 1, btfd));
}

/* Abre o arquivo da árvore-B */
int btopen()
{
    btfd = fopen("btree.bin", "rb+");
    return (btfd > 0);
}

/* Cria uma página raiz */
short create_root(INDEX key, short left, short right)
{
    BTPAGE page;
    short rrn;

    rrn = getpage();
    pageinit(&page);

    page.keys[0] = key;
    page.child[0] = left;
    page.child[1] = right;
    page.keyCount = 1;

    btwrite(rrn, &page);
    putroot(rrn);

    return (rrn);
}

/* Cria o árquivo da árvore-B e insere primeira chave (aparentemente vazia) na primeira página  */
short create_tree()
{
    short header = -1;
    INDEX raiz = {"@@@@@@@@@@@@@@@@@", -1};

    btfd = fopen("btree.bin", "wb+"); // trocar por fopen()

    fwrite(&header, sizeof(short), 1, btfd);
    fclose(btfd); // lembrar de escrever um header = -1 para a posição já existir e ser possível os deslocamentos em outros pontos
    btopen();

    return (create_root(raiz, NIL, NIL));
}

/* Lê o header do arquivo da árvore-B e recupera o RRN da página raiz */
short getroot()
{
    short root;
    fseek(btfd, 0L, SEEK_SET);
    // trocar por fseek()
    if (!fread(&root, sizeof(short), 1, btfd)) // trocar por fread()
    {
        printf("Error: Unable to get root. \007\n");
        exit(1);
    }
    return (root);
}

/* Busca o próximo RRN disponível */
short getpage(){
    long addr;
    short header;

    rewind(btfd);
    fread(&header, sizeof(short), 1, btfd);

    if (header == -1)
    {
        header = ftell(btfd);
        rewind(btfd);
        fwrite(&header, sizeof(short), 1, btfd);
        return ((short)ftell(btfd) / PAGESIZE);
    }
    fseek(btfd, 0L, SEEK_END); // 2L (cabeçalho)!; usar ftell na sequência para saber quantos

    addr = ftell(btfd) - 2L; // bytes andou no arquivo
    return ((short)addr / PAGESIZE);
}

/* Inicializa uma página */
int pageinit(BTPAGE *p_page)
{
    int j;
    INDEX initializer = {"@@@@@@@@@@@@@@@@@", -1};
    for (j = 0; j < MAXKEYS; j++)
    {
        p_page->keys[j] = initializer;
        p_page->child[j] = NIL;
    }
    p_page->child[MAXKEYS] = NIL;
}

/* Realiza a operação "split" de uma página */
int split(INDEX key, short r_child, BTPAGE *p_oldpage, INDEX *promo_key, short *promo_r_child, BTPAGE *p_newpage)
{
    int j;
    short mid;
    INDEX workkeys[MAXKEYS + 1], ind = {NOKEY, 0};
    short workchil[MAXKEYS + 2];
    for (j = 0; j < MAXKEYS; j++)
    {
        workkeys[j] = p_oldpage->keys[j];
        workchil[j] = p_oldpage->child[j];
    }
    workchil[j] = p_oldpage->child[j];
    for (j = MAXKEYS; strcmp(key.key, workkeys[j - 1].key) < 0 && j > 0; j--)
    {
        workkeys[j] = workkeys[j - 1];
        workchil[j + 1] = workchil[j];
    }

    workkeys[j] = key;
    workchil[j + 1] = r_child;

    *promo_r_child = getpage();
    pageinit(p_newpage);

    for (j = 0; j < MINKEYS; j++)
    {
        p_oldpage->keys[j] = workkeys[j];
        p_oldpage->child[j] = workchil[j];
        p_newpage->keys[j] = workkeys[j + 1 + MINKEYS];
        p_newpage->child[j] = workchil[j + 1 + MINKEYS];
        p_oldpage->keys[j + MINKEYS] = ind;
        p_oldpage->child[j + 1 + MINKEYS] = NIL;
    }
    p_oldpage->child[MINKEYS] = workchil[MINKEYS];

    p_newpage->child[MINKEYS] = workchil[j + 1 + MINKEYS];
    p_newpage->keyCount = MAXKEYS - MINKEYS;
    p_oldpage->keyCount = MINKEYS;
    (*promo_key) = workkeys[MINKEYS];
    /* strcpy(promo_key->chave, workkeys[MINKEYS].chave);
    promo_key->offset = workkeys[MINKEYS].offset */
    ;
}


/*  */
int putroot(short root)
{
    fseek(btfd, 0L, 0);                    // trocar por fseek()
    fwrite(&root, sizeof(short), 1, btfd); // trocar por fwrite()
}


/* Fecha o arquivo da árvore-B */
int btclose()
{
    fclose(btfd);
}