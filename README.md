#	HIPS - a.k.a. Hide In Plain Sight
###	A steganography tool wich features compression, encryption and (almost) non-intrusive hiding techniques.


### UPDATE 25.06.2017
- some basic gtk+ interface - uses the binaries hips_c and hips_e 
in the same directory
- now with added support for png, based on the wonderfull libraries
from stb (link below in credits)
- replaced all fprintf with printf because otherwise the messages
appear in the gtk+ interface out of order (TODO: fix and use fprintf)


DESCRIPTION:

Let us suppose you want to communicate with someone over an insecure medium (like, uhm ... internet). The first choice would be 
maybe an encrypted connection, but one problem with it is that ... well it looks like encrypted connection.

Meet steganography (https://en.wikipedia.org/wiki/Steganography) - you take something pretty "normal" at first glance 
(like picture), you hide there your message so that (hopefully) no one except the recipient can read it and voila - 
you can communicate in privacy without even looking suspicious.

Like with everything else there are drawbacks too.

One commonly used technique is hiding the secret message in the two least significant bits in each pixel of an image. When the
hidden message is not encrypted is trivial to detect and read it, but there are some drawbacks even with encrypted messages - 
like for example adding too much "randommness" to the image which can of course draw attention. One other drawback is that 
the size of the hidden text has to be limited, otherwise one has to use too much of the image data, which of course also 
can be suspicious.

Addressing all of this problems, HIPS offers a new kind of steganography, with the following main features:

--- Heavy compression ---

According to some research (http://oxforddictionaries.com/words/the-oec-facts-about-the-language) 90% of the 
most used words in the english language are as much as 7000 words. In HIPS we use dictionary with 10 000 most common words, 
which should be more than enough for the most cases. Each word gets represented by a 14 bit integer, so even with 2-letter 
word you have already spared 2 bits. It gets even more interesting with whole text where the compressions can really make 
the difference.

For example - it would take 11*8 = 88 bits to represent a somewhat longer word like "information", but in our case we still 
have 14 bits.

Drawback 1: one-letter words takes more bits to represent in HIPS (or exactly 14 vs 8), this is but negligible, cause most 
of the words are longer

Drawback 2: words, which are not in the dictionary, are (not yet) supported. TODO.

--- Encryption ---

There are this wonderfull PRNG generators (https://en.wikipedia.org/wiki/Pseudorandom_number_generator) which, given the 
same seed, will always produce the same output. The thing is, if you don't know the seed, than you can not predict the output, 
which looks completely random.

We use (a hash of) the password to seed our PRNG and then get starting position in the file, determine the hidind route for
the secret bits, and even encrypt every single bit before writing it into the file. If you want to go a litle futher, you can
use the password only once which should provide the equivalency of One-Time-Pad which is unbreakable
(https://en.wikipedia.org/wiki/One-time_pad).

--- Small footprint ---

Thanks to the above mentioned techniques, we can achieve really small footprint, writing only one bit per pixel, in total of
(words+1)x14 pixels, which is almost nothing. Additionally, calculating the starting pixel from the password, the only 
additional bits of info we write into the file is our EOF (also 14 bits).

Every image consists of pixels, which in turn are described by three 8-bit color values - Reg, Green and Blue. We hide our message
only in one bit of each pixel, choosing randomly between R, G and B. This leads to nicely dispersed randomness, which is very
difficult to detect.

Drawback 3: As of now, the start pixel is always between 0 and 100 and all manipulated pixels are followind each other. 
In the next updates we plan to disperse the information evenly through the whole file leading to almost impossible detection.

.....

Drawback 4: ~~~As of now, only uncompressed BMP are supported. TODO.~~~ See update 25.06.2017

Drawback 5: ~~~No GUI or other fancy stuff. TODO.~~~ See update 25.06.2017


USAGE:

1. get this archive: (https://github.com/holodon/hips/raw/master/hips_bin.tar.gz) with the binaries (compiled on slackware 14.2 x64) OR

- compile with (*)
    'make'
    This creates hips_c, hips_e and hips_gtk. The dictionary is already inserted into an hips.db as SQL database, but the text version 
    muw.dict is also provided for your convenience.

2. click on hips_gtk OR

-- create a steganography image with hidden text with: (*)
    './hips_c password "text text text" image'
    Please note: all punctuation gets automatically disgarded, and it will fail if some of the words are not in the dictionary.
    If succesfull outputs 'out.bmp'

-- extract the hidden text from the image: (*)
    './hips_e password image'
    It outputs everything what it finds in the terminal, stopping when it reads hips_eof. 
    Note: if there is nothing hidden in the file, the output can be long -
    after all each 14 bits in every image represent some number ;)

- Also provided is some simple bmp picture 'work.bmp' (of me working hard) which you are welcome to use for testing.

(*) If you are using Windows you cannot compile C or use SQL out-of-the-box. 
	The simplest way I can think of to get it all working is installing CYGWIN (https://cygwin.com/install.html).
	Note: Do not forget to include 'gcc-core', 'libsqlite3' and 'libsqlite3-devel' during the installation process.
	After installation, navigate in the cygwin terminal to the directory where you have HIPS and then proceed like above.
	For example - In Windows 7 your 'Desktop' is at '/cygdrive/c/Users/--your-username--/Desktop/'
	TODO: cross-compile binaries for windows

	
CREDITS:

- HIPS uses the stb_image and stb_image_write libraries from: https://github.com/nothings/stb/

- This tool was (originally) made as my final project in CS50 (https://www.edx.org/course/introduction-computer-science-harvardx-cs50x).
It uses some CS50 code from pset4/2017, created by David J. Malan of Harvard University released under CC license 
(Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License)

- The 10 000 words dictionary is from Josh Kaufman:
https://github.com/first20hours/google-10000-english
(I added small number of words at the end like my name and hips_eof)
Data files are derived from the Google Web Trillion Word Corpus, as described by Thorsten Brants and Alex Franz, 
and distributed by the Linguistic Data Consortium. Subsets of this corpus distributed by Peter Novig. 
Corpus editing and cleanup by Josh Kaufman.


DISCLAIMER:
1. Use this software at your own risk.
2. I am not a native english speaker so there may be mispellings and errors.


last updated: 25.06.2017
Vladimir Zhelezarov
jelezarov.vladimir@gmail.com
