#include <scsi.h>

typedef Boolean bool;

OSErr iomega_spin_up_cartridge(int id);
OSErr iomega_spin_down_and_eject(int id);
OSErr iomega_set_prevent_removal(int id, bool lock);
OSErr iomega_eject_cartridge(int id);