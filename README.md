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

Where are the binaries?
-----------------------

Right now the port is in early testing, so I am not releasing any
compiled binaries. There is always a risk of data loss with a tool like
this, so please contact me directly if you want to beta test.

Got projects?
-------------

If you have consulting work, particular contract work pertaining
to vintage computers or THREE.js, please hit me via my [GitHub account]!

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

[GitHub account]: https://github.com/marciot
[mac-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip1.png "TIP"
[win-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/win-tip1.gif "TIP"
[win-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/win-tip2.gif "TIP"
[Gibson Research Corp]: https://www.grc.com
[Trouble in Paradise]: https://www.grc.com/tip/clickdeath.htm
[Metrowerks CodeWarrior 8 Gold]: https://www.macintoshrepository.org/11910-codewarrior-8-gold
[Macintosh SCSI Manager]: https://developer.apple.com/legacy/library/documentation/mac/pdf/Devices/Scsi_Manager.pdf