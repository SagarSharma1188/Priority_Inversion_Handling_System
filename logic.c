#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS      3
#define PRIO_HIGH      1
#define PRIO_MED       5
#define PRIO_LOW      10
#define MUTEX_CEILING  PRIO_HIGH
#define TRUE  1
#define FALSE 0
#define ID_HIGH 0
#define ID_MED  1
#define ID_LOW  2

typedef enum { READY, RUNNING, BLOCKED, FINISHED } TaskState;

typedef struct {
    int id;
    char name[20];
    int base_prio;
    int cur_prio;
    TaskState state;
    int needs_mutex;
    int holding_mutex;
    int arrive_tick;
    int total_ticks;
    int done_ticks;
    int mutex_ticks;
    int mutex_done;
} Task;
Task tasks[MAX_TASKS];
int  mutex_owner; /* -1 = free */

void sep(void)     { printf("==================================================\n"); }
void sub_sep(void) { printf("--------------------------------------------------\n"); }

const char* st(TaskState s) {
    if(s==READY)    return "READY   ";
    if(s==RUNNING)  return "RUNNING ";
    if(s==BLOCKED)  return "BLOCKED ";
    if(s==FINISHED) return "FINISHED";
    return "?";
}
void reset(void) {
    mutex_owner = -1;
    /* HIGH: arrives t=2, needs mutex, 6 ticks, holds mutex 3 ticks */
    tasks[ID_HIGH] = (Task){ID_HIGH,"HIGH_Task",PRIO_HIGH,PRIO_HIGH,READY,TRUE,FALSE,2,6,0,3,0};
    /* MED: arrives t=3, no mutex, 4 ticks */
    tasks[ID_MED]  = (Task){ID_MED, "MED_Task", PRIO_MED, PRIO_MED, READY,FALSE,FALSE,3,4,0,0,0};
    /* LOW: arrives t=0, starts RUNNING, holds mutex, 8 ticks, mutex 5 ticks */
    tasks[ID_LOW]  = (Task){ID_LOW, "LOW_Task", PRIO_LOW, PRIO_LOW, RUNNING,TRUE,TRUE,0,8,0,5,0};
    mutex_owner = ID_LOW;
    /* Hide HIGH and MED until arrival */
    tasks[ID_HIGH].state = FINISHED;
    tasks[ID_MED].state  = FINISHED;
}
