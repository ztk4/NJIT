#!/bin/bash

# First try inserting and removing each module with default parameters
for module in *.ko; do
  insmod $module
  sleep 1
  rmmod $module
done

# Now try the third module with custom args
insmod hello3.ko recipient='"Zachary Kaplan"' addr_lines='"141 Warren Street","Room #329A"' city="Newark" zip=7103
sleep 1
rmmod hello3.ko
