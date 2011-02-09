/**********************************************************************

    Xebec S1410 5.25" Winchester Disk Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __S1410__
#define __S1410__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define S1410_TAG	"s1410"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_S1410_ADD() \
    MCFG_DEVICE_ADD(S1410_TAG, S1410, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s1410_interface

struct s1410_interface
{
};


// ======================> s1410_device_config

class s1410_device_config :   public device_config,
                                public s1410_interface
{
    friend class s1410_device;

    // construction/destruction
    s1410_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

	// optional information overrides
	virtual const rom_entry *rom_region() const;
	virtual machine_config_constructor machine_config_additions() const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};


// ======================> s1410_device

class s1410_device :  public device_t
{
    friend class s1410_device_config;

    // construction/destruction
    s1410_device(running_machine &_machine, const s1410_device_config &_config);

public:

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
    const s1410_device_config &m_config;
};


// device type definition
extern const device_type S1410;



#endif
