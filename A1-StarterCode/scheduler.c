#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include "scheduler.h"

void add_to_queue(struct PCB *process_pcb);
int run_Queue_inorder();
void reset_scheduler();
void SJF();
void FCFS();
int AGING();
int age_processes();
int reorder_queue();
int run_queue_RR();
void print_queue();
void swap_PCBs(struct PCB **first, struct PCB **second);
struct PCB *remove_from_Queue(struct PCB *pcb);

int PROCESS_NUMBER_GENERATOR = 69;

struct ready_queue
{
    struct PCB *HEAD;
} READY_QUEUE;

struct LRU_Queue LRU_q;

// 1 : FCFS, 2: SJF, 3: RR, 4: AGING
int POLICY = -1;
int CURRENT_PCBs_COUNT = 0;
struct PCB *pcb_array[3];

// creates process PCB, allocate memory, updates scheduler control variables
// index : max index of commands
// commands: pointers of commands
int createProcess(char *commands[], int index, int programNumber, int numberOfPages)
{
#ifdef TEST
    printf("creating process with %d command(s)\n", index + 1);
#endif
    // int firstIndex = assignProcessMemory(commands, index, PROCESS_NUMBER_GENERATOR);
    struct PCB *process_pcb = assignProcessMemory(commands, index, PROCESS_NUMBER_GENERATOR, programNumber, numberOfPages);
    if (process_pcb == NULL)
    {
        reset_scheduler();
        printf("process creation failed\n");
        return -1;
    }

#ifdef TEST
    printf("PCB created for PID %d\n", PROCESS_NUMBER_GENERATOR);
#endif
    // add to pcb array and update scheduler control variables
    PROCESS_NUMBER_GENERATOR += 100;
    pcb_array[(++CURRENT_PCBs_COUNT) - 1] = process_pcb;
#ifdef TEST
    printf("PCB added to pcb array at index %d\n", CURRENT_PCBs_COUNT - 1);
#endif
    return 0;
}

void add_to_LRU_HEAD(struct LRU_struct *lru_struct)
{
    struct LRU_struct *current;

    if (LRU_q.HEAD_Most_recent == NULL)
    {
#ifdef TEST
        printf("add_to_LRU_HEAD : HEAD null -> adding LRU_struct %d as HEAD \n", lru_struct->memory_page_index);
#endif
        LRU_q.HEAD_Most_recent = lru_struct;
        LRU_q.HEAD_Most_recent->next = NULL;
        return;
    }
    current = LRU_q.HEAD_Most_recent;
    LRU_q.HEAD_Most_recent = lru_struct;
    lru_struct->next = current;
    return;
}

struct LRU_struct *get_LRU_Struct_from_queue(int memory_page_index)
{
    struct LRU_struct *current = LRU_q.HEAD_Most_recent;
    while (current != NULL)
    {
        if (current->memory_page_index == memory_page_index)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct LRU_struct *remove_from_LRU_Queue(struct LRU_struct *lru_struct)
{
    if (LRU_q.HEAD_Most_recent == lru_struct) // struct is HEAD
    {
        LRU_q.HEAD_Most_recent = lru_struct->next;
        lru_struct->next = NULL;
        return lru_struct;
    }
    struct LRU_struct *current = LRU_q.HEAD_Most_recent;
    while (1)
    {
#ifdef TESTRR
        printf("RR remove_From_LRU_Queue: currrent LRU struct has frame_index : %d \n", current->memory_page_index);
#endif
        if (current->next == lru_struct)
        {
            current->next = lru_struct->next;
            lru_struct->next = NULL;
            break;
        }
        current = current->next;
    }
    return lru_struct;
}

//
int run_processes()
{
    // add to ready queue according to selected policy
    if (POLICY == 1)
    { // FCFS
        // FCFS();
        //  run Queue in added order
        // return run_Queue_inorder();
        exit(1);
    }
    else if (POLICY == 2)
    { // SJF
        // SJF();
        // return run_Queue_inorder();
        exit(1);
    }
    else if (POLICY == 3)
    { // RR
        // add all elements to Queue in input order
        FCFS();
        return run_queue_RR();
    }
    else if (POLICY == 4)
    { // SJF with AGING
        // return AGING();
        exit(1);
    }
    else
    {
        printf("no POLICY specified, current POLICY='%d' \n", POLICY);
        reset_scheduler();
        return 0;
    }
}

// run PCBs with AGING policy
/*int AGING()
{
    // add PCBs to Queue according to inital job Length score
    SJF();
#ifdef TESTAGING
    print_queue();
#endif
    struct PCB *current;
    if (READY_QUEUE.HEAD == NULL)
    {
        printf("Ready Queue head is null\n");
        return -1;
    }

    // current PCB info
    int startIndex;
    int toExecuteIndex;
    int commandsCount;
    int maxIndex;
    current = READY_QUEUE.HEAD;
#ifdef TESTAGING
    printf("aging: running first process as HEAD with PID '%d'\n", current->PID);
#endif
    // loop over Queue PCBs
    while (current != NULL)
    {
#ifdef TESTAGING
        printf("aging loop: running process with PID '%d'\n", current->PID);
        print_queue();
#endif
        // current PCB
        startIndex = current->memory_start_index;
        toExecuteIndex = current->instruction_to_execute_index;
        commandsCount = current->commands_count;
        maxIndex = startIndex + commandsCount - 1;
        // execute one command
        struct memory_struct *process_memory_struct = get_process_command_struct(toExecuteIndex++);
        char *command = strdup(process_memory_struct->value);
        parseInput(command);
        free(command);
#ifdef TESTAGING
        printf("aging loop: executed command from PID '%d'\n", current->PID);
#endif
        // check if PCB is finished
        if (toExecuteIndex > maxIndex)
        { // Process done, mark as finished
            current->finished = 1;
#ifdef TESTAGING
            printf("aging loop:  process with PID '%d' is done\n", current->PID);
#endif
        }
        else
        {
            // Process not done: increment instruction_to_execute_index in PCB
            current->instruction_to_execute_index = toExecuteIndex;
#ifdef TESTAGING
            printf("aging loop:process with PID '%d' has next_instruction_index incremented \n", current->PID);
#endif
        }
        // current stays the same unless Queue needs to be re-ordered
        if (age_processes())
        {
#ifdef TESTAGING
            printf("aging loop: reordering queue \n");
#endif
            if (reorder_queue())
            { // if reorder_Queue returns 1, all processes have finished executing
                break;
            }
            current = READY_QUEUE.HEAD; // if processes are left to execute, restart at HEAD
        }
    }
    // reset scheduler
    reset_scheduler();
    return 0;
}*/

// reorder queue based on AGING criteria, returns 1 if all processes are done executing
int reorder_queue()
{ // note: funtction wouldnt get called from AGING() if only HEAD left AND its not done executing
    // no other processes left AND HEAD done executing.
    if (READY_QUEUE.HEAD->next == NULL && READY_QUEUE.HEAD->finished)
    {
#ifdef TESTAGING
        printf("reorder Queue :head done and no processes left \n");
#endif
        READY_QUEUE.HEAD = NULL;
        return 1;
    }

    // copy Queue into array in same order except HEAD
    struct PCB *queue_arr[3];
    int queue_arr_index = 0;
    struct PCB *current = READY_QUEUE.HEAD->next;
    while (current != NULL)
    {
        queue_arr[queue_arr_index] = current;
        if (current->next != NULL)
            queue_arr_index++;
        current = current->next;
    }
#ifdef TESTAGING
    printf("reorder Queue: added '%d' pcbs to queue_arr  \n", queue_arr_index + 1);
#endif

    for (int i = 0; i < queue_arr_index + 1; i++) // bubble sort based on jobLengthScore
    {
        for (int j = 0; j < queue_arr_index - i; j++)
        {
            if (queue_arr[j]->jobLengthScore > queue_arr[j + 1]->jobLengthScore)
                swap_PCBs(&queue_arr[j], &queue_arr[j + 1]);
        }
    }
    // re create Queue with new order without previous HEAD
    struct PCB *prev_head = READY_QUEUE.HEAD;
    READY_QUEUE.HEAD = NULL;
    for (int i = 0; i <= queue_arr_index; i++)
    {
#ifdef TESTAGING
        printf("reorder Queue:process with PID '%d' added to new Queue\n", queue_arr[i]->PID);
#endif
        add_to_queue(queue_arr[i]);
    }
    // add previous HEAD to tail if not finised
    if (!prev_head->finished)
    {
#ifdef TESTAGING
        printf("reorder QUEUE: previous HEAD not finished, adding to tail of new Queue with PID '%d' \n", prev_head->PID);
#endif
        prev_head->next = NULL;
        add_to_queue(prev_head);
    }
    return 0;
}

// age all processes except HEAD, return 1 if Queue needs re-ordering and 0 otherwise
int age_processes()
{
    int head_jobs_score = READY_QUEUE.HEAD->jobLengthScore;
    int promote = 0;
    struct PCB *current = READY_QUEUE.HEAD->next;
#ifdef TESTAGING
    printf("age_processes(): ------\n");
#endif
    while (current != NULL)
    {
        if (current->jobLengthScore > 0) // age process
        {
            current->jobLengthScore--;
#ifdef TESTAGING
            printf("age_processes(): decreased jobScore for process %d, newjobScore : %d \n", current->PID, current->jobLengthScore);
#endif
        }
        if (current->jobLengthScore < head_jobs_score) // if aged process has lower jobsScore, promote is true
        {
#ifdef TESTAGING
            printf("age_processes(): process %d has lower jobsScore than HEAD -> poromte = 1\n", current->PID);
#endif
            promote = 1;
        }
        current = current->next;
    }
    if (READY_QUEUE.HEAD->finished)
    { // always return true if HEAD finishes
#ifdef TESTAGING
        printf("age_processes(): HEAD finished with PID %d, return 1 \n", READY_QUEUE.HEAD->PID);
#endif
        promote = 1;
    }
    return promote;
}
// add pcbs to queue in input order
void FCFS()
{
#ifdef TEST
    printf("running processes with FCFS policy : \n");
#endif
    int pcb_array_index = 0;
    while (pcb_array_index < CURRENT_PCBs_COUNT)
    {
        struct PCB *pcb = pcb_array[pcb_array_index++];
        add_to_queue(pcb);
#ifdef TEST
        printf("FCFS policy -- PCB added to ready queue with PID %d\n", pcb->PID);
#endif
    }
}

void swap_PCBs(struct PCB **first, struct PCB **second)
{
    struct PCB *temp = *first;
    *first = *second;
    *second = temp;
}

// add pcbs to queue according to their commands count(or jobLengthScore)
void SJF()
{
    struct PCB *copy_arr[CURRENT_PCBs_COUNT];
    for (int i = 0; i < CURRENT_PCBs_COUNT; i++) // copy pcb array
    {
        copy_arr[i] = pcb_array[i];
    }

    for (int i = 0; i < CURRENT_PCBs_COUNT; i++) // bubble sort based on jobLengthScore
    {
        for (int j = 0; j < CURRENT_PCBs_COUNT - i - 1; j++)
        {
            if (copy_arr[j]->jobLengthScore > copy_arr[j + 1]->jobLengthScore)
                swap_PCBs(&copy_arr[j], &copy_arr[j + 1]);
        }
    }
    // add PCBs to Queue based on copy_arr order
    int copy_array_index = 0;
    while (copy_array_index < CURRENT_PCBs_COUNT)
    {
        struct PCB *pcb = copy_arr[copy_array_index++];
        add_to_queue(pcb);
    }
}

void update_LRU(int curr_mem_index_of_executing_page)
{
#ifdef TESTRR
    printf("RR : update_LRU with curr_mem_index_of_executing_page = %d \n", curr_mem_index_of_executing_page);
#endif
    struct LRU_struct *curr_lru_struct = get_LRU_Struct_from_queue(curr_mem_index_of_executing_page);
#ifdef TESTRR
    printf("RR : update_LRU got LRU_struct with page_index = %d \n", curr_lru_struct->memory_page_index);
#endif
    if (curr_lru_struct == NULL)
    {
        printf("RR : cur_lru_struct is NULL, terminating \n");
        exit(1);
    }
    curr_lru_struct = remove_from_LRU_Queue(curr_lru_struct);
    add_to_LRU_HEAD(curr_lru_struct);
}
// runs processes in Queue one by one
/*int run_Queue_inorder()
{
    struct PCB *current;

    if (READY_QUEUE.HEAD == NULL)
    {
        printf("Ready Queue head is null\n");
        return -1;
    }
#ifdef TEST
    printf("running Queue in added order\n");
#endif
    current = READY_QUEUE.HEAD;
    // for each PCB in the queue, run all process commands
    while (current != NULL)
    {
        // shell memory first process command index
        int startIndex = current->memory_start_index;
        int commandsCount = current->commands_count;
        int currentIndex = startIndex;
#ifdef TEST
        printf("current process : %d \n", current->PID);
#endif
        while (currentIndex < startIndex + commandsCount)
        {
            struct memory_struct *process_memory_struct = get_process_command_struct(currentIndex++);
            char *command = strdup(process_memory_struct->value);
#ifdef TEST
            printf("running command :'%s' with length '%ld' chars \n", command, strlen(command));
#endif
            parseInput(command);
            free(command);
        }
        current = current->next;
    }
#ifdef TEST
    printf("processes executed successfully, deallocating processes \n");
#endif
    // clear processes
    reset_scheduler();
    return 0;
}*/
struct LRU_struct *get_last_LRU_struct()
{
    struct LRU_struct *current = LRU_q.HEAD_Most_recent;
    while (current != NULL)
    {
        if (current->next == NULL)
        {
            return current;
        }
        current = current->next;
    }
}

// load page into memory and update PCB of current and LRU_Queue, if frame replaced: update PCB of old frame too
// ;put pcb at end of queue and current becomes current->next
struct PCB *handle_page_fault(struct PCB *pcb)
{
    // pcb->page_table[] is -1
    int free_page_index = find_free_page();
    if (free_page_index == -1)
    { // memory full
        struct LRU_struct *last = remove_from_LRU_Queue(get_last_LRU_struct());
        add_to_LRU_HEAD(last);
        int replacedPID = overrwite_page(pcb, last->memory_page_index);
        pcb->page_table[pcb->index_of_executing_page] = last->memory_page_index;
        // update other PCB
        struct PCB *current = READY_QUEUE.HEAD;
        while (current != NULL)
        {
            if (current->PID == replacedPID)
            {
                for (int j = 0; j < 10; j++)
                {
                    if (current->page_table[j] == last->memory_page_index)
                    {
                        current->page_table[j] = -1;
                        break;
                    }
                }
                break;
            }
            current = current->next;
        }
    }
    else
    { // free spot found
        overrwite_page(pcb, free_page_index);
        // create LRU_stuct for new spot
        struct LRU_struct *current_lru_struct = malloc(sizeof(struct LRU_struct));
        current_lru_struct->memory_page_index = free_page_index;
        current_lru_struct->next = NULL;
        add_to_LRU_HEAD(current_lru_struct);
        pcb->page_table[pcb->index_of_executing_page] = free_page_index;
    }
    pcb->index_instruction_to_execute = 0;
    return pcb;
}
// run processes with RR policy
int run_queue_RR()
{
    struct PCB *current;
    if (READY_QUEUE.HEAD == NULL)
    {
        printf("Ready Queue head is null\n");
        return -1;
    }

    int loopCount;
    int removed = 0;
    int pageFault = 0;

    current = READY_QUEUE.HEAD;
#ifdef TESTRR
    printf("RR : HEAD at PID %d \n", current->PID);
#endif
    // loop over Queue PCBs repeatedly until no PCBs are left
    while (current != NULL)
    {
#ifdef TESTRR
        printf("RR : current process : %d \n", current->PID);
#endif
        // current PCB

        loopCount = 0;
        removed = 0;
        pageFault = 0;

        while (loopCount < 2) // try to execute 2 consecutive commands
        {
            removed = 0;
            pageFault = 0;

            int curr_index_of_executing_page = current->index_of_executing_page; // program{#}-page{index}
            // check if page in memory
            int curr_mem_index_of_executing_page = current->page_table[curr_index_of_executing_page];
            if (curr_mem_index_of_executing_page == -1) // page fault
            {
#ifdef TESTRR
                printf("RR : page fault with pcb  %d \n", current->PID);
#endif
                pageFault = 1;
                handle_page_fault(current);
                // load page into memory and update PCB of current and LRU_Queue, if frame replaced: update PCB of old frame too
                // put pcb at end of queue and current becomes current->next
                break;
            }
            // page in memory, get and execute command
            struct page_struct *mem_page_struct = get_process_command_struct(curr_mem_index_of_executing_page);
            if (mem_page_struct->PID != current->PID)
            { // double check that page belongs to current process
                printf("check page at memory_index %d belongs to process %d failed, terminating\n", curr_mem_index_of_executing_page, current->PID);
                exit(1);
            }
            int ind_of_instruction_in_page_toexecute = current->index_instruction_to_execute;
            char *command = strdup(mem_page_struct->lines[ind_of_instruction_in_page_toexecute++]);
            parseInput(command);
            free(command);

            // update LRU Queue
            // get LRU struct, will always be there since code executed from memory, and put it in front of Queue
            update_LRU(curr_mem_index_of_executing_page);
#ifdef TESTRR
            printf("RR : updated LRU\n");
#endif
// update program counter , check if page done, process done
#ifdef TESTRR
            printf("RR : updated PC: new ind_of_instruction_in_page_toexecute is %d and number_of_lines is %d\n", ind_of_instruction_in_page_toexecute, mem_page_struct->number_of_lines);
#endif
            if (ind_of_instruction_in_page_toexecute + 1 > mem_page_struct->number_of_lines)
            { // current page done executing

                if (current->index_of_executing_page + 1 == current->number_of_pages)
                { // process done
#ifdef TESTRR
                    printf("RR : process done and to be removed  %d \n", current->PID);
#endif              
                    current->finished = 1;
                    current = remove_from_Queue(current);
                    removed = 1;
                    break;
                }
                else
                { // just page done

                    // increment page index
                    int currPageIndex = current->index_of_executing_page;
                    current->index_of_executing_page = currPageIndex + 1;
                    // reset PC
                    current->index_instruction_to_execute = 0;
#ifdef TESTRR
                    printf("RR : page done , new page index_of_executing_page is %d \n", current->index_of_executing_page);
#endif
                }
            }
            else
            {
                // nothing done just increment instr
                current->index_instruction_to_execute = ind_of_instruction_in_page_toexecute;
#ifdef TESTRR
                printf("RR : just instr_index incremented, new is  %d \n", current->index_instruction_to_execute);
#endif
            }

            loopCount++;
        }
        // if removed do nothing current updated
        if (!removed) // page fault or no pcb removed
        {             // current becomes next PCB or HEAD if last pcb wasn't removed from Queue
#ifdef TESTRR
            printf("RR : after while loop, !removed condition met\n");
#endif
            if (current->next == NULL)
                current = READY_QUEUE.HEAD; // end of Queue, restart at HEAD
            else
                current = current->next; // or continue to next PCB in Queue
        }
    }
    // clear processes
    reset_scheduler();
    return 0;
}

// remove PCB from Queue, return next PCB or NULL if no PCBs left in the Queue
struct PCB *remove_from_Queue(struct PCB *pcb)
{
    if (READY_QUEUE.HEAD == pcb) // pcb is HEAD
    {
        if (pcb->next == NULL) // only HEAD is left in Queue
        {
#ifdef TESTRR
            printf("RR remove: only HEAD left: %d \n", pcb->PID);
#endif
            READY_QUEUE.HEAD = NULL;
            return NULL;
        }
#ifdef TESTRR
        printf("RR remove: PCB at HEAD with other PCBs left  : %d \n", pcb->PID);
#endif
        // other PCB(s) after pcb left in Queue
        READY_QUEUE.HEAD = pcb->next;
        pcb->next = NULL;
        return READY_QUEUE.HEAD;
    }
    else if (pcb->next != NULL) // pcb is in the middle
    {
#ifdef TESTRR
        printf("RR remove: PCB in the middle  : %d \n", pcb->PID);
#endif
        READY_QUEUE.HEAD->next = pcb->next;
        pcb->next = NULL;
        return READY_QUEUE.HEAD->next;
    }
    else
    { // pcb is at the end of the Queue
#ifdef TESTRR
        printf("RR remove: PCB at the end  : %d \n", pcb->PID);
#endif
        if (READY_QUEUE.HEAD->next == pcb) // pcb is second
        {
            READY_QUEUE.HEAD->next = NULL;
        }
        else
        { // pcb is third
            READY_QUEUE.HEAD->next->next = NULL;
        }
        return READY_QUEUE.HEAD;
    }
}
// add to queue's tail
void add_to_queue(struct PCB *process_pcb)
{
#ifdef TEST
    printf("add_to_queue : process %d  \n", process_pcb->PID);
#endif
    struct PCB *current;

    if (READY_QUEUE.HEAD == NULL)
    {
#ifdef TEST
        printf("add_to_queue : HEAD null -> adding process %d as HEAD \n", process_pcb->PID);
#endif
        READY_QUEUE.HEAD = process_pcb;
        READY_QUEUE.HEAD->next = NULL;
        return;
    }
    current = READY_QUEUE.HEAD;
    while (current != NULL)
    {
        if (current->next == NULL)
        {
            current->next = process_pcb;
            process_pcb->next = NULL; // added
            break;
        }
        current = current->next;
    }
    return;
}

void initialize_scheduler(int policy)
{
    POLICY = policy;
    CURRENT_PCBs_COUNT = 0;
    READY_QUEUE.HEAD = NULL;
    LRU_q.HEAD_Most_recent = NULL;
}

// clear shell memory related to all processes, reset scheduler control variables, free PCBs in pcb_array
void reset_scheduler()
{
#ifdef TEST
    printf("reset scheduler started : %d processes to deallocate\n", CURRENT_PCBs_COUNT);
#endif
    POLICY = -1;
    resetmem();
    // deallocate PCBs from pcb_array
    while (CURRENT_PCBs_COUNT > 0)
    {
        struct PCB *currentPCB = pcb_array[CURRENT_PCBs_COUNT - 1];

        // free PCB from pcb_array
        free(currentPCB);
#ifdef TEST
        printf("PCB removed from pcb_array \n");
#endif
        CURRENT_PCBs_COUNT--;
    }
    // clear Queue
    READY_QUEUE.HEAD = NULL;

    //clear LRU
    struct LRU_struct *current = LRU_q.HEAD_Most_recent;
    while (current != NULL)
    {
        struct LRU_struct *next = current->next;
        free(current);
        current = next;
    }
    LRU_q.HEAD_Most_recent = NULL;
}

void print_queue()
{
    struct PCB *current = READY_QUEUE.HEAD;
    while (current != NULL)
    {
        printf("view Queue : %d\n", current->PID);
        current = current->next;
    }
}