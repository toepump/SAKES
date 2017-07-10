#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_rpmsg.h>
#include "resource_table.h"


volatile register unsigned __R31;

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

/*Declare the bit of channel A, B and Z*/
#define CHECK_BIT_A	0x0001
#define CHECK_BIT_B	0x0004
#define CHECK_BIT_Z	0x0016


unsigned char payload[RPMSG_BUF_SIZE];

/*
 * main.c
 */
void main(void)
{
	struct pru_rpmsg_transport transport;
	unsigned short src, dst, len;
	volatile unsigned char *status;
	/*The previous state of channel A, B and Z*/
	uint32_t prev_state_A;
	uint32_t prev_state_B;
	uint32_t prev_state_Z;
	uint32_t encoder_state;
	uint32_t angle;


	/* Allow OCP master port access by the PRU so the PRU can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Make sure the Linux drivers are ready for RPMsg communication */
	/* this is another place where a hang could occur */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

	/* Initialize the RPMsg transport structure */
	/* this function is devined in rpmsg_pru.c.  It's sole purpose is to call pru_virtqueue_init twice (once for 
           vring0 and once for vring1).  pru_virtqueue_init is defined in pru_virtqueue.c.  It's sole purpose is to 
           call vring_init.  Not sure yet where that's defined, but it appears to be part of the pru_rpmsg iface.*/
	/* should probably test for RPMSG_SUCCESS.  If not, then the interface is not working and code should halt */
	pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

	/* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
	// In a real-time environment, rather than waiting forever, this can probably be run loop-after-loop
	// until success is achieved.  At that point, set a flag and then enable the send/receive functionality
	angle=12000;
 
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);
	while (1){

		if (__R31 & HOST_INT){
	        
			/*Send message if one received from encoder */
			if (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS){
				pru_rpmsg_send(&transport,dst, src, &angle, sizeof(int));
			}else{
			  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
			}

			if ((__R31 ^ prev_state_A) & CHECK_BIT_A) {
				prev_state_A = __R31 & CHECK_BIT_A;
				angle=angle+1;
			}

		}
	}
}
