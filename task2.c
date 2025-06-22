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
#define PWM_PERIOD_MS 100

XGpio Gpio;
XTmrCtr Timer;
volatile int duty_step = 0;

void TimerISR(void* CallBackRef, u8 TmrCtrNumber) {
    static int counter = 0;

    // Handle button press
    if (XGpio_DiscreteRead(&Gpio, 1) & BTN_MASK) {
        duty_step = (duty_step + 1) % 5;
        xil_printf("Duty step = %d (=> %d%%)\n", duty_step, duty_step * 25);
        for (volatile int i = 0; i < 1000000; i++) {
        } // Debounce delay
    }

    // Set LED state based on duty cycle
    int on_time_ticks =
        duty_step * (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 1000) * PWM_PERIOD_MS / 100;
    int total_ticks = (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 1000) * PWM_PERIOD_MS;

    if (counter < on_time_ticks)
        XGpio_DiscreteWrite(&Gpio, 1, LED_MASK);
    else
        XGpio_DiscreteWrite(&Gpio, 1, 0);

    counter = (counter + 1) % total_ticks;

    XTmrCtr_Reset(&Timer, 0);
}

int main() {
    int status;

    xil_printf("Initializing...\n");

    // GPIO
    status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO init failed\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&Gpio, 1, BTN_MASK); // bit 0 = input
    // All other bits = output by default (0)

    // Timer
    status = XTmrCtr_Initialize(&Timer, TIMER_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Timer init failed\n");
        return XST_FAILURE;
    }

    XTmrCtr_SetHandler(&Timer, TimerISR, &Timer);
    XTmrCtr_SetOptions(&Timer, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
    XTmrCtr_SetResetValue(
        &Timer, 0,
        0xFFFFFFFF - (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 10)); // 10Hz = 100ms

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
                                 &Timer);
    Xil_ExceptionEnable();

    XTmrCtr_Start(&Timer, 0);
    xil_printf("PWM started (interrupt-driven)...\n");

    while (1) {
        // Idle; work is done in ISR
    }

    return 0;
}
