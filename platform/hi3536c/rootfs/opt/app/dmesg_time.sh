#! /bin/sh

echo 1 >/sys/module/printk/parameters/time
let i=$(date +%s)-$(echo `awk -F[.] '{print $1}' /proc/uptime`)+"dmesg_time";date -d @$i "+%Y-%m-%d %H:%M:%S"
