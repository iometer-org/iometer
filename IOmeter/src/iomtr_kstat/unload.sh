#!/bin/sh
module="iomtr_kstat"
device="iomtr_kstat"

# invoke rmmod with all arguments we got
/sbin/rmmod $module $* || exit 1

# Remove stale nodes
rm -f /dev/${device} /dev/${device}0
