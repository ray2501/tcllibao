tcllibao
=====

[libao](https://www.xiph.org/ao/) is a cross-platform audio library
that allows programs to output audio using a simple API on a wide
variety of platforms.

libao is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

It is a Tcl bindings for libao.


License
=====

GNU General Public License version 2, or (at your option) any later version.


Commands
=====

libao::ao initialize  
libao::ao default_id  
libao::ao driver_id short_name  
libao::ao open_live id ?-bits bits? ?-rate samplerate? ?-channels channels? ?-byteformat format?  
libao::ao open_file id filename ?-overwrite overwrite? ?-bits bits? ?-rate samplerate? ?-channels channels? ?-byteformat format?  
libao::ao play byte_array  
libao::ao close  
libao::ao shutdown

The `short_name` you can check
[https://www.xiph.org/ao/doc/drivers.html](https://www.xiph.org/ao/doc/drivers.html).

open_live and open_file `-byteformat` have 3 values.
1 is Little Endian, 2 is Big Endian, and 4 is NATIVE.
If user does not specify, the default value is NATIVE.

open_file `-overwrite` option if non-zero, the file is automatically
overwritten. If zero, then a preexisting file will cause the function
to report a failure.


UNIX BUILD
=====

I only test tcllibao under openSUSE LEAP 42.2 and Ubuntu 14.04.

Users need install libao development files. Below is an example for openSUSE:

	sudo zypper in libao-devel

Below is an example for Ubuntu:

	sudo apt-get install libao-dev

Building under most UNIX systems is easy, just run the configure script
and then run make. For more information about the build process, see the
tcl/unix/README file in the Tcl src dist. The following minimal example
will install the extension in the /opt/tcl directory.

	$ cd tcllibao
	$ ./configure --prefix=/opt/tcl
	$ make
	$ make install

If you need setup directory containing tcl configuration (tclConfig.sh),
below is an example:

	$ cd tcllibao
	$ ./configure --with-tcl=/opt/activetcl/lib
	$ make
	$ make install

WINDOWS BUILD
=====

##MSYS2/MinGW-W64

First step is to download the libao source package.

If libao version is <= 1.2.0, users need apply 2 patches to fix configure and build failed
([patch1](https://github.com/xiph/libao/commit/748ec5ecd7b733a1f5ae4447b4451487bc5b0bef),
[patch2](https://cygwin.com/ml/cygwin/2014-03/txtg9LhZUvVP5.txt)).

Below is the `configure.ac` diff result (after patch and before):

	227c227
	< AS_IF([test "x$enable_wmm" != "xno"],
	---
	> AS_IF([AC_LANG_SOURCE([test "x$enable_wmm" != "xno"],
	231c231
	<    waveout_old_LIBS="$LIBS"; LIBS="$LIBS -lwinmm -Wl,-lksguid"
	---
	>    waveout_old_LIBS="$LIBS"; LIBS="$LIBS -lwinmm"
	233c233
	<    AC_LINK_IFELSE([AC_LANG_SOURCE([
	---
	>    AC_LINK_IFELSE([

Now build libao:

	autoconf
	./configure --enable-wmm=yes --prefix=/c/msys64/mingw64
	make
	make install

Put libao-4.dll to Windows folder or other available folder.

Next step is to build tcllibao:

	./configure --with-tcl=/c/tcl/lib
	make
	make install

Example
=====

Cowork with [tclsndfile](https://github.com/ray2501/tclsndfile).

    #
    # Using libao and libsndfile to play a wav/ogg file
    #

    package require libao
    package require sndfile

    if {$argc > 0} {
        set name [lindex $argv 0]
    } else {
        puts "Please input filename."
        exit
    }

    if {[catch {set data [sndfile snd0 $name READ]}]} {
        puts "sndfile: read file failed."
        exit
    }
    set encoding [dict get $data encoding]

    # only for test seek
    snd0 seek 0 SET

    libao::ao initialize
    set id [libao::ao default_id]

    set bits 16
    switch $encoding {
      {pcm_16} {
          set bits 16
      }
      {pcm_24} {
          set bits 24
      }
      {pcm_32} {
          set bits 32
      }
      {pcm_s8} {
          set bits 8
      }
      {pcm_u8} {
          set bits 8
      }
      default {
          set bits 16
      }
    }

    libao::ao open_live $id -bits $bits \
      -rate [dict get $data samplerate] \
      -channels [dict get $data channels] -byteformat 4

    # libao needs use read_short to get data
    while {[catch {set buffer [snd0 read_short]}] == 0} {
        libao::ao play $buffer
    }

    snd0 close
    libao::ao close
    libao::ao shutdown

Cowork with [tclmpg123](https://github.com/ray2501/tclmpg123).

    #
    # Using libao and libmpg123 to play a mp3 file
    #

    package require libao
    package require mpg123

    if {$argc > 0} {
        set name [lindex $argv 0]
    } else {
        puts "Please input filename."
        exit
    }

    if {[catch {set data [mpg123 mpg0 $name]}]} {
        puts "mpg123: read file failed."
        exit
    }
    set bits [dict get $data bits]

    # only for test seek function
    mpg0 seek 0 SET

    libao::ao initialize
    set id [libao::ao default_id]

    libao::ao open_live $id -bits $bits \
      -rate [dict get $data samplerate] \
      -channels [dict get $data channels]

    while {[catch {set buffer [mpg0 read]}] == 0} {
        libao::ao play $buffer
    }

    mpg0 close
    libao::ao close
    libao::ao shutdown

