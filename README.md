![Trouble In Paradise for Macintosh][mac-screenshot1]

TIP for 68k and PowerPC Macintosh
=================================

This is the repository for a Macintosh port of [Gibson Research Corp]'s
[Trouble in Paradise], a diagnostic and repair tool for Iomega Zip and
Jaz Drives.

This tool is meant for Macintosh computers with a SCSI port, which
range from the Macintosh Plus, released in 1986, through the "Beige"
Power Macintosh G3, released in 1997.

Disclaimer
----------

**This software is provided on as "as is" basis and the user assumes
all risk of data loss. Although this software bears a similarity to
the original by GRC, I may have introduced errors during the conversion
process. Please do not reach out to GRC for support on this Macintosh
port.**

How Can You Help?
-----------------

You can help this project in one of the following ways:

* Star this project to show your support!
* Sign up to [beta test in the discussion forum](https://github.com/marciot/mac-tip/discussions/1#discussion-3704575)!
* Donate a 1 or 2GB Jaz cartridge so I can do some testing with that (I have a 2GB SCSI Jaz drive, but I do not have a cartridge).
* Become a GitHub sponsor to help fund my open-source projects!
* If you have consulting work, in particular contract work pertaining to retro-computers or THREE.js, please hit me up via my [GitHub account]!

Where are the binaries?
-----------------------

Right now the port is in early testing, so I am not releasing any
compiled binaries. There is always a risk of data loss with a tool like
this, so please post in the discussion forum if you want to beta test.

Command console
---------------

When you first start the program, you will be greeted with a Command
Console. This is a unique feature of this port and is not present in
the original TIP:

![Command Line][mac-screenshot2]

**Before launching TIP, you will need to do some prep work. It is *vitally
important* that you unmount the Zip cartridge prior to running TIP on it.**

Unmounting is different than ejecting a cartridge. Once a Zip cartridge is
unmounted, the icon will disappear from the Finder, but the cartridge will
remain in the drive.

There is no way to unmount a cartridge without ejecting it in the Finder,
but you may do it via the Command Console like this:

* Type `volumes` to show a numbered list of all Mac volumes
* Type `unmount` followed by a volume's number to unmount it.

Once you have the Zip cartridge unmounted, you will need to tell TIP which
SCSI device to use.

* Type `list` to show a numbered list of all SCSI devices by SCSI ID.
* Type `tip` followed by a SCSI ID to run TIP on that device.

At the present, I am unable to find any documented MacOS APIs for
automatically unmounting volumes associated with a SCSI ID. Future versions
may simplify the process if I find out how to do this.

About the code
--------------

Portions of the original code have been re-written in C++ and adapted
for compilation using [Metrowerks CodeWarrior 8 Gold]. Native
[Macintosh SCSI Manager] routines have been substituted for the Win32
ASPI routines and the UI has been re-created as closely as possible
using QuickDraw routines.

Although the source code is materially different, I have maintained the
layout and routines names of the original code to allow for easy cross
referencing with the original code and to make it easy to port additional
functionality in the future.

The Original TIP For Windows
----------------------------

The source code in the "x86-asm-source" directory is the original Windows
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
[win-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/win-tip1.gif "Windows TIP About Screen"
[win-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/win-tip2.gif "Windows TIP Testing Scren"
[mac-plus]: https://github.com/marciot/mac-tip/raw/main/images/macplus-tip.jpg "TIP running on a Mac Plus"
[Gibson Research Corp]: https://www.grc.com
[Trouble in Paradise]: https://www.grc.com/tip/clickdeath.htm
[Metrowerks CodeWarrior 8 Gold]: https://www.macintoshrepository.org/11910-codewarrior-8-gold
[Macintosh SCSI Manager]: https://developer.apple.com/legacy/library/documentation/mac/pdf/Devices/Scsi_Manager.pdf
