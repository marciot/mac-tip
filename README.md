![Trouble In Paradise for Macintosh][mac-screenshot1]

TIP for 68k and PowerPC Macintosh
=================================

This is the repository for a Macintosh port of [Gibson Research Corp]'s
[Trouble in Paradise], a diagnostic and repair tool for Iomega Zip and
Jaz Drives. A big thanks goes to Steve Gibson for the x86 source that
made this project possible!

[![Trouble in Paradise Demonstration](https://github.com/marciot/mac-tip/raw/main/images/youtube.png)](https://youtu.be/vtBlOaG2pNw)

:tv: See a demo on [YouTube]!

[![Mention in Steve Gibson's Security Now Podcast](https://github.com/marciot/mac-tip/raw/main/images/security-now.png)](https://twit.tv/shows/security-now/episodes/845)

:tv: Watch Steve Gibson react to my unusual request in the November 16th
episode of the "Security Now" podcast at the 1:05:10 mark.

Compatibility
-------------

This tool is meant for Macintosh computers with a SCSI port, which
range from the Macintosh Plus, released in 1986; through the "Beige"
Power Macintosh G3, released in 1997.

<details>
<summary>Click here to read what works and what doesn't</summary></br>

It has been tested in on the following environments:

| Computer           | Memory | System | Drive                   | Firmware |
|------------------- |--------|--------|-------------------------|----------|
| Macintosh Plus     | 4MB    | 7.0.1  | Zip Plus 100 Ext. SCSI  | J.66     |
| PowerBook 3400c    | 144MB  | 8.6    | Zip 100 Ext. SCSI       | E.08     |
| Power Macintosh G3 | 256MB  | 9.2.1  | Zip 100 Ext. SCSI       | C.22     |
| Power Macintosh G3 | 256MB  | 9.2.1  | Zip 100 Int. SCSI       | J.03     |

What will not work or is missing:

* USB connected Zip drives have been reported to not work.
* The functionality for operating with password or write protected disks has not been ported.
* The partition recovery feature has been removed, since it is meant for PC disks.

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

Compiled binaries are available on the releases page for people who have signed up for
[beta testing in the discussion forum]

How can you help?
-----------------

You can help this project in one of the following ways:

* Star this project on GitHub to show your support!
* Sign up to [beta testing in the discussion forum]!
* Donate a 1 or 2GB Jaz cartridge for testing (I have a 2GB SCSI Jaz drive, but no cartridges!).
* Become a GitHub sponsor to help fund my various open-source projects!

Other Vintage Macintosh Stuff
-----------------------------

* [ScreenChooser]: A dynamic background changer for vintage Macintoshes!
* [Retroweb Vintage Computer Museum]: A web-based museum of vintage computers, including the Macintosh!

Got work?
---------

I am open to paid consulting work related to retro-computing or software
for vintage Macs. If you have a project in mind, please hit me up via my
[GitHub account]!

Unmounting volumes
------------------

**To avoid data corruption, a cartridge must be unmounted prior to
starting a test with TIP!** 

The best way to accomplish this is to start TIP without a disk in the drive you want to test.
Then, insert the cartridge at the testing screen which indicates "Awaiting media...". TIP
will recognize it if Mac OS mounts newly inserted cartridge and will give you the choice to
unmount it.

Alternatively, you can use the command line from the "Advanced" menu to unmount a disk
**prior to** starting a test.

<details>
<summary>Click here to learn how to use the command line</summary></br>

The command line is accessible by selecting run "Run Command Line..." from the "Advanced"
menu. This is a unique feature of this port and is not present in the original TIP:

![Command Line][mac-screenshot2]

### Unmounting Volumes

A cartridge will show up in the Finder as an icon; MacOS calls this a "volume".
To prevent data corruption, you have to "unmount" the volume prior to testing.
When a volume is unmounted, the icon will disappear from the Finder, but the
cartridge will remain in the drive. To unmount a drive:

* Type `volumes` to show a numbered list of all Mac volumes
* Type `unmount` followed by a volume's number to unmount it.

*The unmount process will fail if you have any open files or applications in
the volume; if this happens, close those files and try again.*

### Listing SCSI devices

* Type `list` to show a numbered list of all SCSI devices by SCSI ID.

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

Credits
-------

* A big thanks goes to Steve Gibson for the source code that made this possible!
* Thank you to [Stone Table Software] for providing the tool I used to convert the Windows RTF docs into SimpleText...
* ...and to the Internet Archive's Wayback Machine for allowing me to access it long after the website was shut down!
* Thank you to Thomas Tempelmann sharing his [LaunchLib code] which I used to open the documents

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
[mac-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip1.gif "Mac TIP Animation"
[mac-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/mac-cmd.png "Mac TIP Command Line"
[mac-screenshot3]: https://github.com/marciot/mac-tip/raw/main/images/mac-tip-cw8.png "Mac TIP Build Environment"
[win-screenshot1]: https://github.com/marciot/mac-tip/raw/main/images/win-tip1.gif "Windows TIP About Screen"
[win-screenshot2]: https://github.com/marciot/mac-tip/raw/main/images/win-tip2.gif "Windows TIP Testing Scren"
[mac-plus]: https://github.com/marciot/mac-tip/raw/main/images/macplus-tip.jpg "TIP running on a Mac Plus"
[Gibson Research Corp]: https://www.grc.com
[Trouble in Paradise]: https://www.grc.com/tip/clickdeath.htm
[Metrowerks CodeWarrior 8 Gold]: https://www.macintoshrepository.org/11910-codewarrior-8-gold
[Macintosh SCSI Manager]: https://developer.apple.com/legacy/library/documentation/mac/pdf/Devices/Scsi_Manager.pdf
[explanation document]: https://github.com/marciot/mac-tip/raw/main/x86-asm-source/RTF.RTF
[beta testing in the discussion forum]: https://github.com/marciot/mac-tip/discussions/1
[LaunchLib code]: http://www.tempel.org/macdev/index.html#Libs
[Stone Table Software]: https://web.archive.org/web/20010308062807/http://www.stonetablesoftware.com/rtf2text.html
[ScreenChooser]: https://archive.org/details/screen-chooser
[YouTube]: https://youtu.be/vtBlOaG2pNw
[Retroweb Vintage Computer Museum]: http://retroweb.maclab.org


