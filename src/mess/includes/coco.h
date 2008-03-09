/*****************************************************************************
 *
 * includes/coco.h
 * 
 * CoCo/Dragon
 *
 ****************************************************************************/

#ifndef COCO_H_
#define COCO_H_

#include "devices/snapquik.h"


#define COCO_CPU_SPEED_HZ		894886	/* 0.894886 MHz */
#define COCO_FRAMES_PER_SECOND	(COCO_CPU_SPEED_HZ / 57.0 / 263)
#define COCO_CPU_SPEED			(ATTOTIME_IN_HZ(COCO_CPU_SPEED_HZ))
#define COCO_TIMER_CMPCARRIER	(COCO_CPU_SPEED * 0.25)


/*----------- defined in video/coco.c -----------*/

ATTR_CONST UINT8 coco_get_attributes(UINT8 c);

VIDEO_START( dragon );
VIDEO_START( coco );
VIDEO_START( coco2b );


/*----------- defined in video/coco3.c -----------*/

VIDEO_START( coco3 );
VIDEO_START( coco3p );
VIDEO_UPDATE( coco3 );
WRITE8_HANDLER ( coco3_palette_w );
UINT32 coco3_get_video_base(UINT8 ff9d_mask, UINT8 ff9e_mask);
void coco3_vh_blink(void);


/*----------- defined in machine/coco.c -----------*/

extern UINT8 coco3_gimereg[16];

MACHINE_START( dragon32 );
MACHINE_START( dragon64 );
MACHINE_START( tanodr64 );
MACHINE_START( dgnalpha );
MACHINE_START( coco );
MACHINE_START( coco2 );
MACHINE_START( coco3 );
MACHINE_RESET( coco3 );

DEVICE_LOAD(coco_rom);
DEVICE_LOAD(coco3_rom);
DEVICE_UNLOAD(coco_rom);
DEVICE_UNLOAD(coco3_rom);

INPUT_CHANGED(coco_keyboard_changed);

SNAPSHOT_LOAD ( coco_pak );
SNAPSHOT_LOAD ( coco3_pak );
READ8_HANDLER ( coco3_mmu_r );
WRITE8_HANDLER ( coco3_mmu_w );
READ8_HANDLER ( coco3_gime_r );
WRITE8_HANDLER ( coco3_gime_w );
READ8_HANDLER ( coco_cartridge_r);
WRITE8_HANDLER ( coco_cartridge_w );
READ8_HANDLER ( coco3_cartridge_r);
WRITE8_HANDLER ( coco3_cartridge_w );
offs_t coco3_mmu_translate(int bank, int offset);
WRITE8_HANDLER( coco_pia_1_w );
void coco3_horizontal_sync_callback(int data);
void coco3_field_sync_callback(int data);
void coco3_gime_field_sync_callback(void);

/* Compusense Dragon Plus board */
READ8_HANDLER ( plus_reg_r );
WRITE8_HANDLER ( plus_reg_w );

READ8_HANDLER( dragon_alpha_mapped_irq_r );

/* Dragon Alpha AY-8912 */
READ8_HANDLER ( dgnalpha_psg_porta_read );
WRITE8_HANDLER ( dgnalpha_psg_porta_write );

/* Dragon Alpha WD2797 FDC */
READ8_HANDLER(wd2797_r);
WRITE8_HANDLER(wd2797_w);

void coco_set_halt_line(int halt_line);

#define IO_BITBANGER IO_PRINTER
#define IO_VHD IO_HARDDISK

/* CoCo 3 video vars; controlling key aspects of the emulation */
struct coco3_video_vars
{
	UINT8 bordertop_192;
	UINT8 bordertop_199;
	UINT8 bordertop_0;
	UINT8 bordertop_225;
	unsigned int hs_gime_flip : 1;
	unsigned int fs_gime_flip : 1;
	unsigned int hs_pia_flip : 1;
	unsigned int fs_pia_flip : 1;
	UINT16 rise_scanline;
	UINT16 fall_scanline;
};

extern const struct coco3_video_vars coco3_vidvars;


#endif /* COCO_H_ */
