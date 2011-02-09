/************************************************\
* Multitech Micro Professor 1                    *
*                                                *
*     CPU: Z80 @ 1.79 MHz                        *
*     ROM: 4-kilobyte ROM monitor                *
*     RAM: 4 kilobytes                           *
*   Input: Hex keypad                            *
* Storage: Cassette tape                         *
*   Video: 6x 7-segment LED display              *
*   Sound: Speaker                               *
\************************************************/

/*

    TODO:

    - remove halt callback
    - crt board
    - speech board
    - printers
    - clickable artwork

*/

#include "emu.h"
#include "includes/mpf1.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/i8255a.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "audio/spchroms.h"
#include "sound/speaker.h"
#include "sound/tms5220.h"
#include "imagedev/cassette.h"
#include "mpf1.lh"
#include "mpf1b.lh"
#include "mpf1p.lh"

/* Address Maps */

static ADDRESS_MAP_START( mpf1_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1b_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x5000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1p_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255a_r, i8255a_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_r, z80ctc_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_cd_ba_r, z80pio_cd_ba_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1b_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255a_r, i8255a_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_r, z80ctc_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_cd_ba_r, z80pio_cd_ba_w)
	AM_RANGE(0xfe, 0xfe) AM_MIRROR(0x01) AM_DEVREADWRITE(TMS5220_TAG, tms5220_status_r, tms5220_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mpf1p_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x3c) AM_DEVREADWRITE(I8255A_TAG, i8255a_r, i8255a_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_r, z80ctc_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x3c) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_cd_ba_r, z80pio_cd_ba_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_CHANGED( trigger_nmi )
{
	cputag_set_input_line(field->port->machine, Z80_TAG, INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_CHANGED( trigger_irq )
{
	cputag_set_input_line(field->port->machine, Z80_TAG, INPUT_LINE_IRQ0, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( mpf1 )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 HL") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 HL'") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B I*IF") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F *PNC'") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 DE") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 DE'") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A SP") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E SZ*H'") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 BC") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 BC'") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 IY") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D *PNC") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE RD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 AF") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 AF'") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 IX") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C SZ*H") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAPE WR") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBR") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADDR") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RELA") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SBR") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DATA") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MOVE") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("USER KEY") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED(trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED(trigger_irq, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( mpf1b )
	PORT_START("PC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 /") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 >") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B STOP") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F LET") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 *") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 <") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A CALL") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E INPUT") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 -") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 =") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 P") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D PRINT") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONT") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 +") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 * *") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 M") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C NEXT") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SAVE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("IF/\xE2\x88\xA7") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TO/\xE2\x86\x93") PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("THEN/\xE2\x88\xA8") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOTO") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET/\xE2\x89\x81") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOSUB") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FOR/\xE2\x86\x91") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LIST") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR/\xE2\x86\x92") PORT_CODE(KEYCODE_INSERT) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL/\xE2\x86\x90") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MONI") PORT_CODE(KEYCODE_M) PORT_CHANGED(trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INTR") PORT_CODE(KEYCODE_I) PORT_CHANGED(trigger_irq, 0)
INPUT_PORTS_END

/* Intel 8255A Interface */

static TIMER_CALLBACK( led_refresh )
{
	mpf1_state *state = machine->driver_data<mpf1_state>();

	if (BIT(state->lednum, 5)) output_set_digit_value(0, param);
	if (BIT(state->lednum, 4)) output_set_digit_value(1, param);
	if (BIT(state->lednum, 3)) output_set_digit_value(2, param);
	if (BIT(state->lednum, 2)) output_set_digit_value(3, param);
	if (BIT(state->lednum, 1)) output_set_digit_value(4, param);
	if (BIT(state->lednum, 0)) output_set_digit_value(5, param);
}

static READ8_DEVICE_HANDLER( mpf1_porta_r )
{
	mpf1_state *state = device->machine->driver_data<mpf1_state>();
	UINT8 data = 0x7f;

	/* bit 0 to 5, keyboard rows 0 to 5 */
	if (!BIT(state->lednum, 0)) data &= input_port_read(device->machine, "PC0");
	if (!BIT(state->lednum, 1)) data &= input_port_read(device->machine, "PC1");
	if (!BIT(state->lednum, 2)) data &= input_port_read(device->machine, "PC2");
	if (!BIT(state->lednum, 3)) data &= input_port_read(device->machine, "PC3");
	if (!BIT(state->lednum, 4)) data &= input_port_read(device->machine, "PC4");
	if (!BIT(state->lednum, 5)) data &= input_port_read(device->machine, "PC5");

	/* bit 6, user key */
	data &= input_port_read(device->machine, "SPECIAL") & 1 ? 0xff : 0xbf;

	/* bit 7, tape input */
	data |= (cassette_input(state->cassette) > 0 ? 1 : 0) << 7;

	return data;
}

static WRITE8_DEVICE_HANDLER( mpf1_portb_w )
{
	mpf1_state *state = device->machine->driver_data<mpf1_state>();

	/* swap bits around for the mame 7-segment emulation */
	UINT8 led_data = BITSWAP8(data, 6, 1, 2, 0, 7, 5, 4, 3);

	/* timer to update segments */
	state->led_refresh_timer->adjust(attotime::from_usec(70), led_data);
}

static WRITE8_DEVICE_HANDLER( mpf1_portc_w )
{
	mpf1_state *state = device->machine->driver_data<mpf1_state>();

	/* bits 0-5, led select and keyboard latch */
	state->lednum = data & 0x3f;
	state->led_refresh_timer->adjust(attotime::never);

	/* bit 6, monitor break control */
	state->_break = BIT(data, 6);

	if (state->_break)
	{
		state->m1 = 0;
		cputag_set_input_line(device->machine, Z80_TAG, INPUT_LINE_NMI, CLEAR_LINE);
	}

	/* bit 7, tape output, tone and led */
	set_led_status(device->machine, 0, !BIT(data, 7));
	speaker_level_w(state->speaker, BIT(data, 7));
	cassette_output(state->cassette, BIT(data, 7));
}

static I8255A_INTERFACE( ppi8255_intf )
{
	DEVCB_HANDLER(mpf1_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(mpf1_portb_w),
	DEVCB_HANDLER(mpf1_portc_w),
};

/* Z80CTC Interface */

static Z80CTC_INTERFACE( mpf1_ctc_intf )
{
	0,
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* Z80PIO Interface */

static Z80PIO_INTERFACE( mpf1_pio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* Z80 Daisy Chain */

static const z80_daisy_config mpf1_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};

/* Cassette Interface */

static const cassette_config mpf1_cassette_config =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED),
	NULL
};

/* TMS5220 Interface */

static const tms5220_interface mpf1_tms5220_intf =
{
	DEVCB_NULL,					/* no IRQ callback */
	DEVCB_NULL,					/* no Ready callback */
#if 1
	spchroms_read,				/* speech ROM read handler */
	spchroms_load_address,		/* speech ROM load address handler */
	spchroms_read_and_branch	/* speech ROM read and branch handler */
#endif
};

/* Machine Initialization */

static TIMER_DEVICE_CALLBACK( check_halt_callback )
{
	// halt-LED; the red one, is turned on when the processor is halted
	// TODO: processor seems to halt, but restarts(?) at 0x0000 after a while -> fix
	INT64 led_halt = cpu_get_reg(timer.machine->device(Z80_TAG), Z80_HALT);
	set_led_status(timer.machine, 1, led_halt);
}

static MACHINE_START( mpf1 )
{
	mpf1_state *state = machine->driver_data<mpf1_state>();

	/* find devices */
	state->speaker = machine->device(SPEAKER_TAG);
	state->cassette = machine->device(CASSETTE_TAG);

	state->led_refresh_timer = machine->scheduler().timer_alloc(FUNC(led_refresh));

	/* register for state saving */
	state->save_item(NAME(state->_break));
	state->save_item(NAME(state->m1));
	state->save_item(NAME(state->lednum));
}

static MACHINE_RESET( mpf1 )
{
	mpf1_state *state = machine->driver_data<mpf1_state>();

	state->lednum = 0;
}

/* Machine Drivers */

static MACHINE_CONFIG_START( mpf1, mpf1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(mpf1_map)
	MCFG_CPU_IO_MAP(mpf1_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	MCFG_MACHINE_START(mpf1)
	MCFG_MACHINE_RESET(mpf1)

	/* devices */
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_3_579545MHz/2, mpf1_pio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_3_579545MHz/2, mpf1_ctc_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi8255_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, mpf1_cassette_config)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_TIMER_ADD_PERIODIC("halt_timer", check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpf1b, mpf1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(mpf1b_map)
	MCFG_CPU_IO_MAP(mpf1b_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	MCFG_MACHINE_START(mpf1)
	MCFG_MACHINE_RESET(mpf1)

	/* devices */
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_3_579545MHz/2, mpf1_pio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_3_579545MHz/2, mpf1_ctc_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi8255_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, mpf1_cassette_config)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1b)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD(TMS5220_TAG, TMS5220, 680000L)
	MCFG_SOUND_CONFIG(mpf1_tms5220_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_TIMER_ADD_PERIODIC("halt_timer", check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mpf1p, mpf1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(mpf1p_map)
	MCFG_CPU_IO_MAP(mpf1p_io_map)
	MCFG_CPU_CONFIG(mpf1_daisy_chain)

	MCFG_MACHINE_START(mpf1)
	MCFG_MACHINE_RESET(mpf1)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mpf1p)

	/* devices */
	MCFG_Z80PIO_ADD(Z80PIO_TAG, 2500000, mpf1_pio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, 2500000, mpf1_ctc_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi8255_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, mpf1_cassette_config)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_TIMER_ADD_PERIODIC("halt_timer", check_halt_callback, attotime::from_hz(1))
MACHINE_CONFIG_END

/* ROMs */

ROM_START( mpf1 )
    ROM_REGION( 0x10000, Z80_TAG, 0 )
    ROM_LOAD( "mpf.u6", 0x0000, 0x1000, CRC(b60249ce) SHA1(78e0e8874d1497fabfdd6378266d041175e3797f) )
ROM_END

ROM_START( mpf1b )
    ROM_REGION( 0x10000, Z80_TAG, 0 )
    ROM_LOAD( "c55167.u6", 0x0000, 0x1000, CRC(28b06dac) SHA1(99cfbab739d71a914c39302d384d77bddc4b705b) )
    ROM_LOAD( "basic.u7", 0x2000, 0x1000, CRC(d276ed6b) SHA1(a45fb98961be5e5396988498c6ed589a35398dcf) )
    ROM_LOAD( "ssb-mpf.u", 0x5000, 0x1000, CRC(f926334f) SHA1(35847f8164eed4c0794a8b74e5d7fa972b10eb90) )
    ROM_LOAD( "prt-mpf.u5", 0x6000, 0x1000, CRC(730f2fb0) SHA1(f31536ee9dbb9babb9ce16a7490db654ca0b5749) )
ROM_END

ROM_START( mpf1p )
    ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "mpf1pmon.bin", 0x0000, 0x2000, CRC(91ace7d3) SHA1(22e3c16a81ac09f37741ad1b526a4456b2ba9493) )
    ROM_LOAD( "prt-mpf-ip.u5", 0x6000, 0x1000, CRC(4dd2a4eb) SHA1(6a3e7daa7834d67fd572261ed4a9a62c4594fe3f) )
ROM_END

/* System Drivers */

DIRECT_UPDATE_HANDLER( mpf1_direct_update_handler )
{
	mpf1_state *state = machine->driver_data<mpf1_state>();

	if (!state->_break)
	{
		state->m1++;

		if (state->m1 == 5)
		{
			cputag_set_input_line(machine, Z80_TAG, INPUT_LINE_NMI, ASSERT_LINE);
		}
	}

	return 0;
}

static DRIVER_INIT( mpf1 )
{
	cputag_get_address_space(machine, Z80_TAG, ADDRESS_SPACE_PROGRAM)->set_direct_update_handler(direct_update_delegate_create_static(mpf1_direct_update_handler, *machine));
}

COMP( 1979, mpf1,  0,    0, mpf1, mpf1,  mpf1, "Multitech", "Micro Professor 1", 0)
COMP( 1979, mpf1b, mpf1, 0, mpf1b,mpf1b, mpf1, "Multitech", "Micro Professor 1B", 0)
COMP( 1982, mpf1p, mpf1, 0, mpf1p,mpf1b, mpf1, "Multitech", "Micro Professor 1 Plus", GAME_NOT_WORKING)
