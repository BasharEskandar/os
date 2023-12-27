#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "shell.h"
#include "scheduler.h"

struct variable_struct variable_memory[VARMEMSIZE];
struct page_struct page_memory[FRAMESIZE / 3];

void mem_init()
{

	int i;
	for (i = 0; i < VARMEMSIZE; i++)
	{
		variable_memory[i].PID = -1;
		variable_memory[i].var = "none";
		variable_memory[i].value = "none";
	}
	for (i = 0; i < FRAMESIZE / 3; i++)
	{
		page_memory[i].PID = -1;
		page_memory[i].number_of_lines = 0;
	}
}

struct page_struct *get_process_command_struct(int memory_page_index)
{
	return &(page_memory[memory_page_index]);
}

/*int findContiguousMemoryBlock(int commandsCount){

	//loop through mem to find first empty spot
	for(int i =0; i<1000; i++){
		//found empty spot
		if(shellmemory[i].PID == -1 && strcmp(shellmemory[i].var, "none") == 0){
			int empty = 1;
			int j;
			//check empty from index i to i+commandsCount - 1
			for(j = i;j<i+commandsCount; j++){
				//if spot not empty
				if(shellmemory[i].PID != -1 || shellmemory[i].var != "none"){
					i = j;
					empty = 0;
					break;
				}
			}
			if(empty){
				//success -> return first index
				return i;
			}
			// - (-) -    -    !-   -     -<
			//	  j  j+1  j+2  j+3 j+4    j+5
		}
	}
	printf("error : could not find %d contiguous empty memory spots\n",commandsCount);
	return -1;
}*/

int createPage(char *commands[], int progNumber, int pageIndex, int startIndex, int endIndex)
{
	char fileName[15];
	sprintf(fileName, "prog%d-page%d", progNumber, pageIndex);

	char dirPath[100] = "./backingStore/";
	strcat(dirPath, fileName);

	FILE *p = fopen(dirPath, "w");
	if (p == NULL)
	{
		return -1;
	}
	for (int i = startIndex; i <= endIndex; i++)
	{

		if (i == endIndex)
		{
			commands[i][strcspn(commands[i], "\n")] = 0;
		}
#ifdef TEST
		printf("create page: writing line '%s' to page '%s' ", commands[i], dirPath); // print a\0  vs print a\n\0 ->
#endif
		fprintf(p, "%s", commands[i]);
	}
	fclose(p);
	return 0;
}

/*
 *converts script into 3-line frames and stores them in backstore directory
 *return: number of frames
 */
int send_to_backstore(char *commands[], int maxIndex, int progNumber)
{

	int number_of_pages;
	int linesLastPage;
	int division = (maxIndex + 1) / 3;
	int modulo = (maxIndex + 1) % 3;

	if (modulo == 0)
	{
		number_of_pages = division;
		linesLastPage = 3;
	}
	else
	{
		number_of_pages = division + 1;
		linesLastPage = modulo;
	}

	for (int page = 1; page <= number_of_pages; page++)
	{
		int startIndex = (page - 1) * 3;
		int endIndex;
		if (page == number_of_pages)
		{
			endIndex = startIndex + linesLastPage - 1;
		}
		else
		{
			endIndex = startIndex + 2;
		}

		int error = createPage(commands, progNumber, page - 1, startIndex, endIndex);
		if (error == -1)
			return -1;
	}
	return number_of_pages;
}

int find_free_page()
{
	for (int i = 0; i < FRAMESIZE / 3; i++)
	{
		if (page_memory[i].PID == -1)
			return i;
	}
	return -1;
}

int overrwite_page(struct PCB *pcb, int mem_index)
{
	int previousPID = page_memory[mem_index].PID;
	if (previousPID != -1) // non empty frame
	{
		printf("Page fault! Victim page contents:\n");
		for (int i = 0; i < page_memory[mem_index].number_of_lines; i++)
		{
			printf("%s", page_memory[mem_index].lines[i]);
		}

		printf("End of victim page contents.\n");
	}
	page_memory[mem_index].PID = pcb->PID;

	char fileName[15];
	sprintf(fileName, "prog%d-page%d", pcb->prog_number, pcb->index_of_executing_page);

	char dirPath[100] = "./backingStore/";
	strcat(dirPath, fileName);

	char line[1000];
	memset(line, 0, sizeof(line));

	FILE *p = fopen(dirPath, "rt");
	if (p == NULL)
	{
		printf("overrwite page : page %s not found\n", dirPath);
		return -1;
	}
	fgets(line, 999, p);
	int index = 0;
	while (1)
	{
		size_t length = strlen(line);
#ifdef TEST
		printf("length of line : %ld", length); // print a\0  vs print a\n\0 ->
#endif
		if (line[length - 1] != '\n')
		{
			line[length] = '\n';
			line[length + 1] = '\0';
		}
		page_memory[mem_index].lines[index] = strdup(line);
#ifdef TEST
		printf("reading script : added command '%s' to frame array\n", line);
#endif
		memset(line, 0, sizeof(line));
		if (feof(p))
		{
#ifdef TEST
			printf("reading script: end of file reached , stopped at index %d \n", index);
#endif
			break;
		}
		fgets(line, 999, p);
		index++;
	}
	fclose(p);

	page_memory[mem_index].number_of_lines = index + 1;
	return previousPID;
}
/*
 *commands: array of pointers to commmand strings
 *maxIndex: max index of commands[]
 *pid: process id of process to be created
 *return: first Index in shell memory where commands start
 */
struct PCB *assignProcessMemory(char *commands[], int maxIndex, int pid, int programNumber, int numberOfPages)
{
#ifdef TEST
	printf("assigning process memory with pid : %d\n", pid);
#endif
	struct PCB *process_pcb = malloc(sizeof(struct PCB));
	// int firstIndex = findContiguousMemoryBlock(maxIndex + 1); // maxindex == 2 -> 3 elements
	// load 2 first (or 1) pages of the process into memory from backstore
	int first;
	int pageMemoCount = 0;
	for (int i = 0; i <= 1; i++) // load 2 pages or 1
	{
		int firstIndex = find_free_page();
		if (i == 0)
			first = firstIndex;

		process_pcb->page_table[i] = firstIndex;
		page_memory[firstIndex].PID = pid;

		struct LRU_struct *current_lru_struct = malloc(sizeof(struct LRU_struct));
		current_lru_struct->memory_page_index = firstIndex;
		current_lru_struct->next = NULL;
		add_to_LRU_HEAD(current_lru_struct);

		char fileName[15];
		sprintf(fileName, "prog%d-page%d", programNumber, i);
#ifdef TEST
		printf("assignProcessMemory: fileName is '%s'\n", fileName);
#endif
		char dirPath[100] = "./backingStore/";
		strcat(dirPath, fileName);
#ifdef TEST
		printf("assignProcessMemory: dirPath is '%s'\n", dirPath);
#endif
		char line[1000];
		memset(line, 0, sizeof(line));

		FILE *p = fopen(dirPath, "rt");
		if (p == NULL)
		{
#ifdef TEST
			printf("assignProcessMemory: unable to open file : '%s'\n", dirPath);
#endif
			return NULL;
		}
		int index = 0;
		int numberOfLines = 0;
		fgets(line, 999, p);
		while (1)
		{
			size_t length = strlen(line);
			if (line[length - 1] != '\n')
			{
				line[length] = '\n';
				line[length + 1] = '\0';
			}
			page_memory[firstIndex].lines[index] = strdup(line);
#ifdef TEST
			printf("reading script: inserted line : '%s' at index '%d' \n", line, index);
#endif
			numberOfLines++;
			memset(line, 0, sizeof(line));
			if (feof(p))
			{
#ifdef TEST
				printf("reading script: end of file reached , stopped at index %d \n", index);
#endif
				break;
			}
			fgets(line, 999, p);
			index++;
		}
		fclose(p);
		pageMemoCount++;
		page_memory[firstIndex].number_of_lines = numberOfLines;
		if (maxIndex <= 2)
			break;
	}

	process_pcb->PID = pid;
	process_pcb->number_of_pages = numberOfPages;
	process_pcb->prog_number = programNumber;

	process_pcb->index_of_inmemory_executing_page = first;
	process_pcb->index_instruction_to_execute = 0;
	for (int j = pageMemoCount; j <= 11; j++)
	{
		process_pcb->page_table[j] = -1;
	}
	process_pcb->pages_loaded_in_memory = pageMemoCount;
	process_pcb->index_of_executing_page = 0;

	process_pcb->commands_count = maxIndex + 1;
	process_pcb->jobLengthScore = maxIndex + 1;
	process_pcb->finished = 0;
	process_pcb->next = NULL;

	return process_pcb;
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in)
{

	int i;
	for (i = 0; i < VARMEMSIZE; i++)
	{
		if (strcmp(variable_memory[i].var, var_in) == 0)
		{
			variable_memory[i].value = strdup(value_in);
			return;
		}
	}
	// Value does not exist, need to find a free spot.
	for (i = 0; i < VARMEMSIZE; i++)
	{
		if (strcmp(variable_memory[i].var, "none") == 0)
		{
			variable_memory[i].var = strdup(var_in);
			variable_memory[i].value = strdup(value_in);
			return;
		}
	}
	return;
}

// get value based on input key
char *mem_get_value(char *var_in)
{
	int i;

	for (i = 0; i < VARMEMSIZE; i++)
	{
		if (strcmp(variable_memory[i].var, var_in) == 0)
		{

			return strdup(variable_memory[i].value);
		}
	}
	return "Variable does not exist";
}

// resets the variables memory
void resetmem()
{
	int i;
	for (i = 0; i < VARMEMSIZE; i++)
	{
		variable_memory[i].PID = -1;
		variable_memory[i].var = "none";
		variable_memory[i].value = "none";
	}

	for (i = 0; i < FRAMESIZE / 3; i++)
	{
		if (page_memory[i].PID != -1)
		{
			for(int j = 0; j<page_memory[i].number_of_lines;j++)
			{
				free(page_memory[i].lines[j]);
			}
			page_memory[i].PID = -1;
			page_memory[i].number_of_lines = 0;
		}
	}
}