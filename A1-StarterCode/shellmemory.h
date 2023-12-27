#ifndef SHELLMEMORY_H_
#define SHELLMEMORY_H_

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
struct PCB* assignProcessMemory(char* commands[], int maxIndex, int pid, int programNumber, int numberOfPages);
struct page_struct* get_process_command_struct(int memory_page_index);
int send_to_backstore(char* commands[], int maxIndex, int progNumber);
int find_free_page();
void resetmem();
struct variable_struct{
	int PID;
	char *var;
	char *value;
};
struct page_struct{
	int PID;
	int number_of_lines;
	char* lines[3];
};

int overrwite_page(struct PCB* pcb, int mem_index);

#endif