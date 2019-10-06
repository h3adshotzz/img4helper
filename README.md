# img4helper

Slightly less good version of img4tool. The purpose of img4helper is to improve my understanding of the Image4 file format. I aim to have the same set of features as img4tool, and possibly even image patching for iBoot and Kernelcache.

## Building

img4helper uses Meson for the build system, and relies on glib. You should be able to build by running the following:
```
$ meson build
$ cd build/ && ninja
```

Run with:
```
$ ./src/img4helper --args
```

Any issues tweet @h3adsh0tzz, or submit an issue and i'll get back to you!
