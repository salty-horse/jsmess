/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Philips MEA 8000 emulation.

**********************************************************************/

#ifndef MEA8000
#define MEA8000

extern void mea8000_config ( running_machine *machine, int channel, write8_machine_func req_out_func );

/* reset by external signal */
extern void mea8000_reset ( running_machine *machine );

/* interface to CPU via address/data bus*/
extern READ8_HANDLER  ( mea8000_r );
extern WRITE8_HANDLER ( mea8000_w );

#endif
