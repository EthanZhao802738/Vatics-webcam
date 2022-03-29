#!/bin/bash
make clean
make
arm-linux-strip test
rm /mnt/nfs_share/test
cp test /mnt/nfs_share/
