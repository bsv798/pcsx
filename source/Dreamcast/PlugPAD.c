#include <kos.h>
#include "plugins.h"

uint8 c;
long  PadFlags = 0;

long PAD__init(long flags) {
//	SysPrintf("start PAD_init()\r\n");

//	printf("Flags: %08x\n", flags);
	PadFlags |= flags;

	/* Read Configuration here */

//	SysPrintf("end PAD_init()\r\n");
	
	return 0;
}

long PAD__shutdown(void) {
	return 0;
}

long PAD__open(void)
{
//	SysPrintf("start PAD1_open()\r\n");
	c = maple_first_controller();
//	SysPrintf("end PAD1_open()\r\n");
	return 0;
}

long PAD__close(void) {
	return 0;
}

long PAD__readPort1(PadDataS* pad) {
//	SysPrintf("start PAD1_readPort()\r\n");
	cont_cond_t cond;
	uint16 pad_status = 0xffff;
	 
	if (cont_get_cond(c, &cond) < 0) {
		printf("Error reading controller\n");
	}
	if (!(cond.buttons & CONT_START))
		pad_status &= ~(1<<3);
	if (!(cond.buttons & CONT_A))
		pad_status &= ~(1<<14);
	if (!(cond.buttons & CONT_B))
		pad_status &= ~(1<<13);
	if (!(cond.buttons & CONT_X))
		pad_status &= ~(1<<15);
	if (!(cond.buttons & CONT_Y))
		pad_status &= ~(1<<12);
	if (!(cond.buttons & CONT_DPAD_UP))
		pad_status &= ~(1<<4);
	if (!(cond.buttons & CONT_DPAD_DOWN))
		pad_status &= ~(1<<6);
	if (!(cond.buttons & CONT_DPAD_LEFT))
		pad_status &= ~(1<<7);
	if (!(cond.buttons & CONT_DPAD_RIGHT))
		pad_status &= ~(1<<5);
			
	pad->buttonStatus = pad_status;
	pad->controllerType = 4; // standard
//	printf("Pad1 Status: %04x\n", pad_status);
//	SysPrintf("end PAD1_readPort()\r\n");
	return 0;
}

long PAD__readPort2(PadDataS*a) {
	return -1;
}
