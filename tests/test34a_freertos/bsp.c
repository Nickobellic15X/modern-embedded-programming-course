/* Board Support Package (BSP) for FreeRTOS
* and the EK-TM4C123GXL board
*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "bsp.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

/* on-board switch */
#define BTN_SW1   (1U << 4)


/* Hooks ===================================================================*/
/* Application hooks used in this project ==================================*/
/* NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook */
void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    Event const *ep;

    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    if (BSP_timout_blinky != 0U) { /* timeout armed? */
        if (--BSP_timout_blinky == 0U) { /* just expiring? */
            static Event const timeoutEvt = {TIMEOUT_SIG};
            ep = &timeoutEvt;
            xQueueSendFromISR(BSP_queue_blinky, &ep,
                              &xHigherPriorityTaskWoken);
        }
    }

    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    current = ~GPIOF_AHB->DATA_Bits[BTN_SW1]; /* read SW1 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & BTN_SW1) != 0U) {  /* debounced SW1 state changed? */
        if ((buttons.depressed & BTN_SW1) != 0U) { /* is SW1 depressed? */
            /* post the "button-pressed" event from ISR */
            static Event const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            ep = &buttonPressedEvt;
            xQueueSendFromISR(BSP_queue_button, &ep,
                              &xHigherPriorityTaskWoken);
        }
        else { /* the button is released */
             /* post the "button-released" event from ISR */
             static Event const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
             ep = &buttonReleasedEvt;
             xQueueSendFromISR(BSP_queue_button, &ep,
                               &xHigherPriorityTaskWoken);
        }
    }

    /* notify FreeRTOS to perform context switch from ISR, if needed */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
/*..........................................................................*/
void vApplicationIdleHook(void) {
#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    __WFI(); /* Wait-For-Interrupt */
#endif
}
/*..........................................................................*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* ERROR!!! */
}
/*..........................................................................*/
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
* provide an implementation of vApplicationGetIdleTaskMemory() to provide
* the memory that is used by the Idle task.
*/
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside
    * this function then they must be declared static - otherwise they will
    * be allocated on the stack and so not exists after this function exits.
    */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the
    * Idle task's state will be stored.
    */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configMINIMAL_STACK_SIZE is specified in words, not bytes.
    */
    *pulIdleTaskStackSize = sizeof(uxIdleTaskStack)/sizeof(uxIdleTaskStack[0]);
}

/* BSP functions ===========================================================*/
void BSP_init(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* configure switch SW1 */
    GPIOF_AHB->DIR &= ~BTN_SW1; /* input */
    GPIOF_AHB->DEN |= BTN_SW1; /* digital enable */
    GPIOF_AHB->PUR |= BTN_SW1; /* pull-up resistor enable */

}

/*..........................................................................*/
void BSP_start(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    /* SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); done in FreeRTOS */

    /* assign all priority bits for preemption-prio. and none to sub-prio. */
    NVIC_SetPriorityGrouping(0U);

    /* set priorities of ALL ISRs used in the system, see NOTE1 */
    NVIC_SetPriority(SysTick_IRQn, 1U + configMAX_SYSCALL_INTERRUPT_PRIORITY);
    /* ... */

    /* enable IRQs... */
    /* ... */
}

void BSP_ledRedOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}

void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}
/*..........................................................................*/
/* error-handling function called by exception handlers in the startup code */
void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
#ifndef NDEBUG /* debug build? */
    while (loc != 0) { /* tie the CPU in this endless loop */
    }
#endif
    NVIC_SystemReset(); /* reset the CPU */
}

/*****************************************************************************
* NOTE1:
* Only ISRs prioritized at or below the
* configMAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
* with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
* QP/FreeRTOS services. These ISRs are "kernel-aware".
*
* Only ISRs prioritized at or below the configMAX_SYSCALL_INTERRUPT_PRIORITY
* level (i.e., with the numerical values of priorities equal or higher than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any QF services.
* These ISRs are "kernel-aware".
*
* Conversely, any ISRs prioritized above the
* configMAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
* the numerical values of priorities less than
* configMAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
* not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
* QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
* can communicate with the QF framework is by triggering a "kernel-aware"
* ISR, which can post/publish events.
*
* For more information, see article "Running the RTOS on a ARM Cortex-M Core"
* http://www.freertos.org/RTOS-Cortex-M3-M4.html
*/

