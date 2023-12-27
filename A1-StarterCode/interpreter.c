#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "shellmemory.h"
#include "shell.h"

#include "scheduler.h"
#include "interpreter.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int set(char *command_args[], int size);
int print(char *var);
int exec(char *comman_args[], int size);
int run(char *script);
int echo_variable(char *command_args[], int size);
char *recombine_args(char *command_args[], int size);
int list_dir();
int badcommandFileDoesNotExist();
struct script_read read_script(char *script);

struct script_read
{
	int max_index;
	char **temp_mem_pointer;
};

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size)
{

	int i;
	// args_size is max index + 1
	// args end with \0
	if (args_size < 1)
	{
		return badcommand();
	}
#ifdef TEST
	printf("args_size is %d\n", args_size);
	for (int ind = 0; ind < args_size; ind++)
	{
		printf("argument with index %d is : '%s' with length '%ld' \n", ind, command_args[ind], strlen(command_args[ind]));
	}
#endif
	if (args_size > MAX_ARGS_SIZE)
	{
		if (strcmp(command_args[0], "set") == 0)
		{
			printf("%s\n", "Bad command: Too many tokens");
			return 1;
		}
		else if (strcmp(command_args[0], "echo") == 0)
		{
		}
		else
		{
			return badcommand();
		}
	}

	for (i = 0; i < args_size; i++)
	{ // strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help") == 0)
	{
		// help
		if (args_size != 1)
			return badcommand();
		return help();
	}
	else if (strcmp(command_args[0], "quit") == 0)
	{
		// quit
		if (args_size != 1)
			return badcommand();
		return quit();
	}
	else if (strcmp(command_args[0], "set") == 0)
	{
		if (args_size < 3)
			return badcommand();
		// set
		return set(command_args, args_size);
	}

	else if (strcmp(command_args[0], "exec") == 0)
	{
		if (args_size < 3 || args_size > 5)
		{
			printf("exec bad command \n");
			return 0;
		}

		return exec(command_args, args_size);
	}

	else if (strcmp(command_args[0], "echo") == 0)
	{
		if (*command_args[1] == '$')
			return echo_variable(command_args, args_size);
		char *string = recombine_args(command_args, args_size);
		printf("%s\n", string);
		free(string);
		return 0;
	}
	else if (strcmp(command_args[0], "print") == 0)
	{
		if (args_size != 2)
			return badcommand();
		return print(command_args[1]);
	}
	else if (strcmp(command_args[0], "my_ls") == 0)
	{
		if (args_size != 1)
			return badcommand();
		return list_dir();
	}
	else if (strcmp(command_args[0], "run") == 0)
	{
		if (args_size != 2)
			return badcommand();
		return run(command_args[1]);
	}
	else
		return badcommand();
}

int help()
{

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit()
{
	removeBackingStore();
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand()
{
	printf("%s\n", "Unknown Command");
	return 1;
}

int list_dir()
{
	int errCode = system("ls | sort");
	return errCode;
}

// For run command only
int badcommandFileDoesNotExist()
{
	printf("%s\n", "Bad command: File not found");
	return 3;
}

// recombine arguments into one string
char *recombine_args(char *command_args[], int size)
{
	char *space = " ";
	char *buffer = malloc(sizeof(char) * 100);
	strcpy(buffer, command_args[1]); // each token is null terminated
	// command a b   +  c d e f
	for (int i = 2; i < size; i++)
	{
		strcat(buffer, space);
		strcat(buffer, command_args[i]);
	}
	return buffer;
}
int set(char *command_args[], int size)
{
	char *space = " ";
	char buffer[1000];
	strcpy(buffer, command_args[2]); // each token is null terminated
	// set a b   +  c d e f size = 7
	for (int i = 3; i < size; i++)
	{
		strcat(buffer, space);
		strcat(buffer, command_args[i]);
	}
	mem_set_value(command_args[1], buffer); // recombine args separated by spaces into 1 string

	return 0;
}

// echo variable that starts with '$'
int echo_variable(char *command_args[], int size)
{
	if (size > 2)
	{
		return badcommand();
	}
	char var[100];
	strcpy(var, command_args[1] + 1);
	char *result = mem_get_value(var);
	if (strcmp(result, "Variable does not exist") == 0)
	{
		printf("\n");
		return 0;
	}
	printf("%s\n", result);
	return 0;
}

int print(char *var)
{
	printf("%s\n", mem_get_value(var));
	return 0;
}

int exec(char *command_args[], int args_size)
{
	/*#ifdef TEST
		printf("exec : checking scripts names\n");
	#endif
		for (int i = 1; i < args_size - 1; i++)
		{
			for (int j = 1; j < args_size - 1; j++)
			{
				// if not same index AND same files then error
				if (i != j && strcmp(command_args[i], command_args[j]) == 0)
				{
					printf("exec bad command : 2 or more identical scripts\n");
					return 1;
				}
			}
		}
		*/
	char *policy = command_args[args_size - 1];
#ifdef TEST
	printf("exec : checking policy\n");
#endif
	if (strcmp(policy, "FCFS") == 0)
	{
		initialize_scheduler(1);
	}
	else if (strcmp(policy, "SJF") == 0)
	{
		initialize_scheduler(2);
	}
	else if (strcmp(policy, "RR") == 0)
	{
		initialize_scheduler(3);
	}
	else if (strcmp(policy, "AGING") == 0)
	{
		initialize_scheduler(4);
	}
	else
	{
		printf("policy '%s' is undefined, exec terminated\n", policy);
		return 1;
	}

	int error = 0;
	// create processes .. not started yet
	for (int ind = 1; ind < args_size - 1; ind++)
	{
#ifdef TEST
		printf("exec : reading script\n");
#endif
		struct script_read result = read_script(command_args[ind]);
		if (result.max_index == -1)
		{
			printf("script reading error : could not open script %s, possible path error\n", command_args[ind]);
			return 0;
		}
#ifdef TEST
		printf("exec : storing script in backstore with format:  prog{prog#}-page{page#}\n");
#endif
		int number_of_pages = send_to_backstore(result.temp_mem_pointer, result.max_index, ind);

		if (number_of_pages == -1)
		{
			printf("exec failed : process creation for script %s failed with error in send_to_backstore, terminating\n", command_args[ind]);
			return 0;
		}
		error = createProcess(result.temp_mem_pointer, result.max_index, ind, number_of_pages);
		free(result.temp_mem_pointer);
		// process creation error
		if (error != 0)
		{
			printf("exec failed : process creation for script %s failed, terminating\n", command_args[ind]);
			return 0;
		}
	}
	// run process(es)
	return run_processes();
}

int run(char *script)
{
#ifdef TEST
	printf("run : read script\n");
#endif

	initialize_scheduler(3);
	struct script_read result = read_script(script);

	if (result.max_index == -1)
	{
		printf("script reading error : could not open script %s, possible path error\n", script);
		return 0;
	}
#ifdef TEST
	printf("run : saving script to backstore\n");
#endif
	int number_of_pages = send_to_backstore(result.temp_mem_pointer, result.max_index, 1);
	if (number_of_pages == -1)
	{
		printf("exec failed : process creation for script failed with error in send_to_backstore, terminating\n");
		return 0;
	}

#ifdef TEST
	printf("run : calling create process\n");
#endif

	int error = createProcess(result.temp_mem_pointer, result.max_index, 1, number_of_pages);
	free(result.temp_mem_pointer);

	if (error != 0)
	{
		printf("exec failed : process creation for script failed, terminating\n");
		return 0;
	}
	return run_processes();

}

// reads script and return pointer to commands read in a script_read structure with max index
struct script_read read_script(char *script)
{
	int index = 0;
	struct script_read result;
	char line[1000];
	memset(line, 0, sizeof(line));

	char **tempMem = malloc(300 * sizeof(char *));
	FILE *p = fopen(script, "rt"); // the program is in a file
	if (p == NULL)
	{
		result.max_index = -1;
		return result;
	}
	fgets(line, 999, p);
	while (1)
	{
		size_t length = strlen(line);
#ifdef TEST
		printf("length of line : %ld", length); // print a\0  vs print a\n\0 ->
#endif
		/*if (line[length - 1] != '\n')
		{
			line[length] = '\n';
			line[length + 1] = '\0';
		}*/
		tempMem[index] = strdup(line);
#ifdef TEST
		printf("reading script : added '%s' to tempMem array\n", tempMem[index]);
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

	result.max_index = index;
	result.temp_mem_pointer = tempMem;
	return result;
}
