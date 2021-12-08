void mac_list_volumes();
void mac_unmount(int id);
void mac_eject(int id);
OSErr mac_get_drive_volumes(int driveNum, Str255 str);
OSErr mac_unmount_drive(int driveNum);
OSErr mac_mount_drive(int driveNum);
OSErr mac_mount_drives();
OSErr mac_list_drives();