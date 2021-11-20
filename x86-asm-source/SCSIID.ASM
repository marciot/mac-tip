Here is	some code that will take a handle to an	open partition and return
the SCSI ID. For example, if you opened	\\.\D: and called the following
code, it will return the SCSI ID for that drive	letter.

This code is kernel-mode, but can easly	be translated to native	NT by
changing the ZwXxx() calls to NtXxx() calls. I assume you can make a fairly
easy Win32 implementation since	this function shows the	IOCTL and data
structure to use.

Have fun...

// Get scsi address information	about a	device.	 This includes
// the port, bus, id, and lun, as well as the shortname	of the miniport
// driver that owns the	device.
//
// Arguments:
//
// Handle - handle to open device.
//
// ScsiAddress - receives port,	bus, id, and lun for the device	described
// by Handle.
//
// ScsiAdapterName - receives pointer to buffer	containing shortname
// for miniport	driver that owns the device (ie, aha154x).
// The caller must free	this buffer via	SpMemFree().
//
// Return Value:
//
// TRUE	- scsi address information was determined successfully.
// FALSE - error determining scsi address information.

BOOLEAN	GetScsiAddress
(
    IN	HANDLE Handle,
    OUT	PSCSI_ADDRESS  ScsiAddress,
    OUT	PWSTR *ScsiAdapterName
)
{
    NTSTATUS Status;
    PWSTR MiniportName = NULL;
    IO_STATUS_BLOCK IoStatusBlock;

    Status = ZwDeviceIoControlFile
    (
	Handle,
	NULL,
	NULL,
	NULL,
	&IoStatusBlock,
	IOCTL_SCSI_GET_ADDRESS,
	NULL,
	0,
	ScsiAddress,
	sizeof (SCSI_ADDRESS)
    );

    if (!NT_SUCCESS(Status))
    {
	KdPrint(("SETUP: Unable	to get scsi address info (%lx)\n",Status));
	return (FALSE);
    }

    // We can get the miniport name from the scsi port information list
    // we built	earlier.

    if(ScsiAddress->PortNumber < ScsiPortCount)
	MiniportName = ScsiPortInfo[ScsiAddress->PortNumber].MiniportName;
    else
    {
	// This	should not happen.
	ASSERT(ScsiAddress->PortNumber < ScsiPortCount);

	MiniportName = (PWSTR)TemporaryBuffer;
    }

    *ScsiAdapterName = DupString(MiniportName);

    return (TRUE);
}




