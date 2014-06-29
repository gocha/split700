split700
========

Extracts BRR samples from SNES SPC700 format (*.spc)

How to use
------------------------

Syntax: `split700 (options) *.spc`

You can display a help by `split700 --help`

Drag and drop SPC file(s) into split700.exe, and you will get BRR samples in the input directory.

### Options

|Short   |Long           |Description                                           |
|--------|---------------|------------------------------------------------------|
|`-f`    |`--force`      |Force output corrupt samples.                         |
|`-l`    |`--list`       |Display voice list, but do not dump any brrs.         |
|N/A     |`--least n`    |Assume a brr sample has at least n blocks.            |
|`-L`    |N/A            |Add loop point info to output filename of the sample. |
|`-M`    |N/A            |Add a simple header for addmusicM.                    |
|`-z`    |`--allow-zero` |Allow samples which starts from $0000.                |
|`-?`    |`--help`       |Display this help.                                    |

Thanks to
------------------------

- [Snes9x](http://www.snes9x.com/): One of famous SNES emulators. BRR decoding routine helped me a lot.
- [snesbrr](http://www.romhacking.net/utilities/407/): Good BRR converter. On the other hand, I created brr2wav to get more raw output. (It should be bundled with split700 package.)
