#include "mac_scsi.h"
#include "iomega_cmds.h"

// Iomega commands

OSErr iomega_spin_up_cartridge( int id ) {
    // issue an Asynchronous START command to induce spinup
    char cmd[6] = {
        SCSI_Cmd_StartStopUnit,
        1,   // set the IMMED bit for offline
        0,
        0,
        1,   // start the disk spinning
        0
    };
    return scsi_cmd(id, cmd, sizeof(cmd), 0, 0, 0, 0);
}

OSErr iomega_spin_down_and_eject( int id ) {
    // issue an Asynchronous STOP command to induce spindown and ejection
    char cmd[6] = {
        SCSI_Cmd_StartStopUnit,
        1,   // set the IMMED bit for offline
        0,
        0,
        2,   // eject a Jaz disk after stopping
        0
    };
    return scsi_cmd(id, cmd, sizeof(cmd), 0, 0, 0, 0);
}

OSErr iomega_set_prevent_removal( int id, bool lock) {
    OSErr err;
    char cmd[6] = {
        SCSI_Cmd_PreventAllow,
        0,
        0,
        0,
        lock ? 1 : 0,
        0
    };
    return scsi_cmd(id, cmd, sizeof(cmd), 0, 0, 0, 0);
}

OSErr iomega_eject_cartridge( int id ) {
    OSErr err;
    err = iomega_set_prevent_removal(id, false);
    if (err != noErr) return err;
    return iomega_spin_down_and_eject(id);
}
