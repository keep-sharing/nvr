#! /bin/sh

echo -e "start application"
export LD_LIBRARY_PATH=/lib:/usr/lib:/opt/app/libs
export QT_QWS_FONTDIR=/opt/app/bin/fonts

#echo 8192 > /proc/sys/vm/min_free_kbytes
#echo 200 > /proc/sys/vm/vfs_cache_pressure

#sysctl -w net.core.rmem_default=163840
#sysctl -w net.core.rmem_max=163840
#sysctl -w net.core.wmem_default=163840
#sysctl -w net.core.wmem_max=163840

#echo "1" > /proc/sys/vm/overcommit_memory

#############################################
##########设置硬盘超时5分钟后睡眠
#disks=`lsscsi | awk '{print $NF}'`
#for hdd in $disks
#do
#	echo "hddparm $hdd"
#	/sbin/hdparm -S 60 $hdd
#	smartctl -s on $hdd #开启smart功能
#done
#############################################
date '+%Y-%m-%d %X' > /tmp/uptime.txt
sysconf -s sysinit
sysconf -s network network
#sysconf -s network pppoe
#sysconf -s network ddns
#sysconf -s network mail
#ntpd -f /etc/ntp.conf & 

#sample_hifb 0 & #bruce debug
daemon & 
mscore &
sleep 1
gui -qws &

#sleep 8
#fio &
#sleep 8
#recosdk &
#sleep 2
mssp &
#ntpq -p > /dev/null
