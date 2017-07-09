#	HIPS - a.k.a. Hide In Plain Sight
###	A steganography tool wich features compression, encryption and (almost) non-intrusive hiding techniques.

#### QUICK START:
- this works only on linux (Windows support comes at some point later),
- get this archive: https://github.com/holodon/hips/raw/master/hips_bin_latest.tar.gz
- extract the archive and then start hips_gtk,
- (OR of course clone the repository and compile by yourself just with `make`)
#### for creating image with embedded secret message:
1. use the **create** tab
2. set input and output image:
* 	use some jpg, png or bmp image as input (like the included cat.png),
* 	output can be only png or bmp (as of now).
3. set password and type the secret message
4. and hit **Go!**

#### for extracting the embedded secret message:
1. use the **extract** tab
2. choose the image
3. fill in the password
4. hit **Go!**

#### SECURITY NOTE:
- Only use unique pictures (i.e. made by you) and use each picture just once with the same password - this adds 
a lot to the overall security

### DESCRIPTION:

#### 1. What is Steganography?
[Steganography](https://en.wikipedia.org/wiki/Steganography) - you take something pretty "normal" at first glance 
(like picture), you hide there your message so that (hopefully) no one except the recipient can read it and voila - 
you can communicate in privacy without even looking suspicious.

#### 2. Other software for steganography already exists - why another tool?
- H.I.P.S. uses different approach, which leads to significantly lower impact on the host image, very difficult detection and ecnrypted message.

#### 3. Compression?
- H.I.P.S. uses database with the ~10 000 most used english words. Each word has an id in the database. The software stores not the word itself but the id, which leads to heavy compression in almost all cases.

#### 4. Encryption?
- This software uses stream encryption based on PRNG, which is seeded by the hash of the password. The encryption gets applied to the hiding route, the color channel and on the secret data itself.

#### 5. More details/ example?
* **creating an image:**
- If we take a our host image image.jpg and store there the secret sequence "All is set up! Meet me at the hospital" using the password "Meg@SecretP@33w0rt" then it goes like this:
```
    $./hips_c "Meg@SecretP@33w0rt" "All is set up! Meet me at the hospital" image.jpg out.png
```

* the software first find the hash of the password and use this to initiate the PRNG:
```
    Hashed pass: 1824454183
```
* then loads the image:
```
Loading image...
Input image loaded
```
* tokenizes the secret, removing all punctuation and finding the id of each word/token in the database:
```
Tokenizing input: found 9 tokens.
    bits_text: 140
    token #0: all
    id found in dictionary: 25
    token #1: is
    id found in dictionary: 8
    token #2: set
    id found in dictionary: 189
    token #3: up
    id found in dictionary: 54
    token #4: meet
    id found in dictionary: 859
    token #5: me
    ...
```

* then flattens the image in an array consisting of width X height X (rgb_channels X 8 bits) ignoring alpha if any, and shuffles this with our PRNG to create a route to hide:
```
    Hiding route (pixel/channel:  
    6801b  23498r  6944g  11517b  32849b  26470g  1014g  16886r  13829b  31400b  17290r  18088g  38656g  26036b  5013r  35611b  3652b  29594r  21762b  38679b  12956g  34585g  32460g  31221b ...
```

  6801b means that the first bit of our encrypted message will be stored in pixel 6801 in our image array and more exactly in its "B" color channel, then the next bit goes in the "R" channel of pixel 23498  and so on. NOTE: the software never uses more than one color channel per pixel in order to minimize the image invasion.

* **retrieving secret text:**
* the same as storing just in the opposite direction

#### 6. Limitations/TODO/planed upgrades?
* no windows binaries or gui, although you can compile and run all under cygwin
* png output only
* the used png compression is not optimal (WIP) and this can lead to somewhat bigger output
which has nothing to do with the inserted hidden data. H.I.P.S. adds --nothing-- to the file just changes some bits.
In numbers - our example cat.png grows from 66 to 85 kB.
* words not in the dictionary are not supported
	
### CREDITS:

* HIPS uses the stb_image and stb_image_write libraries from: https://github.com/nothings/stb/

* This tool was (originally) made as my final project in [CS50](https://www.edx.org/course/introduction-computer-science-harvardx-cs50x). It uses some CS50 code from pset4/2017, created by David J. Malan of Harvard University released under CC license (Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License)

* The 10 000 words dictionary is from [Josh Kaufman](https://github.com/first20hours/google-10000-english) (I added small number of words at the end like my name and hips_eof) Data files are derived from the Google Web Trillion Word Corpus, as described by Thorsten Brants and Alex Franz, and distributed by the Linguistic Data Consortium. Subsets of this corpus distributed by Peter Novig. Corpus editing and cleanup by Josh Kaufman.

* the sample picture 'cat.png' was taken from http://www.freeimages.com

DISCLAIMER:
1. Use this software at your own risk.
2. I am not a native english speaker so there may be mispellings and errors.


last updated: 09.07.2017
