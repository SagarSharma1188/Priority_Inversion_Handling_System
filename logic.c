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
int tick_step(int tick, int use_pip, int use_pcp, int* inherited) {

    /* Arrivals */
    if(tick == tasks[ID_HIGH].arrive_tick) {
        tasks[ID_HIGH].state = READY;
        printf("         >>> HIGH_Task ARRIVED (prio=%d, needs mutex)\n", PRIO_HIGH);
    }
    if(tick == tasks[ID_MED].arrive_tick) {
        tasks[ID_MED].state = READY;
        printf("         >>> MED_Task ARRIVED  (prio=%d, no mutex)\n", PRIO_MED);
    }

    /* HIGH tries to grab mutex */
    if(tasks[ID_HIGH].state == READY) {
        if(mutex_owner == -1) {
            mutex_owner = ID_HIGH;
            tasks[ID_HIGH].holding_mutex = TRUE;
            if(use_pcp) tasks[ID_HIGH].cur_prio = MUTEX_CEILING;
            tasks[ID_HIGH].state = RUNNING;
            printf("         >>> HIGH_Task acquired mutex%s\n",
                   use_pcp ? " [PCP: boosted to ceiling=1]" : "");
        } else {
            /* Must block */
            tasks[ID_HIGH].state = BLOCKED;
            printf("         *** HIGH_Task BLOCKED — mutex held by LOW_Task\n");

            if(use_pip && !(*inherited)) {
                tasks[ID_LOW].cur_prio = PRIO_HIGH;
                *inherited = TRUE;
                printf("         [PIP] LOW_Task inherits priority %d → prevents MED preemption!\n",
                       PRIO_HIGH);
            }
            if(!use_pip && !use_pcp) {
                printf("         !!! PRIORITY INVERSION: MED may preempt LOW while HIGH waits!\n");
            }
        }
    }

    int runner = pick_runner();
    if(runner == -1) return 0;

    /* Set states */
    for(int i=0;i<MAX_TASKS;i++)
        if(i!=runner && tasks[i].state==RUNNING) tasks[i].state=READY;
    tasks[runner].state = RUNNING;

    print_state(tick, runner);

    /* Advance */
    tasks[runner].done_ticks++;

    /* Mutex release by LOW */
    if(runner==ID_LOW && tasks[ID_LOW].holding_mutex) {
        tasks[ID_LOW].mutex_done++;
        if(tasks[ID_LOW].mutex_done >= tasks[ID_LOW].mutex_ticks) {
            mutex_owner = -1;
            tasks[ID_LOW].holding_mutex = FALSE;
            tasks[ID_LOW].cur_prio = PRIO_LOW;
            printf("         >>> LOW_Task released mutex (priority restored to %d)\n", PRIO_LOW);
            if(tasks[ID_HIGH].state==BLOCKED) {
                tasks[ID_HIGH].state = READY;
                printf("         >>> HIGH_Task UNBLOCKED\n");
            }
        }
    }
    /* Mutex release by HIGH */
    if(runner==ID_HIGH && tasks[ID_HIGH].holding_mutex) {
        tasks[ID_HIGH].mutex_done++;
        if(tasks[ID_HIGH].mutex_done >= tasks[ID_HIGH].mutex_ticks) {
            mutex_owner = -1;
            tasks[ID_HIGH].holding_mutex = FALSE;
            tasks[ID_HIGH].cur_prio = PRIO_HIGH;
        }
    }

    /* Finish */
    if(tasks[runner].done_ticks >= tasks[runner].total_ticks) {
        tasks[runner].state = FINISHED;
        printf("         >>> %s FINISHED\n", tasks[runner].name);
    }
    return 1;
}
void scenario1(void) {
    sep();
    printf("  SCENARIO 1: No Solution — Priority Inversion Demo\n");
    sub_sep();
    printf("  t=0 : LOW_Task starts, acquires mutex\n");
    printf("  t=2 : HIGH_Task arrives, needs mutex\n");
    printf("  t=3 : MED_Task arrives, no mutex needed\n");
    printf("  Expected: MED preempts LOW while HIGH waits (INVERSION)\n");
    sep();
    reset();
    int dummy = FALSE;
    for(int t=0; t<=25 && !all_done(); t++) tick_step(t,FALSE,FALSE,&dummy);
    printf("\n  OUTCOME: HIGH_Task was delayed by MED_Task — inversion occurred.\n");
    sep(); printf("\n");
}
void scenario2(void) {
    sep();
    printf("  SCENARIO 2: Priority Inheritance Protocol (PIP)\n");
    sub_sep();
    printf("  When HIGH blocks, LOW inherits HIGH's priority (=1)\n");
    printf("  MED (prio=5) cannot preempt LOW at prio=1\n");
    sep();
    reset();
    int inherited = FALSE;
    for(int t=0; t<=25 && !all_done(); t++) tick_step(t,TRUE,FALSE,&inherited);
    printf("\n  OUTCOME: Inversion prevented. HIGH ran before MED.\n");
    sep(); printf("\n");
}

void scenario3(void) {
    sep();
    printf("  SCENARIO 3: Priority Ceiling Protocol (PCP)\n");
    sub_sep();
    printf("  Mutex ceiling = %d. Any task acquiring mutex is\n", MUTEX_CEILING);
    printf("  immediately boosted to ceiling — proactive prevention.\n");
    sep();
    reset();
    /* PCP boost at acquisition time (t=0) */
    tasks[ID_LOW].cur_prio = MUTEX_CEILING;
    printf("  [T=00] PCP applied: LOW_Task acquired mutex → boosted to ceiling=%d\n\n",
           MUTEX_CEILING);
    int dummy = FALSE;
    for(int t=0; t<=25 && !all_done(); t++) tick_step(t,FALSE,TRUE,&dummy);
    printf("\n  OUTCOME: Inversion impossible. LOW was already at ceiling priority.\n");
    sep(); printf("\n");
}

void banner(void) {
    sep();
    printf("      PRIORITY INVERSION HANDLING SYSTEM\n");
    printf("      BTech 2nd Year | Real-Time OS Simulation\n");
    sub_sep();
    printf("  Tasks:  HIGH(prio=1)  MED(prio=5)  LOW(prio=10)\n");
    printf("  Mutex:  shared resource with ceiling=%d\n", MUTEX_CEILING);
    printf("  LOW acquires mutex at t=0, HIGH arrives at t=2\n");
    sep(); printf("\n");
}

int main(void) {
    banner();
    int ch = 0;
    do {
        printf("  1. No Solution (see inversion)\n");
        printf("  2. Priority Inheritance Protocol\n");
        printf("  3. Priority Ceiling Protocol\n");
        printf("  4. Run All Three\n");
        printf("  5. Exit\n");
        printf("  Choice: ");
        scanf("%d",&ch);
        printf("\n");
        switch(ch) {
            case 1: scenario1(); break;
            case 2: scenario2(); break;
            case 3: scenario3(); break;
            case 4: scenario1(); scenario2(); scenario3();
                    printf("  All 3 scenarios complete. Compare outputs above.\n\n"); break;
            case 5: printf("  Goodbye!\n\n"); break;
            default: printf("  Invalid.\n\n");
        }
    } while(ch!=5);
    return 0;
}
