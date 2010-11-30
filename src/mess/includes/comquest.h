/*****************************************************************************
 *
 * includes/comquest.h
 *
 ****************************************************************************/

#ifndef COMQUEST_H_
#define COMQUEST_H_


class comquest_state : public driver_device
{
public:
	comquest_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 data[128][8];
	void *timer;
	int line;
	int dma_activ;
	int state;
	int count;
};


/*----------- defined in video/comquest.c -----------*/

VIDEO_START( comquest );
VIDEO_UPDATE( comquest );


#endif /* COMQUEST_H_ */
