![Trouble In Paradise for Macintosh][mac-screenshot1]

TIP for 68k and PowerPC Macintosh
=================================

This is the repository for a Macintosh port of [Gibson Research Corp]'s
[Trouble in Paradise], a diagnostic and repair tool for Iomega Zip and
Jaz Drives.

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

[mac-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip1.png "TIP"
[win-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/win-tip1.gif "TIP"
[win-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/win-tip2.gif "TIP"
[Gibson Research Corp]: https://www.grc.com
[Trouble in Paradise]: https://www.grc.com/tip/clickdeath.htm
[Metrowerks CodeWarrior 8 Gold]: https://www.macintoshrepository.org/11910-codewarrior-8-gold
[Macintosh SCSI Manager]: https://developer.apple.com/legacy/library/documentation/mac/pdf/Devices/Scsi_Manager.pdf