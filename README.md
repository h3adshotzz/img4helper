# img4helper

Img4Helper is a small tool built for hnadling Apple's Image4 format used for the boot files of iOS/iPadOS and other ARM-Based platforms from Apple. The Image4 format is ASN.1 encoded and consits of an Image4 header, describing the type of image, version and any Keybags. The actual executable image, or just some type of data (Restore and boot Images are packed as .im4p), are contained within a payload tag. 

Apple ships an Image4 parser with iBoot, however the one used in this is based on that of @tihmstar's. Also, the SEP Splitting utility is based on @xerub's. So a special thanks to them, although they did not personally help me with this project, by open-sourcing both img4tool and sepsplit made this possible.


## Usage

Some documentation on using Img4Helper:
```
-----------------------------------------------------
Img4Helper 1.0.0 - Built Fri Dec 27 23:20:44 2019
-----------------------------------------------------

Usage: img4helper [options] FILE

Application Options:
  -h, --help			Print everything from the Image4 file.
  -v, --version			View Img4helper build info.

Image4:
  -a, --print-all		Print everything from the Image4 file.
  -i, --print-im4p		Print everything from the im4p (Providing there is one).
  -m, --print-im4m		Print everything from the im4m (Providing there is one).
  -r, --print-im4r		Print everything from the im4r (Providing there is one).

Extract/Decrypt/Decompress:
  -e, --extract         Extract a payload from an IMG4 or IM4P (Use with --ivkey and --outfile).
  -s, --extract-sep     Extract and split a Secure Enclave (SEPOS).
  -k, --ivkey           Specify an IVKEY pair to decrypt an im4p (Use with --extract and --outfile).

HTool Preview:
  -x, --xnu             Analyse an XNU KernelCache.
  -d, --devtree         Analyse a Device Tree.
```

### Application Options

To get to the help menu above, either run Img4Helper on it's own, or with the `--help` option. You can also get version information like so:

```
$ img4helper --version
h3adsh0tzz Img4Helper Version 1.0.0~Release (libhelper-1000.643.46.9)
	Build Time:		    Sat Dec 28 15:31:05 2019
	Default Target:		darwin-x86_64
	Build Type: 		Release
	Built With: 		libhelper-1000.643.46.9
```

This simply shows the version and build type of Img4Helper, the version of libhelper that's been linked, compile time, and build target.

### Image4 Options

There are four types of Image4 files: .img4, .im4p, .im4m and .im4r. The first option, `--print-all` can be used on any of these types and will print everything that it can find that makes sense to the parser. With `--print-{im4p|im4m|im4r}`, you can either run these on an entire .img4 which contains a payload, manifest and restore info, or their induvidual files. 

### Extract/Decrypt/Decompress Options

There are three options here. Firstly, `--extract` will extract and decompress the payload tag of a given .img4 or .im4p. If you pair this with `--ivkey` it will decrypt the payload with your provided key. Example:

```
$ img4helper --extract --ivkey <your_key> iBSS.d22.RELEASE.im4p
Loaded:         iBSS.d22.RELEASE.im4p
Image4 Type:    IM4P
Component:      iBoot Single Stage (iBSS)

[*] Detecting compression type... encrypted.
[*] Attempting to decrypting with ivkey: <your_key>
[*] File decrypted successfully.
[*] Detecting compression type... bvx2
[*] Writing decompressed payload to file: outfile.raw (n bytes)
```

The output file will always be `outfile.raw`. I will add an option to specify the outfile in a future release.

Finally is `--extract-sep`. You can pair this with `--ivkey`, providing you have a decryption key like so to extract, decrypt and split a Secure Enclave OS Image:
```
$ img4helper --extract-sep --ivkey <your_key> sep-firmware.n841.RELEASE.im4p
Loaded: 	    sep-firmware.n841.RELEASE.im4p
Image4 Type: 	IM4P
Component: 	    Secure Enclave OS (SEP OS)

[*] Detecting compression type... encrypted
[*] Attempting to decrypting with ivkey: <your_key>
[*] File decrypted successfully.
[*] Detecting compression type... none.
[*] Writing decompressed payload to file: .sep-img4helper-decrytped (7438336 bytes)
[*] File loaded okay. Attempting to identify Mach-O regions...
Parsed: SEP_boot
Parsed: SEP_kernel
Parsed: SEP_MachO_2
...
Parsed: SEP_MachO_8
```


### HTool Preview Options

These will be added soon. HTool is my closed-source analysis tool for Mach-O files, specifically iOS firmware files. I'll discuss this more in the near future, but this will contain a few preview options from HTool like Device Tree parsing and XNU KernelCache analysis.


## Releases

Img4Helper is opensource, so you'll always be able to get the latest debug version from here. I regularly ship Beta versions between major releases. The most recent Beta/Release can be downloaded from the Releases tab here on Github, or from my website http://h3adsh0tzz.com.

There will be a downloads API eventually for all my tools (Maybe even a package manager :P), but for now you can query the following link, and modify `latest` based on the version you wish to obtain:
```
https://s3.cloud-itouk.org/dnlds/releases/img4helper/img4helper-darwinx86-latest.zip
```

## Roadmap

There are a number of features left to add. They aren't critical features, just things that would enhance the capability of Img4Helper:
 * Constructing Image4 files
 * Identification of Firmware component, e.g. "Darwin Kernel Version 19.0.0, xnu-4903.270.47~7".
 * Extracting firmwares keys from connected Checkra1n device (May require checkra1n sources).

There are also a number of things I'd like to rework:
 * Move ASN1/Image4 parsing to libhelper and make it less specialised to this application. 
 * Add AES decryption in a way that doesn't require OpenSSL or CryptoKit.


## Build / Compile

Img4Helper using Meson and Ninja for building. First, ensure you have both Meson and Ninja installed on your system. Currently, I'm only supporting macOS so if you'd like to compile on either iOS or Linux, you'll have to link OpenSSL and modify `img4_decrypt_bytes ()`. On macOS I'd assume CryptoKit is already installed. To build, run from the base directory:

```sh
$ meson build
...
$ cd build && ninja
...
$ ./src/img4helper --version
```

Because Img4Helper uses a libhelper, make sure you run the following to also pull the current version of libhelper:

```sh
$ git submodule init && git submodule update
```

If you are building from Master, please be advised there may be compile errors as I commit code when switching between machines. Any issues with this please create an Issue or Tweet me @h3adsh0tzz. 
