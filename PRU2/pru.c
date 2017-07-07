#include "pru_cfg.h"

#define PRU_SHARED_MEM_ADDR 0x00012000

int main(void)
{
	// direct pointer to memory address
	volatile int* shared_mem = (volatile int *) PRU_SHARED_MEM_ADDR;

	int output=0;

	// enable OCP
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	//while(shared_mem[0]!=0xCAFEBABE) {}

	//shared_mem[1]=0xDEADBEEF;

	while(1) {

		shared_mem[0]=output;

		output=output+1;

		if(output==3000){
			output=0;
		}

	}

	__halt();

	return 0;
}
