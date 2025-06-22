#define __MICROBLAZE__
#include "xgpio.h"
#include "xparameters.h"
#include <stdio.h>

#define GPIO_DEVICE_ID 0

#define BTN_MASK 0x1         // BTN0
#define LED_RED_MASK 0x10000 // LD5 Red = bit 16

int main() {
    XGpio gpio;
    int status;

    status = XGpio_Initialize(&gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO init failed\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(&gpio, 1,
                           0x00000001); // bit 0 = input, rest = output

    xil_printf("Monitoring BTN0...\n");

    while (1) {
        u32 btn = XGpio_DiscreteRead(&gpio, 1);
        if (btn & BTN_MASK) {
            xil_printf("BTN0 pressed! Lighting LD5 RED\n");
            XGpio_DiscreteWrite(&gpio, 1, LED_RED_MASK);
        } else {
            XGpio_DiscreteWrite(&gpio, 1, 0x00000000);
        }
    }

    return 0;
}
