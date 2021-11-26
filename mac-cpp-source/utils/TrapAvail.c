/*
 * TrapAvail.c
 * by Thomas Tempelmann, macdev@tempel.org
 */

#include <Traps.h>
#include <OSUtils.h>
#include "TrapAvail.h"

Boolean TrapAvailable (short theTrap);

#define TrapMask 0x0800

static short NumToolboxTraps( void )
{
    if (NGetTrapAddress(_InitGraf, ToolTrap) ==
            NGetTrapAddress(0xAA6E, ToolTrap))
        return 0x0200;
    else
        return 0x0400;
}

static TrapType GetTrapType(short theTrap)
{
    if ((theTrap & TrapMask) > 0)
        return ToolTrap;
    else
        return OSTrap;
}

Boolean TrapAvailable (short theTrap)
{
    TrapType    tType;

    tType = GetTrapType(theTrap);
    if (tType == ToolTrap)
        theTrap = theTrap & 0x07FF;
    if (theTrap >= NumToolboxTraps())
        theTrap = _Unimplemented;
    return NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap);
}

// EOF
