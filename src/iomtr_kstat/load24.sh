#!/bin/sh
module="iomtr_kstat"
device="iomtr_kstat"
mode="664"

# Group: since distributions do it differently, look for wheel or use staff
if grep '^staff:' /etc/group > /dev/null; then
    group="staff"
else
    group="wheel"
fi

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod $module.o $* || exit 1

major=`cat /proc/devices | awk "\\$2==\"$module\" {print \\$1}"`

# Remove stale nodes and replace them, then give gid and perms
rm -f /dev/${device}0
rm -f /dev/${device}
mknod /dev/${device}0 c $major 0
ln -sf ${device}0 /dev/${device}
chgrp $group /dev/${device}
chmod $mode  /dev/${device}

