#ifndef SCHEDULER_H_
#define SCHEDULER_H_
int createProcess(char* commands[], int index,int programNumber, int numberOfPages );
void initialize_scheduler(int policy);
int run_processes();


struct PCB{
    int PID;
    //
    //int memory_start_index;
    //int instruction_to_execute_index;
    //
    int number_of_pages;
    int prog_number;

    int index_of_inmemory_executing_page;//index in frame store of current page REMOVE
    
    int index_instruction_to_execute;//index in page struct of instr in current page
    int page_table[10]; //page_table[i] is -1 if page{i} is not in memory, or int for its index inmemory
    int index_of_executing_page;// prorgam-scoped index of current page -> page{index}

    int pages_loaded_in_memory;//number of pages loaded memory REMOVE

    int commands_count;
    int jobLengthScore;
    struct PCB* next;
    int finished;
};

struct LRU_struct{
    int memory_page_index;
    struct LRU_struct* next;
};

struct LRU_Queue{
    struct LRU_struct* HEAD_Most_recent;
};
void add_to_LRU_HEAD(struct LRU_struct* lru_struct);
struct LRU_struct* remove_from_LRU_Queue(struct LRU_struct* lru_struct);
struct LRU_struct* get_LRU_Struct_from_queue(int memory_page_index);
struct LRU_struct* get_last_LRU_struct();
#endif