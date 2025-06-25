You should program the hardware block design using the bitstream on vivado and uncheck the
"Reset entire system" checkbox at the launch.json!! 

My linker script threw an error whenever i had more than one .c file at the vitis src folder so
i only added main.c and then copied the contents of the task1.c and task2.c to test it out
