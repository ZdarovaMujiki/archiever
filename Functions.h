typedef struct Node
{
	unsigned long long amount;
	unsigned char data;
	struct Node *left, *right;
	char inTree;
} node;

typedef struct Cell
{
	int amount;
	char *code;
} cell;

typedef struct List
{
	unsigned char data;
	struct List *next;
} list;


int countControll(FILE *file);
int checkIntegrity(FILE *file);
int writeControll(FILE *file);

int zip(char* dir, FILE* out);
int unzip(FILE* in);
int info(FILE *arc);
int del(char *arcName, char *file);