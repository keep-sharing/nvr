#! /bin/sh

echo 16384 > /proc/sys/vm/min_free_kbytes
echo 200 > /proc/sys/vm/vfs_cache_pressure
echo 0 > /proc/sys/net/ipv4/tcp_sack

echo e > /proc/irq/55/smp_affinity
echo e > /sys/class/net/eth0/queues/rx-0/rps_cpus
echo e > /sys/class/net/eth1/queues/rx-0/rps_cpus

echo 4096 87380 16777216 > /proc/sys/net/ipv4/tcp_rmem 
echo 4096 65536 16777216 > /proc/sys/net/ipv4/tcp_wmem 
echo 60 > /proc/sys/net/ipv4/tcp_fin_timeout
echo 0 > /proc/sys/net/ipv4/tcp_tw_recycle
echo 0 > /proc/sys/net/ipv4/tcp_window_scaling 
echo 1 > /proc/sys/net/ipv4/tcp_no_metrics_save 
echo 262144 > /proc/sys/net/ipv4/tcp_max_orphans
echo 655360 > /proc/sys/net/core/rmem_max

echo 0 > /proc/sys/net/ipv4/tcp_syncookies
echo 2 > /proc/sys/net/ipv4/tcp_synack_retries
echo 4096 > /proc/sys/net/ipv4/tcp_max_syn_backlog
echo 4096 > /proc/sys/net/core/somaxconn

echo 2 > /proc/irq/16/smp_affinity
echo 2 > /proc/irq/22/smp_affinity
echo 2 > /proc/irq/34/smp_affinity
echo 2 > /proc/irq/51/smp_affinity
echo 2 > /proc/irq/54/smp_affinity

echo -e "start application"
#############################################

hdbase="/dev/sd"

disks=`lsscsi | awk '{print $NF}'`
for hdd in $disks
do
	echo "$hdd" | grep -v "$hdbase" && continue
	echo "hdparm & smartctl -->  $hdd"
	#/sbin/hdparm -S 60 $hdd  #zbing modify 20170113
	/sbin/hdparm -S 0 $hdd | grep 'Input/output error' && echo $hdd >> /tmp/health
	smartctl -s on $hdd | grep 'hardware error' && echo $hdd >> /tmp/health
done

#############################################
date '+%Y-%m-%d %X' > /tmp/uptime.txt
mkdir -p /mnt/nand/app
mkdir -p /mnt/nand/etc
mkdir -p /mnt/nand/etc/smtp

#clear all nas files
rm -fr /mnt/nas_*

ifdown -a
sysconf -s sysinit
sysconf -s network network
sysconf -s network hostname
ifup -a
#/usr/sbin/telnetd & #bruce debug
#sysconf -s network pppoe
dropbear -p 22 &
sysconf -s network pppoe #hrz.milesight add for pppoe
sysconf -s network ddns
sysconf -s network mail
sysconf -s time &
sysconf -s network snmp #hrz.milesight add for snmp
sysconf -s network nginx #yun.milesight add for nginx
#sleep 2
ethtool -K eth0 tso on
ethtool -K eth0 gso on

#showlogo /opt/app/bin/logo.jpg #& #clean the background(uboot load image)
#daemon & 
#killall -9 showlogo
if [ -f "/etc/gdb.enable" ]; then
   echo "######### Start GDB Mode ########"
   #nohup gdb mscore -x /etc/gdb.cmd >/dev/null 2>&1 &
   gdb mscore -x /etc/gdb.cmd
else
   echo "######### Start Relese Mode ########"
   mscore -qws &
fi
#update -B 1 #set boot state is OK!
#gui -qws &
