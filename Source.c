#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <winuser.h>
#include <wchar.h>
#include <io.h>
#include <stdio.h>
#include "Functions.h"

int main(unsigned int argc, char** argv)
{
	FILE *arc = NULL, *in = NULL, *out = NULL;

	if (!strcmp(argv[1], "-h"))
	{
		printf("a - zip file(s)\nx - unzip file(s)\nd - delete file(s) from archieve\nl - get information about archieve\nt - check archieve integrity\nh - get help");
	}	
	else
	if (!strcmp(argv[1], "-a"))//поместить файл(ы) в архив
	{
		arc = fopen(argv[2], "r+b");
		if (!arc)
		{
			arc = fopen(argv[2], "w");
			arc = fopen(argv[2], "r+b");
		}
		for (int i = 3; i < argc; ++i)
		{
			zip(argv[i], arc);
		}
		writeControll(arc);
	}
	else
	if (!strcmp(argv[1], "-x"))//извлечь файл(ы) из архива;
	{
		arc = fopen(argv[2], "rb");
		if (!arc)
		{
			printf("Archieve \"%s\" can't be found\n", argv[2]);
			exit(0);
		}
		for (int i = 3; i < argc; ++i)
		{
			unzip(arc, argv[i]);
		}
	}
	else
	if (!strcmp(argv[1], "-d"))//удалить файл(ы) из архива;
	{
		for (int i = 3; i < argc; ++i)
		{
			del(argv[2], argv[i]);
		}
		arc = fopen(argv[2], "r+b");
		int sum = writeControll(arc);
		_fcloseall();
		if (!sum)
		{
			remove(argv[2]);
		}
	}
	else
	if (!strcmp(argv[1], "-l"))//вывести информацию о файлах, хранящихся в архиве;
	{
		arc = fopen(argv[2], "rb");
		if (!arc)
		{
			printf("Archieve \"%s\" can't be found\n", argv[2]);
			exit(0);
		}
		info(arc);
	}
	else
	if (!strcmp(argv[1], "-t"))//проверить целостность архива.
	{
		arc = fopen(argv[2], "r+b");
		if (!arc)
		{
			printf("Archieve \"%s\" can't be found\n", argv[2]);
			exit(0);
		}
		if (checkIntegrity(arc))
		{
			printf("archieve is fine\n");
		}
		else
		{
			printf("archieve is NOT fine\n");
		}
	}
	else
	{
		printf("Unknown operation, try -h for operation list");
	}
	_fcloseall();
}