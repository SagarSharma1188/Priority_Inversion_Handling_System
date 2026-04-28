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
