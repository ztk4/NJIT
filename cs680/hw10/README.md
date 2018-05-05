# Homework 8

### pkt\_cntr\_low\_traffic.txt
The contents of /proc/pkt\_cntr when there's no particular network traffic.


### pkt\_cntr\_download.txt
The contents of /proc/pkt\_cntr when there's a youtube video streaming at 1080p.

### pkt\_cntr\_upload.txt
The contents of /proc/pkt\_cntr while uploading a 2GB archive to Google drive.

### FILENAME\_dddd\_DDDD.txt
These files are regions of the kernel source files I edited for this assignment.
Each file indicates which kernel source file the region is taken from, as well as the line numbers.

### KBuild & Makefile
These are the build files for the module. Compilation commands are `make` to build and `make install`
to put in the module path. The latter of course requires root privelages.
Note that after installation, `modprobe pkt_cntr` is need to insert the module, and `modprobe -r pkt_cntr`
can be used to remove it.

### pkt\_cntr.c
The source for the kernel module.

NOTE: All files are derived from kernel version 4.15-rc8.
