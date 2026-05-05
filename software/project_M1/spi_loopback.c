#include <stdio.h>
#include <includes.h>
#include <stdint.h>
#include "../project_M1_bsp/system.h"
#include "../project_M1_bsp/HAL/inc/io.h"

// definition of task stacks
#define TASK_STACKSIZE 1024
OS_STK	task1_stk[TASK_STACKSIZE];

// pin and base address definitions'
#define LED9 (1 << 9)
//*OutPort_LEDR |= LED9;       // ON
//*OutPort_LEDR &= ~LED9;      // OFF
volatile int * OutPort_LEDR = (int*) 0x04009090; // ledr
volatile int * SPI_RxData = (int*) 0x04009000; // receive data
volatile int * SPI_TxData = (int*) 0x04009004; // transmit data
volatile int * SPI_Status = (int*) 0x04009008; // status
volatile int * SPI_Control = (int*) 0x0400900C; // control

volatile int * SPI_SS = (int*) 0x04009040; // SPI SS from GPIO

// definitions of task priorities
#define TASK1_PRIORITY

#define TRDY 0x40 // masking
#define RRDY 0x80

// task 1
void task1(void* pdata) {
	// data to be transmitted
	uint8_t tx = 0x55;
	uint8_t rx = 0x0;

	// keep slave select off (active low)
	*SPI_SS = 0x1;
	printf("START SPI\n");

	// turn led off - flag
	*OutPort_LEDR &= ~LED9;

	while (1) {
		// select spi slave
		*SPI_SS = 0x0;
		for (volatile int i = 0; i < 1000; i++); // small delay

		while (!(*SPI_Status & TRDY));
		*SPI_TxData = tx;

		// wait for rx
		while (!(*SPI_Status & RRDY));
		rx = *SPI_RxData;

		printf("Received: 0x%02X\n", rx);

		// deselect spi slave
		*SPI_SS = 0x1;

		// turn led9 off
		*OutPort_LEDR &= ~LED9;
		// delay 2 second
		OSTimeDlyHMSM(0, 0, 2, 0);
		// turn led9 on
		*OutPort_LEDR |= LED9;

		// increment data to be transmitted
		tx++;

		OSTimeDlyHMSM(0, 0, 2, 0); // delay 2 seconds
	}
}

int spi_main(void) {
	printf("=====================\n");

	OSInit(); // initialise uC/OS-II

	OSTaskCreateExt(task1, // task function
					NULL, // task argument
					(void *)&task1_stk[TASK_STACKSIZE-1], // top of stack
					TASK1_PRIORITY, // priority
					TASK1_PRIORITY, // task id
					task1_stk, // bottom of stack
					TASK_STACKSIZE, // stack size
					NULL, // no extended data
					0); // task options

	OSStart(); // start the scheduler

	return 0;
}
