#	STIDE - hiding secrets in images
###	A steganography tool wich features compression, encryption and (almost) non-intrusive hiding techniques.

<img src="media/stide_big.png" alt="Stide screenshots" />

#### MAJOR UPDATE
- Stide is completely rewritten - now fully modular, single binary, lot more options and better responses.
- Now beside the 'strict' mode with words only from the dictionary we added support of 'loose' mode - which accepts arbitrary ASCII characters for input, at the cost of compression.
- Fully backwards compatible (use the '-s' switch for 'strict' mode).
- The old binaries and GUI can be found in bin/linux/old
- Changed license to MIT.
- Name changed because we found some old project on the internet with the same name.
- Build with sqlite3, zlib and libpng as static libs to be even more easy to use - no dependancies.

#### QUICK START:
- this works only on 64 bit linux (32 bit Linux / Windows support comes at some point later),
- [get this archive](https://github.com/vlzware/stide/blob/master/releases/stide_bin_x64_2.1.tar.gz?raw=true)
- extract the archive and then start stide_gtk,
- OR for **the latest and greatest** version clone the repository and compile by yourself (see "Compiling").
#### for creating image with embedded secret message:
1. use the **create** tab;
2. set input and output image:
* 	use some jpg, png or bmp image as input (like the included cat.png),
* 	output can be only png or bmp (as of now);
3. set password and type the secret message;
4. set some switches if you need to;
5. and hit **Go!**

#### for extracting the embedded secret message:
1. use the **extract** tab
2. choose the image
3. fill in the password
4. set some switches if you need to;
5. hit **Go!**

#### SECURITY NOTE:
- Only use unique pictures (i.e. made by you) and use each picture just once with the same password - this adds a lot to the overall security

### DESCRIPTION:

#### 1. What is Steganography?
[Steganography](https://en.wikipedia.org/wiki/Steganography) - you take something pretty "normal" at first glance 
(like picture), you hide there your message so that (hopefully) no one except the recipient can read it and voila - 
you can communicate in privacy without even looking suspicious.

#### 2. Other software for steganography already exists - why another tool?
- Stide uses different approach, which leads to significantly lower impact on the host image, very difficult detection and ecnrypted message.

#### 3. Compression?
- Stide uses database with the ~10 000 most used english words. Each word has an id in the database. The software stores not the word itself but the id, which leads to heavy compression in almost all cases. Longer sentences lead usually to even more compression.
- The new 'loose' mode do not compress at all - it stores the same size as it reads.

#### 4. Encryption?
- Stide uses stream encryption based on PRNG, which is seeded by the hash of the password. The encryption gets applied to the hiding route, the color channel and on the secret data itself.

#### 5. Compiling:
- Compile with 'make' from the 'src' directory. You may need to install sqlite3 and libpng headers. The resulting binaries (stide and stide_gtk) reside in 'bin/linux'.
- Stide uses statically builded versions of zlib and libpng.
- sqlite3 source and headers are also compiled.
- Here is the complete output of 'ldd stide':
```
	linux-vdso.so.1 =>  (0x00007ffc7b77e000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007efffb37e000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007efffafb3000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007efffad96000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007efffab92000)
	/lib64/ld-linux-x86-64.so.2 (0x000055f67faf6000)

```
- 'stide.glade' is a .xml file for the GUI settings;
- 'stide_gtk' is the gtk gui executable - this is basically just an interface to the command line tool.

#### 6. Command line usage:
```
    $./stide
```
- This shows all available options with their description and usage.

* **creating an image:**
```
    $./stide -c[spvdf] [path/to/stide.db] password text img-in [img-out[.png]]
```

* **retrieving secret text:**
```
    $./stide -e[svdf] [path/to/stide.db] password img-in
```

#### 7. Limitations/TODO/planed upgrades?
- The hidden secret is *fragile* - meaning the simplest change to the picture will destroy the payload. Note: this can be also a pro, depending on the situation;
- No windows binaries or gui (WIP), although you can compile and run all under cygwin;
- Outputs only png or bmp;
- ~~The used png compression is not optimal (WIP) and this can lead to somewhat bigger output
which has nothing to do with the inserted hidden data. Stide adds --nothing-- to the file just changes some bits. In numbers - our example cat.png grows from 66 to 85 kB.~~ Stide switched to libpng, so there is no more png compression penalty on the output.
- ~~words not in the dictionary are not supported~~ Stide now supports both modes - words only from the dictionary ('strict' mode) and 'loose' mode with arbitrary ASCII characters.
	
### CREDITS:
- Stide uses a static build of [libpng](http://libpng.org/pub/png/libpng.html). The code for saving in png is mostly from [here](http://www.labbookpages.co.uk/software/imgProc/libPNG.html).

- For the needs of libpng, stide compiles also with a static build of [zlib](http://zlib.net/).

- Stide uses the stb_image and stb_image_write libraries from: [STB](https://github.com/nothings/stb/).

- This tool was (originally) made as my final project in [CS50](https://www.edx.org/course/introduction-computer-science-harvardx-cs50x).

- The 10 000 words dictionary is from [Josh Kaufman](https://github.com/first20hours/google-10000-english) (I added small number of words at the end like my name and stide_eof) Data files are derived from the Google Web Trillion Word Corpus, as described by Thorsten Brants and Alex Franz, and distributed by the Linguistic Data Consortium. Subsets of this corpus distributed by Peter Novig. Corpus editing and cleanup by Josh Kaufman.

- the sample picture 'cat.png' was taken from http://www.freeimages.com

DISCLAIMER:
 Use this software at your own risk.


last updated: 15.01.2018
