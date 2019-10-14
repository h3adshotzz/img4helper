# img4helper

Based on img4tool by Tihmstar. The primary purpose of img4helper was to improve my understanding of the Image4 file format and iOS boot components. The aim for this project is to match the features of img4tool and incorporate features from other tools so img4helper can be a single tool that does the job of many.

My current plan is to add the following features:
 * Decrypt / Decompress Image4 files
 * Extract / Construct Image4's
 * Identify payload type
 * Analysis for different payload types (Kernel, iBoot, etc)

Other features that may either be built into another tool, or just cause this tool to be renamed:
 * TSS Checker
 * IPSW Downloads via https://api.ipswcentral.com/
 * SecureROM & iBoot dumps via Checkra1n/Checkm8
 * Firmware restore / update.

## Building

Img4helper uses Meson as it's build system. It also relies on glib, lzfse and openssl. Please make sure you have these properly setup and installed.

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
