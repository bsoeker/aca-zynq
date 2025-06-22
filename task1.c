#define __MICROBLAZE__
#include "xgpio.h"
#include "xparameters.h"
#include "xtmrctr.h"
#include <stdio.h>

#define GPIO_DEVICE_ID 0
#define TIMER_DEVICE_ID 0

#define BTN_MASK 0x1           // BTN0 = bit 0
#define LED_RED_MASK (1 << 16) // LD5 Red = bit 16
#define DEBOUNCE_DELAY 1000000

XGpio gpio;
XTmrCtr timer;

int getButtonPressed() {
    // Read BTN0 (bit 0)
    return (XGpio_DiscreteRead(&gpio, 1) & BTN_MASK);
}

void delay_ms(int ms) {
    // Uses AXI Timer to wait approximately 'ms' milliseconds
    XTmrCtr_Reset(&timer, 0);
    XTmrCtr_Start(&timer, 0);
    while ((XTmrCtr_GetValue(&timer, 0)) <
           (u32)(XPAR_CPU_CORE_CLOCK_FREQ_HZ / 1000) * ms)
        ;
    XTmrCtr_Stop(&timer, 0);
}

int main() {
    int status;
    int duty_step = 0; // 0 to 4 (0%, 25%, ..., 100%)

    xil_printf("Initializing peripherals...\n");

    status = XGpio_Initialize(&gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO init failed\n");
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&gpio, 1, BTN_MASK); // bit 0 input, rest output

    status = XTmrCtr_Initialize(&timer, TIMER_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Timer init failed\n");
        return XST_FAILURE;
    }

    xil_printf("Starting PWM loop...\n");

    while (1) {
        // Check button press
        if (getButtonPressed()) {
            duty_step = (duty_step + 1) % 5; // Cycle through 0–4
            xil_printf("Duty cycle changed to %d%%\n", duty_step * 25);
            delay_ms(200); // Basic debouncing
        }

        int on_time = duty_step * 25; // in ms
        int off_time = 100 - on_time; // in ms

        // PWM ON period
        if (on_time > 0) {
            XGpio_DiscreteWrite(&gpio, 1, LED_RED_MASK);
            delay_ms(on_time);
        }

        // PWM OFF period
        if (off_time > 0) {
            XGpio_DiscreteWrite(&gpio, 1, 0);
            delay_ms(off_time);
        }
    }

    return 0;
}
