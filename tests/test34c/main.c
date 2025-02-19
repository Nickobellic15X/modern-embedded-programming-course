/* Blinky/Button with uC/OS-II RTOS */
#include "uc_ao.h"  /* uC/AO API */
#include "bsp.h"
#include <stdbool.h>

static char const this_module[] = "main"; /* this module name for Q_ASSERT() */

/* The Blinky thread =========================================================*/
OS_STK stack_blinky[100]; /* task stack */

enum { INITIAL_BLINK_TIME = (OS_TICKS_PER_SEC / 4) };

/* data shared between tasks */
INT32U volatile shared_blink_time = INITIAL_BLINK_TIME;
OS_EVENT *shared_blink_time_mutex;

typedef struct {
    Active super; /* inherit Active base class */
    /* add private data for the AO... */
    TimeEvent te;
    bool isLedOn;
} Blinky;
static void Blinky_dispatch(Blinky * const me, Event const * const e) {
    switch (e->sig) {
        case INIT_SIG: /* intentionally fall through... */
        case TIMEOUT_SIG: {
            INT8U err;
            INT32U bt; /* local copy of shared_blink_time */

            OSMutexPend(shared_blink_time_mutex, 0, &err); /* mutual exclusion */
            Q_ASSERT(err == 0U);
            bt = shared_blink_time;
            OSMutexPost(shared_blink_time_mutex); /* mutual exclusion */


            if (!me->isLedOn) { /* LED not on */
                BSP_ledGreenOn();
                me->isLedOn = true;
                TimeEvent_arm(&me->te, bt, 0U);
            }
            else {  /* LED is on */
                BSP_ledGreenOff();
                me->isLedOn = false;
                TimeEvent_arm(&me->te, bt * 3U, 0U);
            }
            break;
        }
        default: {
            break;
        }
    }
}
void Blinky_ctor(Blinky * const me) {
    Active_ctor(&me->super, (DispatchHandler)&Blinky_dispatch);
    TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
    me->isLedOn = false;
}
static Event *blinky_queue[10];
static Blinky blinky;
Active *AO_Blinky = &blinky.super;

/* The Button AO ============================================================*/
typedef struct {
    Active super; /* inherit Active base class */
    /* add private data for the AO... */
} Button;

static void Button_dispatch(Button * const me, Event const * const e) {
    switch (e->sig) {
        case INIT_SIG: {
            BSP_ledBlueOff();
            break;
        }
        case BUTTON_PRESSED_SIG: {
            INT8U err; /* uC/OS-II error status */

            BSP_ledBlueOn();

            /* update the blink time for the 'blink' thread */
            OSMutexPend(shared_blink_time_mutex, 0, &err); /* mutual exclusion */
            BSP_ASSERT(err == 0);
            shared_blink_time >>= 1; /* shorten the blink time by factor of 2 */
            if (shared_blink_time == 0U) {
                shared_blink_time = INITIAL_BLINK_TIME;
            }
            OSMutexPost(shared_blink_time_mutex); /* mutual exclusion */
            break;
        }
        case BUTTON_RELEASED_SIG: {
            BSP_ledBlueOff();
            break;
        }
        default: {
            break;
        }
    }
}

void Button_ctor(Button * const me) {
    Active_ctor(&me->super, (DispatchHandler)&Button_dispatch);
}

OS_STK stack_button[100]; /* task stack */
static Event *button_queue[10];
static Button button;
Active *AO_Button = &button.super;


/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* initialize the RTOS objects before using them */
    shared_blink_time_mutex = OSMutexCreate(OS_LOWEST_PRIO - 5, &err);
    Q_ASSERT(err == 0U);

    /* create AO and start it */
    Blinky_ctor(&blinky);
    Active_start(AO_Blinky,
                 2U,
                 blinky_queue,
                 sizeof(blinky_queue)/sizeof(blinky_queue[0]),
                 stack_blinky,
                 sizeof(stack_blinky),
                 0U);

    /* create AO and start it */
    Button_ctor(&button);
    Active_start(AO_Button,
                 1U,
                 button_queue,
                 sizeof(button_queue)/sizeof(button_queue[0]),
                 stack_button,
                 sizeof(stack_button),
                 0U);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}

/*******************************************************************************
* NOTE1:
* The call to uC/OS-II API OSTaskCreateExt() assumes that the pointer to the
* top-of-stack (ptos) is at the end of the provided stack memory. This is
* correct only for CPUs with downward-growing stack, but must be changed for
* CPUs with upward-growing stack.
*/
