
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "scheduler.h"
#include "interpreter.h"
#include "shellmemory.h"
#include "shell.h"

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);
int parseCommands(char ui[]);
void createBackingStore();
// Start of everything
int main(int argc, char *argv[])
{
	printf("Frame Store Size = %d ; Variable Store Size = %d\n", FRAMESIZE, VARMEMSIZE);
	createBackingStore();
	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();

	char prompt = '$';				// Shell prompt
	char userInput[MAX_USER_INPUT]; // user's input stored here
	int errorCode = 0;				// zero means no error, default

	// init user input
	for (int i = 0; i < MAX_USER_INPUT; i++)
		userInput[i] = '\0';

	// init shell memory
	mem_init();
	while (1)
	{
		printf("%c ", prompt);

		fgets(userInput, MAX_USER_INPUT - 1, stdin);

		if (feof(stdin))
		{
			freopen("/dev/tty", "r", stdin);
		}
#ifdef TEST
		printf("sending input : [%s] to new parser\n", userInput);
#endif
		errorCode = parseInput(userInput);
		if (errorCode == -1)
			exit(99); // ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	}

	return 0;
}

void createBackingStore()
{
	struct stat st = {0};

	if (stat("./backingStore", &st) == -1)
	{
		mkdir("./backingStore", 0700);
	}
	else
	{
		printf("cleaning store\n");
		DIR *d;
		struct dirent *dir;
		d = opendir("./backingStore");
		if (d)
		{
			while ((dir = readdir(d)) != NULL)
			{
				char dirPath[100] = "./backingStore/";
				if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
				{
					printf("removing %s\n", dir->d_name);
					remove(strcat(dirPath, dir->d_name));
				}
			}
			closedir(d);
		}
	}
}

void removeBackingStore()
{
	DIR *d;
	struct dirent *dir;
	d = opendir("./backingStore");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			char dirPath[100] = "./backingStore/";
			if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
			{
				printf("removing %s\n", dir->d_name);
				remove(strcat(dirPath, dir->d_name));
			}
		}
		closedir(d);
	}
	rmdir("./backingStore");
}
// parse commands based on ';' CURRENTLY UNUSED
int parseCommands(char ui[])
{
	char *token = strtok(ui, ";");
	int errCode = 0;
	// loop through the string to extract all other tokens
	while (token != NULL)
	{
		printf("tokenizer : sending token [%s] to parser\n", token);
		if (parseInput(strdup(token)) == -1)
			errCode = -1; // input each token
		token = strtok(NULL, ";");
	}
	return errCode;
}

// Extract words from the input then call interpreter
int parseInput(char ui[])
{
#ifdef TEST
	printf("parseInput : parsing command '%s'\n", ui);
#endif
	char tmp[200];
	char *words[100];
	int a = 0;
	int b;
	int w = 0; // wordID
	int errorCode;
	for (a = 0; ui[a] == ' ' && a < 1000; a++)
		; // skip white spaces
	while (ui[a] != '\n' && ui[a] != '\0' && a < 1000)
	{
		for (b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' && ui[a] != ' ' && a < 1000; a++, b++)
			tmp[b] = ui[a]; // extract a word
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		if (ui[a] == ';')
		{
#ifdef TEST
			printf("parseInput : found ';' at index a = '%d'\n", a);
#endif
			w++;

			errorCode = interpreter(words, w);
			if (errorCode == -1)
			{
				return errorCode;
			}

			a++;
			w = 0;
			for (; ui[a] == ' ' && a < 1000; a++)
				; // skip white spaces
			continue;
		}
		a++;
		w++;
	}
	errorCode = interpreter(words, w);

	return errorCode;
}
