# img4helper

Based on img4tool by Tihmstar. The primary purpose of img4helper was to improve my understanding of the Image4 file format and iOS boot components. The aim for this project is to match the features of img4tool and incorporate features from other tools so img4helper can be a single tool that does the job of many.

The following features are currently implemented and require testing:
 * Decryption of firmware files.
 * Decompressing bvx2 and lzss compressed files.
 * Decrypting/Decompressing .im4p's (Tested).
 * Decrypting/Decompressing a full .img4 (Not tested).
 * Component detection.
 * Printing all information from an img4, im4p, im4m or im4r.
* Device tree parsing

The following features I would like to add:
 * Constructing Image4 files
 * (Requires libhelper-macho) Identification for component version, e.g. Darwin Kernel Version 19.0.0, xnu-4903.270.47~7, etc
 * Extracting firmware keys from devices using Checkra1n/Checkm8
 * Kernel and iBoot analysis. (Symbols, etc)

## Releases

I plan to release compiled versions of img4helper. Currently, I do not compile img4helper with glib and lzfse, so one must first have both installed. If you are compiling, please also add lzfse to your PKG_CONFIG_PATH.

I have had some issues with OpenSSL, so img4helper uses CommonCrypto, which is default on Apple systems, but not on Linux. So if you would like to use img4helper on Linux, some help with OpenSSL would be massively appreciated.

The most recent version can be downloaded either from Github, or from the following link. There will be a downloads API added eventually so you can query the latest version and/or platform.
```
https://s3.cloud-itouk.org/dnlds/releases/img4helper/img4helper-darwinx86-latest.zip
```

## Building

Img4helper uses Meson as it's build system. It also relies on glib, lzfse and OpenSSL/CommonCrypto. Please make sure you have these properly setup and installed.

You can build Img4helper with the following commands:
```
$ meson build
$ cd build/ && ninja
```

Img4helper can then be run as follows:
```
$ ./src/img4helper --help
```

Any issues tweet @h3adsh0tzz, or submit an issue and i'll get back to you!
