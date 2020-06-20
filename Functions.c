#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>
#include <windows.h>
#include <winuser.h>
#include <time.h>
#include "Functions.h"

cell *table;

long long fileSize;

unsigned char treeByteAmount;
int treeElementsCount;
unsigned char treeElements[256];
unsigned char *arr = NULL;
unsigned char bit8 = 0;
unsigned char rank = 1 << 7;

int countControll(FILE *file)
{
	rewind(file);
	unsigned int sum = 0, c0 = 0, c1 = 0, bufferSize;
	unsigned char *inBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);
	do
	{
		bufferSize = fread(inBuffer, sizeof(unsigned char), (int)1e4, file); //количество считанных символов
		if (bufferSize == 1e4)
		{
			sum = fletcher16(inBuffer, bufferSize, &c0, &c1);
		}
		else
		if (bufferSize > 4)
		{
			sum = fletcher16(inBuffer, bufferSize - 4, &c0, &c1);
		}
	} while (bufferSize != 0);
	return sum;
}

int writeControll(FILE *file)
{
	printf("counting controll sum started\n");
	int sum = countControll(file);
	fseek(file, -4, SEEK_END);
	fwrite(&sum, sizeof(int), 1, file);
	printf("counting controll sum ended: %u\n", sum);
	return sum;
}

int checkIntegrity(FILE *file)
{
	int sum1 = countControll(file);
	int sum2;
	fseek(file, -4, SEEK_END);
	fread(&sum2, 4, 1, file);
	return (sum1 == sum2);
}

int fletcher16(const unsigned char *data, int len, int *c0, int *c1)
{
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		*c0 = *c0 + *data++;
		*c1 = *c1 + *c0;
	}
	*c0 = *c0 % 255;
	*c1 = *c1 % 255;
	return (*c1 << 8 | *c0);
}

void fwriteCompact(unsigned long long value, FILE *file)
{
	unsigned long long tmp = value;
	unsigned char n = 0;
	while (value > 0)
	{
		value >>= 8;
		n++;
	}
	fwrite(&n, 1, 1, file);
	fwrite(&tmp, n, 1, file);
}

unsigned long long freadCompact(FILE *file)
{
	unsigned long long n = 0;
	unsigned char size = 0;
	fread(&size, 1, 1, file);
	if (size)
	{
		fread(&n, size, 1, file);
	}
	return n;
}

void AddNode(list **first, list **last, unsigned char value)
{
	list *add = malloc(sizeof(list));
	add->data = value;

	if (!*first)
	{
		*first = add;
		*last = add;
		add->next = NULL;
	}
	else
	{
		(*last)->next = add;
		*last = add;
		add->next = NULL;
	}
}

void BuildTable(node *root, char *currentCode, int size)
{
	int i;
	if (root->left)
	{
		currentCode[size] = 0;
		BuildTable(root->left, currentCode, size + 1);
	}
	if (root->right)
	{
		currentCode[size] = 1;
		BuildTable(root->right, currentCode, size + 1);
	}
	if (!root->right && !root->left)
	{
		table[root->data].code = (char*)calloc(size, sizeof(char));
		for (i = 0; i < size; i++)
		{
			table[root->data].code[i] = currentCode[i];
		}
		fileSize += i * table[root->data].amount;
	}
}

void getTreePref(node *t, list **first, list **last)
{
	if (!rank)
	{
		treeByteAmount++;
		AddNode(first, last, bit8);
		bit8 = 0;
		rank = 1 << 7;
	}
	if (!t) return;
	if (!t->left && !t->right)
	{
		bit8 += rank;
		rank >>= 1;
		treeElements[treeElementsCount] = t->data;
		treeElementsCount++;
	}
	else
	{
		rank >>= 1;
	}
	getTreePref(t->left, first, last);
	getTreePref(t->right, first, last);
}

node* buildTree(node *root, FILE *in)
{
	node *p = (node*)calloc(1, sizeof(node));
	if (!rank)
	{
		*arr++;
		rank = 1 << 7;
	}
	if (arr[0] & rank)
	{
		fileSize++;
		fread(&p->data, sizeof(char), 1, in);
		p->left = NULL;
		p->right = NULL;
		rank >>= 1;
	}
	else
	{
		rank >>= 1;
		p->left = buildTree(p->left, in);
		p->right = buildTree(p->right, in);
	}
	return p;
}

void printTree(node * p, int level)
{
	if (p)
	{
		printTree(p->left, level + 1);
		for (int i = 0; i < level; i++) printf("   ");
		printf("%llu", p->amount);
		if (p->data)
		{
			printf("(%c(%d))", p->data, p->data);
		}
		printf("\n");
		printTree(p->right, level + 1);
	}
}

void freeTree(node *tree)
{
	if (tree->right)
	{
		freeTree(tree->right);
	}
	if (tree->left)
	{
		freeTree(tree->left);
	}
	if (!tree->right && !tree->left)
	{
		free(tree);
	}
}

unsigned long long getFileSize(FILE* f)
{
	fseek(f, 0, SEEK_END);
	unsigned long long pos = _ftelli64(f);
	rewind(f);
	return pos;
}

node* build_tree(node* mas, int symb_sum)
{
	node *min1, *min2;

	if (mas[0].amount < mas[1].amount)
	{
		min1 = &mas[0];
		min2 = &mas[1];
	}
	else
	{
		min1 = &mas[1];
		min2 = &mas[0];
	}

	for (int i = 2; i < symb_sum; ++i)
	{
		if (mas[i].amount < min1->amount && mas[i].amount < min2->amount)
		{
			min2 = min1;
			min1 = &mas[i];
		}
		else if (mas[i].amount < min2->amount)
		{
			min2 = &mas[i];
		}
	}

	if (min2->amount == 1e10)
	{
		return min1;
	}

	mas[symb_sum].right = min2;
	mas[symb_sum].left = min1;
	mas[symb_sum].amount = min1->amount + min2->amount;
	symb_sum++;

	min1->amount = 1e10;
	min2->amount = 1e10;

	return build_tree(mas, symb_sum);
}

int zip(char *directory, FILE *out)
{
	_fseeki64(out, -4, SEEK_END);
	unsigned int start_time = clock(); // начальное время
	FILE *in = fopen(directory, "rb");
	if (!in)
	{
		printf("Unable to find file \"%s\"\n", directory);
		return 0;
	}
	unsigned long long originFileSize = getFileSize(in);
	if (!originFileSize)
	{
		printf("file \"%s\" is empty\n", directory);
		return 0;
	}
	char *filename = (char*)malloc(sizeof(char)* 256);
	node* nodes = (node*)calloc(513, sizeof(node));
	int treeSize = 0, nameBitAmount = 0, bufferSize = 0;
	long long codeBitAmount = 0;
	fileSize = 0;

	table = (cell*)calloc(256, sizeof(cell));

	treeElementsCount = 0;
	treeByteAmount = 0;

	//получить имя файла:
	if (strrchr(directory, '\\'))
	{
		filename = strrchr(directory, '\\');
		filename++;
	}
	printf("zipping %s\n", filename);

	//подсчет символов
	unsigned char *inBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);
	do
	{
		bufferSize = fread(inBuffer, sizeof(unsigned char), (int)1e4, in); //количество считанных символов
		for (int i = 0; i < bufferSize; i++) //подсчёт количества символов 
		{
			nodes[inBuffer[i]].data = inBuffer[i];
			nodes[inBuffer[i]].amount++;
			table[inBuffer[i]].amount++;
		}
	} while (bufferSize != 0);
	for (int i = 0; filename[i] != '\0'; i++)
	{
		nodes[filename[i]].data = filename[i];
		nodes[filename[i]].amount++;
	}
	for (int i = 0; i < 256; i++)
	{
		if (nodes[i].amount == 0) //все символы, которые не встретились
		{
			nodes[i].amount = 1e10;
			continue;
		}
	}

	node* root = build_tree(&nodes[0], 256); //построение дерева

	//построение таблицы значений
	char *code = malloc(16 * sizeof(char));
	BuildTable(root, code, 0);
	for (int i = 0; filename[i] != '\0'; i++)
	{
		for (int j = 0; table[filename[i]].code[j] == 1 || table[filename[i]].code[j] == 0; j++)
		{
			nameBitAmount++;
		}
	}

	//получение битового дерева Хаффмана
	list *first = NULL, *last = NULL;
	bit8 = 0;
	rank = 1 << 7;
	getTreePref(root, &first, &last);
	AddNode(&first, &last, bit8);
	treeByteAmount++;

	fwriteCompact(originFileSize, out); //запись исходного размера файла

	fwrite(&treeByteAmount, sizeof(char), 1, out);	//запись количества бит закодированного дерева
	//запись закодированного дерева
	list *t = first;
	while (t)
	{
		fwrite(&t->data, sizeof(char), 1, out);
		t = t->next;
	}
	fwrite(treeElements, treeElementsCount, sizeof(char), out);	//запись элементов дерева


	//запись имени файла
	fwriteCompact(nameBitAmount, out);
	bit8 = 0;
	rank = 1 << 7;
	for (int i = 0; filename[i] != '\0'; i++)
	{
		for (int j = 0; table[filename[i]].code[j] == 1 || table[filename[i]].code[j] == 0; j++)
		{
			bit8 += table[filename[i]].code[j] * rank;
			rank >>= 1;

			if (!rank)
			{
				fwrite(&bit8, sizeof(char), 1, out);
				bit8 = 0;
				rank = 1 << 7;
			}
		}
	}
	fwrite(&bit8, sizeof(char), 1, out);

	//запись зашифрованного сообщения
	fwriteCompact(fileSize, out);
	fseek(in, 0, 0);
	bit8 = 0;
	rank = 1 << 7;

	unsigned char *outBuffer = malloc((int)1e4 * sizeof(unsigned char));
	unsigned int pos = 0;
	do
	{
		bufferSize = fread(inBuffer, sizeof(unsigned char), (int)1e4, in); //количество считанных символов
		for (int i = 0; i < bufferSize; i++)
		{
			for (int j = 0; table[inBuffer[i]].code[j] == 1 || table[inBuffer[i]].code[j] == 0; j++)
			{
				bit8 += table[inBuffer[i]].code[j] * rank;
				rank >>= 1;
				if (!rank)
				{
					outBuffer[pos] = bit8;
					pos++;
					bit8 = 0;
					rank = 1 << 7;
				}
				if (pos == 1e4 - 1)
				{
					fwrite(outBuffer, sizeof(unsigned char), 1e4 - 1, out);
					pos = 0;
				}
			}
		}
	} while (bufferSize != 0);
	if (rank != 1 << 7)
	{
		outBuffer[pos] = bit8;
		pos++;
	}
	fwrite(outBuffer, sizeof(unsigned char), pos, out);

	free(nodes);
	free(table);

	char zeros[4] = { 0, 0, 0, 0 };
	fwrite(zeros, sizeof(char), 4, out);

	unsigned int end_time = clock(); // конечное время
	printf("%s zipped in %ums\n", filename, end_time - start_time);
}

int unzip(FILE *in, char *file)
{
	fseek(in, 0, 0);
	unsigned long long size = getFileSize(in);

	printf("unzipping %s started\n", file);
	unsigned int start_time = clock(); // начальное время
	long long codeBitAmount = 0, codeSize = 0;
	unsigned long long originFileSize;
	int nameBitAmount, pos = 0;
	unsigned char c;

	node *t = NULL, *root = NULL;
	FILE *out = NULL;

	unsigned char *inBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);
	unsigned char *outBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);


	for (int i = 1; _ftelli64(in) < size - 5; i++)
	{
		originFileSize = freadCompact(in);
		if (feof(in))
		{
			break;
		}

		char *filename = (char*)malloc(256 * sizeof(char));
		fread(&treeByteAmount, sizeof(char), 1, in);
		arr = (unsigned char*)calloc(treeByteAmount, sizeof(unsigned char));
		fread(arr, sizeof(unsigned char), treeByteAmount, in);

		//построение дерева
		char *back = arr;
		root = (node*)calloc(1, sizeof(node));
		rank = 1 << 7;
		root = buildTree(root, in, arr);

		//расшифровка имени
		nameBitAmount = freadCompact(in);
		t = root;
		int i = 0;
		while (codeSize <= nameBitAmount)
		{
			fread(&c, sizeof(char), 1, in);
			unsigned char mask = 128;
			while (mask > 0)
			{
				if (!t->left && !t->right)
				{
					filename[i] = t->data;
					i++;
					t = root;
				}
				if (mask & c)
				{
					t = t->right;
					codeSize++;
				}
				else
				{
					t = t->left;
					codeSize++;
				}
				mask >>= 1;
				if (codeSize > nameBitAmount)
				{
					break;
				}
			}
		}
		arr = back;
		filename[i] = 0;
		codeBitAmount = freadCompact(in);
		if (!strcmp(file, filename))
		{
			out = fopen(filename, "wb");
			break;
		}
		fseek(in, (codeBitAmount + 7) / 8, SEEK_CUR);
		codeSize = 0;
		free(filename);
		free(arr);
		freeTree(root);
	}
	if (!out)
	{
		printf("File \"%s\" can't be found, try -l to see files in archieve\n", file);
	}
	else
	{
		//расшифровка сообщения
		codeSize = 0;
		t = root;

		int bufferSize = 0;
		do
		{
			bufferSize = fread(inBuffer, sizeof(unsigned char), (int)1e4, in); //количество считанных символов
			for (int i = 0; i < bufferSize && codeSize <= codeBitAmount; i++)
			{
				unsigned char mask = 128;
				while (mask > 0)
				{
					if (!t->left && !t->right)
					{
						outBuffer[pos] = t->data;
						pos++;
						if (pos == 1e4 - 1)
						{
							fwrite(outBuffer, sizeof(unsigned char), 1e4 - 1, out);
							pos = 0;
						}
						t = root;
					}
					if (mask & inBuffer[i])
					{
						t = t->right;
						codeSize++;
					}
					else
					{
						t = t->left;
						codeSize++;
					}
					mask >>= 1;
					if (codeSize > codeBitAmount)
					{
						break;
					}
				}
			}
		} while (bufferSize != 0);
		fwrite(outBuffer, sizeof(unsigned char), pos, out);
		fclose(out);

		unsigned int end_time = clock(); // конечное время
		printf("%s unzipped in %ums\n", file, end_time - start_time);
	}
}

int info(FILE *arc)
{
	unsigned char c;

	unsigned long long size = getFileSize(arc);

	for (int i = 1; _ftelli64(arc) < size - 5; i++)
	{

		unsigned long long startPosition = ftell(arc);
		unsigned long long originFileSize = freadCompact(arc);
		if (feof(arc))
		{
			break;
		}
		long long codeSize = 0;
		int nameBitAmount = 0;
		long long codeBitAmount = 0;
		printf("%d: ", i);
		fread(&treeByteAmount, sizeof(char), 1, arc);

		arr = (unsigned char*)calloc(treeByteAmount, sizeof(unsigned char));
		fread(arr, sizeof(unsigned char), treeByteAmount, arc);

		//построение дерева
		char *back = arr;
		node* root = (node*)calloc(1, sizeof(node)), *t;
		rank = 1 << 7;
		root = buildTree(root, arc);

		nameBitAmount = freadCompact(arc);

		//вывод названия файла
		t = root;
		while (codeSize <= nameBitAmount)
		{
			fread(&c, sizeof(char), 1, arc);
			unsigned char mask = 128;
			while (mask > 0)
			{
				if (!t->left && !t->right)
				{
					printf("%c", t->data);
					t = root;
				}
				if (mask & c)
				{
					t = t->right;
					codeSize++;
				}
				else
				{
					t = t->left;
					codeSize++;
				}
				mask >>= 1;
				if (codeSize > nameBitAmount)
				{
					break;
				}
			}
		}
		arr = back;

		codeBitAmount = freadCompact(arc);
		_fseeki64(arc, (codeBitAmount + 7) / 8, SEEK_CUR);

		unsigned long long endPosition = _ftelli64(arc);
		printf(". Size of archieve: %llu bytes, size of file: %lld bytes (%.2lf%%)\n", endPosition - startPosition, originFileSize, (double)(endPosition - startPosition) / originFileSize * 100);
		free(arr);
		freeTree(root);
		free(root);
	}
}

int del(char *arcName, char *file)
{
	FILE *arc = fopen(arcName, "r+b");
	if (strrchr(arcName, '\\'))
	{
		arcName = strrchr(arcName, '\\');
		arcName++;
	}
	if (!arc)
	{
		printf("Archieve \"%s\" can't be found\n", arcName);
		exit(0);
	}

	unsigned char *inBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);
	unsigned char *outBuffer = (unsigned char*)malloc(sizeof(unsigned char)* (int)1e4);
	unsigned char c;

	unsigned long long size = getFileSize(arc);

	for (int i = 1; _ftelli64(arc) < size - 5; i++)
	{
		int k = 0;
		long long startPosition = _ftelli64(arc);

		unsigned long long originFileSize = freadCompact(arc);
		if (feof(arc))
		{
			break;
		}

		char *filename = (char*)malloc(256 * sizeof(char));
		long long codeSize = 0;
		int nameBitAmount = 0;
		long long codeBitAmount = 0;
		fread(&treeByteAmount, sizeof(char), 1, arc); //+ 1

		arr = (unsigned char*)calloc(treeByteAmount, sizeof(unsigned char));
		fread(arr, sizeof(unsigned char), treeByteAmount, arc);

		//построение дерева
		char *back = arr;
		node* root = (node*)calloc(1, sizeof(node)), *t;
		rank = 1 << 7;
		root = buildTree(root, arc);

		nameBitAmount = freadCompact(arc);

		//вывод названия файла
		t = root;
		while (codeSize <= nameBitAmount)
		{
			fread(&c, sizeof(char), 1, arc);
			unsigned char mask = 128;
			while (mask > 0)
			{
				if (!t->left && !t->right)
				{
					filename[k] = t->data;
					k++;
					t = root;
				}
				if (mask & c)
				{
					t = t->right;
					codeSize++;
				}
				else
				{
					t = t->left;
					codeSize++;
				}
				mask >>= 1;
				if (codeSize > nameBitAmount)
				{
					break;
				}
			}
		}
		arr = back;
		filename[k] = 0;

		codeBitAmount = freadCompact(arc);
		fseek(arc, (codeBitAmount + 7) / 8, SEEK_CUR);

		long long endPosition = _ftelli64(arc);

		if (!strcmp(file, filename))
		{
			fseek(arc, 0, 0);
			FILE *tmp = fopen("temporaryArchieve.tmp", "wb");
			int bufferSize;

			bufferSize = fread(inBuffer, sizeof(unsigned char), 1e4, arc);
			while (bufferSize <= startPosition - bufferSize)
			{
				fwrite(inBuffer, sizeof(unsigned char), 1e4, tmp);
				bufferSize = fread(inBuffer, sizeof(unsigned char), 1e4, arc);
				startPosition -= bufferSize;
			}
			fwrite(inBuffer, sizeof(unsigned char), startPosition, tmp);

			fseek(arc, endPosition, 0);

			do
			{
				bufferSize = fread(inBuffer, sizeof(unsigned char), (int)1e4, arc); //количество считанных символов
				fwrite(inBuffer, sizeof(unsigned char), bufferSize, tmp);
			} while (bufferSize != 0);


			fclose(arc);
			remove(arcName);
			{
				fclose(tmp);
				rename("temporaryArchieve.tmp", arcName);
			}
			printf("File \"%s\" deleted\n", filename);
			return 1;
		}
		free(arr);
		freeTree(root);
		free(root);
	}
	printf("File \"%s\" can't be found in \"%s\", try \"-l %s\" to see files in archieve\n", file, arcName, arcName);
	return 0;
}