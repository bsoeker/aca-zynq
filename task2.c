#define __MICROBLAZE__

#include "xgpio.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xtmrctr.h"

#define GPIO_DEVICE_ID 0
#define TIMER_DEVICE_ID 0

#define BTN_MASK 0x1
#define LED_MASK (1 << 16)
#define PWM_RESOLUTION 100 // number of ticks per full PWM cycle
#define TIMER_FREQ_HZ 1000 // 1 kHz = 1 ms tick

XGpio Gpio;
XTmrCtr Timer;
volatile int duty_step = 0; // 0 to 4 → 0% to 100%
volatile int tick_counter = 0;

void TimerISR(void* CallBackRef, u8 TmrCtrNumber) {
    static int btn_debounce = 0;

    // Debounce every 200ms
    if (++btn_debounce >= 200) {
        btn_debounce = 0;
        if (XGpio_DiscreteRead(&Gpio, 1) & BTN_MASK) {
            duty_step = (duty_step + 1) % 5;
            xil_printf("Duty step = %d (=> %d%%)\n", duty_step, duty_step * 25);
        }
    }

    int duty_ticks = duty_step * (PWM_RESOLUTION / 4); // 0, 25, 50, 75, 100
    if (tick_counter < duty_ticks)
        XGpio_DiscreteWrite(&Gpio, 1, LED_MASK);
    else
        XGpio_DiscreteWrite(&Gpio, 1, 0);

    tick_counter = (tick_counter + 1) % PWM_RESOLUTION;
    XTmrCtr_Reset(&Timer, 0); // clear interrupt
}

int main() {
    int status;

    xil_printf("Initializing...\n");

    status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO init failed\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&Gpio, 1, BTN_MASK);

    status = XTmrCtr_Initialize(&Timer, TIMER_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Timer init failed\n");
        return XST_FAILURE;
    }

    XTmrCtr_SetHandler(&Timer, TimerISR, &Timer);
    XTmrCtr_SetOptions(&Timer, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
    XTmrCtr_SetResetValue(
        &Timer, 0,
        0xFFFFFFFF - (XPAR_CPU_CORE_CLOCK_FREQ_HZ / TIMER_FREQ_HZ)); // 1 kHz

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
                                 &Timer);
    Xil_ExceptionEnable();

    XTmrCtr_Start(&Timer, 0);
    xil_printf("PWM with flickering started (interrupt-driven)...\n");

    while (1) {
        // All logic handled by ISR
    }

    return 0;
}
