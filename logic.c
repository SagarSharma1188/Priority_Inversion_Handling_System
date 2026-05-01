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
int pick_runner(void) {
    int best=-1, bp=9999;
    for(int i=0;i<MAX_TASKS;i++) {
        if(tasks[i].state==RUNNING || tasks[i].state==READY)
            if(tasks[i].cur_prio < bp) { bp=tasks[i].cur_prio; best=i; }
    }
    return best;
}

int all_done(void) {
    for(int i=0;i<MAX_TASKS;i++) if(tasks[i].state!=FINISHED) return FALSE;
    return TRUE;
}

void print_state(int tick, int runner) {
    printf("[T=%02d] %-12s prio=%-2d %s | %-12s prio=%-2d %s | %-12s prio=%-2d %s  => RUNS: %s\n",
        tick,
        tasks[0].name, tasks[0].cur_prio, st(tasks[0].state),
        tasks[1].name, tasks[1].cur_prio, st(tasks[1].state),
        tasks[2].name, tasks[2].cur_prio, st(tasks[2].state),
        runner>=0 ? tasks[runner].name : "(none)");
}
