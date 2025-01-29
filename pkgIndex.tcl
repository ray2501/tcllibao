# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded libao 0.5 \
	    [list load [file join $dir libtcl9libao0.5.so] [string totitle libao]]
} else {
    package ifneeded libao 0.5 \
	    [list load [file join $dir liblibao0.5.so] [string totitle libao]]
}
