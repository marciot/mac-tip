![Trouble In Paradise for Macintosh][mac-screenshot1]

TIP for 68k and PowerPC Macintosh
=================================

This is the repository for a Macintosh port of [Gibson Research Corp]'s
[Trouble in Paradise], a diagnostic and repair tool for Iomega Zip and
Jaz Drives. A big thanks goes to Steve Gibson for the x86 source that
made this project possible!

Compatibility
-------------

This tool is meant for Macintosh computers with a SCSI port, which
range from the Macintosh Plus, released in 1986; through the "Beige"
Power Macintosh G3, released in 1997.

<details>
<summary>Click here to read what works and what doesn't</summary></br>

It has been tested in on the following environments:

| Computer        | Memory | System | Drive        |
|-----------------|--------|--------|--------------|
| Macintosh Plus  | 4MB    | 7.0.1  | Zip Plus 100 |
| PowerBook 3400c | 144MB  | 8.6    | Zip 100      |

What will not work or is missing:

* Support for JAZ drives is currently missing.
* TIP will only do a read-pass and the write-tests have been disabled.
* The functionality for operating with password or write protected disks is missing.
* The partition recovery feature has been removed, since it is meant for PC disks.
* The detailed post-test explanation screens are absent. For now, I refer you to the documentation on the [Trouble in Paradise] pages.

</details>

Disclaimer
----------

**This software is provided on as "as is" basis and the user assumes
all risk of data loss. Although this software bears a similarity to
the original by GRC, I may have introduced errors during the conversion
process. Please do not reach out to GRC for support on this Macintosh
port.**

Where are the binaries?
-----------------------

Right now the port is in early testing, so I am not releasing any
compiled binaries. There is always a risk of data loss with a tool like
this, so please post in the discussion forum if you want to beta test.

How can you help?
-----------------

You can help this project in one of the following ways:

* Star this project on GitHub to show your support!
* Sign up to [beta test in the discussion forum](https://github.com/marciot/mac-tip/discussions/1#discussion-3704575)!
* Donate a 1 or 2GB Jaz cartridge for testing (I have a 2GB SCSI Jaz drive, but no cartridges!).
* Become a GitHub sponsor to help fund my work with this and other open-source projects!

Got work?
---------

I am open to paid consulting work related to retro-computing or software
for vintage Macs. If you have a project in mind, please hit me up via my
[GitHub account]!

Unmounting volumes using the Command Console
--------------------------------------------

**To avoid data corruption, you must "unmount" the cartridge prior to
starting a test with TIP!** 

There is no way to unmount a cartridge (without also ejecting it) in the
Finder, but the program provides a Command Console that allows you to do
this and other things.

<details>
<summary>Click here to learn how to use the Command Console</summary></br>

The Command Console shows up when you first start the program. This is a
unique feature of this port and is not present in the original TIP:

![Command Line][mac-screenshot2]

Start by inserting the disk you want to test. The disk will show up in the
Finder as an icon; MacOS calls this a "volume". To prevent data corruption,
you have to "unmount" the volume prior to testing. When a volume is unmounted,
the icon will disappear from the Finder, but the cartridge will remain in the
drive. To unmount a drive:

* Type `volumes` to show a numbered list of all Mac volumes
* Type `unmount` followed by a volume's number to unmount it.

*The unmount process will fail if you have any open files or applications in
the volume; if this happens, close those files and try again.*

Once you have the cartridge unmounted, you will need to tell TIP which
SCSI device to use:

* Type `list` to show a numbered list of all SCSI devices by SCSI ID.
* Type `tip` followed by a SCSI ID to run TIP on that device.

Future versions of this tool may eliminate the Command Console, if I can find
the right way to automate all these steps in MacOS :grin:
</details>

About the code
--------------

Portions of the original code have been re-written in C++ and adapted
for compilation using [Metrowerks CodeWarrior 8 Gold]. The code is located
in the `mac-cpp-source` directory.

<details>
<summary>Click here to learn more about the code</summary></br>

![Metrowerks CodeWarrior 8 Gold Project][mac-screenshot3]

Native [Macintosh SCSI Manager] routines have been substituted for
the Win32 ASPI routines and the UI has been re-created as closely as
possible using QuickDraw routines.

Although the source code is materially different, I have maintained the
layout and routines names of the original assembly code to allow for easy
cross referencing and to make it easy to port additional functionality
in the future.

</details>

The Original TIP For Windows
----------------------------

The source code in the `x86-asm-source` directory is the original Windows
source code as graciously provided to me by Steve Gibson. I am republishing
his code in this repository with his permission.

<details>
<summary>Click here to see screenshots of the original TIP</summary></br>

![Trouble In Paradise About Box][win-screenshot1]
![Trouble In Paradise Testing][win-screenshot2]

</details>

![TIP Running on a Mac Plus][mac-plus]

[GitHub account]: https://github.com/marciot
[mac-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip1.png "Mac TIP Testing Screen"
[mac-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/mac-cmd.png "Mac TIP Command Line"
[mac-screenshot3]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip-cw8.png "Mac TIP Build Environment"
[win-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/win-tip1.gif "Windows TIP About Screen"
[win-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/win-tip2.gif "Windows TIP Testing Scren"
[mac-plus]: https://github.com/marciot/mac-tip/raw/main/images/macplus-tip.jpg "TIP running on a Mac Plus"
[Gibson Research Corp]: https://www.grc.com
[Trouble in Paradise]: https://www.grc.com/tip/clickdeath.htm
[Metrowerks CodeWarrior 8 Gold]: https://www.macintoshrepository.org/11910-codewarrior-8-gold
[Macintosh SCSI Manager]: https://developer.apple.com/legacy/library/documentation/mac/pdf/Devices/Scsi_Manager.pdf
