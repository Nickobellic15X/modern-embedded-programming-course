/*****************************************************************************
* BSP for EK-TM4C123GXL with uC/OS-II RTOS
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

void BSP_init(void);
void BSP_start(void);

void BSP_ledRedOn(void);
void BSP_ledRedOff(void);
void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);
void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

#define BSP_ASSERT(check_)                 \
    if (!(check_)) {                       \
        Q_onAssert(this_module, __LINE__); \
    } else (void)0

void Q_onAssert(char const *module, int loc);

/*---------------------------------------------------------------------------*/
/* Event facilities... */

typedef uint16_t Signal; /* event signal */

/* Event base class */
typedef struct {
    Signal sig; /* event signal */
    /* event parameters added in subclasses of Event */
} Event;

enum EventSignals {
    TIMEOUT_SIG,
    BUTTON_PRESSED_SIG,
    BUTTON_RELEASED_SIG,
};

/* Event queues... */
extern OS_EVENT *BSP_queue_button; /* global queue handle */

#endif /* BSP_H */
