#!/bin/sh

###############################
# hjh.milesight 20151209 add for sata and usb hotplug
################################

################### SCSI disk #################################################
#DEVTYPE=partition 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb/sdb2 
#SUBSYSTEM=block 
#SEQNUM=800 
#UDEVD_EVENT=1 
#ACTION=remove 
#DEVNAME=/dev/sdb1 
#DEVTYPE=partition 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb/sdb1 
#SUBSYSTEM=block 
#SEQNUM=801 
#UDEVD_EVENT=1 
#ACTION=remove 
#DEVNAME=/dev/sdb 
#DEVTYPE=disk 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb 
#SUBSYSTEM=block 
#SEQNUM=803 
#UDEVD_EVENT=1 

################### USB disk #################################################
#
# Block device event:
#
# ACTION=add 
# DEVNAME=/dev/sda 
# DEVTYPE=disk 
# DEVPATH=/devices/platform/hiusb-ehci.0/usb1/1-2/1-2.1/1-2.1:1.0/host8/target8:0:0/8:0:0:0/block/sda 
# SUBSYSTEM=block 
# SEQNUM=544 
# UDEVD_EVENT=1 
# 
# ACTION=add 
# DEVNAME=/dev/sda1 
# DEVTYPE=partition 
# DEVPATH=/devices/platform/hiusb-ehci.0/usb1/1-2/1-2.1/1-2.1:1.0/host8/target8:0:0/8:0:0:0/block/sda/sda1 
# SUBSYSTEM=block 
# SEQNUM=545 
# UDEVD_EVENT=1 
#
# Use command "hdparm -z /dev/sda" to reread partition table
#

################################################################################
PREFIX=udisk
CONSOLE=/dev/ttyS000
HDDMAPFILE="/etc/udev/hdd_map.cfg"
MOUNT="/bin/mount"
UMOUNT="/bin/umount"
NTFSMOUNT="/bin/ntfs-3g"
ESATAMNT="/media/esata"
name="`basename "$DEVNAME"`"
PROCESS="mscore"
hubNum=`dmesg | grep -c "Port Multiplier"`
if [ ! -e $HDDMAPFILE -o ! -s $HDDMAPFILE ];then
	case $hubNum in
		2)
			echo "two hdd hubs" > ${CONSOLE} 
			cp -a /etc/udev/hdd_8_map.cfg $HDDMAPFILE 
	  	;;
		1)
			echo "one hdd hub" > ${CONSOLE}
	 	 	cp -a /etc/udev/hdd_4_map.cfg $HDDMAPFILE
	 	;;
		*)
			echo "none hdd hub" > ${CONSOLE}
			cp -a /etc/udev/hdd_2_map.cfg $HDDMAPFILE
		;;
	esac
fi
################################################################################
show_env ()
{
	local RED="\033[31m"
	local NORMAL="\033[00m"
	{
		echo -e ${RED}"ACTION=$ACTION"           ${NORMAL}
		echo -e ${RED}"DEVNAME=$DEVNAME"         ${NORMAL}
		echo -e ${RED}"DEVTYPE=$DEVTYPE"         ${NORMAL}
		echo -e ${RED}"DEVPATH=$DEVPATH"         ${NORMAL}
		echo -e ${RED}"SUBSYSTEM=$SUBSYSTEM"     ${NORMAL}
		echo -e ${RED}"SEQNUM=$SEQNUM"           ${NORMAL}
		echo -e ${RED}"UDEVD_EVENT=$UDEVD_EVENT" ${NORMAL}
	} > ${CONSOLE}
}

rm_dir() 
{
	if test "`find "$1" | wc -l | tr -d " "`" -lt 2 -a -d "$1"
	then
		! test -z "$1" && rm -r "$1"
	else
		logger "mount.sh/automount" "Not removing non-empty directory [$1]"
	fi
}

###############################################################################
#get NTFS type, "Basic data" contain NTFS and microsoft filesystem.
get_fs_type()
{
	local no_part=`echo $DEVNAME | sed -e 's/[0-9]//g'`
	local ntfs_part=`/opt/app/bin/parted $no_part print | grep "ntfs" | awk '{print $1}'`
	echo $ntfs_part
}
##############################################################################

################################################################################
#  add scsi disk 
add_disk () 
{
	local part_index=$1
	local dev_column=$2


	local global_name=`echo $DEVNAME | sed -e 's/[0-9]//g'`
	
	local main_name=`echo $DEVNAME | cut -f3 -d '/' | sed -e 's/[0-9]//g'`

	sed -i  '/'"$main_name"'/d' $HEALTH_PATH

	/sbin/hdparm -S 0 $global_name | grep 'Input/output error' && echo $global_name >> $HEALTH_PATH && return

	smartctl -s on $global_name | grep 'hardware error' && echo $global_name >> $HEALTH_PATH && return

	if [ "$part_index" = "" ]; then #hjh.milesight 20160506 modify, no part not be supported
		return
	fi

	local dev_host=`echo $dev_column | cut -f1 -d':'`
	local dev_channel=`echo $dev_column | cut -f2 -d':'`

	cat $HDDMAPFILE | grep -v '^[#t]' | awk -F ':' '{print $1, $4}' | while read line
	do
			local column=`echo $line | awk '{print $1}'`
			local mnt_path=`echo $line | awk '{print $2}'`
			local host=`echo $column | cut -f1 -d'-'`
			local channel=`echo $column | cut -f2 -d'-'`

			if [ "$host" -eq "$dev_host" -a "$channel" -eq "$dev_channel" ]; then
				mkdir -p $mnt_path
				if [ $mnt_path != $ESATAMNT ]; then
					if [ "$part_index" != "1" ]; then
						return
					fi
					$MOUNT -t ext4  -o rw,async,noatime,nodiratime,barrier=0 $DEVNAME $mnt_path
				else
					local fs_part=`get_fs_type`
					if [ $fs_part == $part_index ]; then
						$NTFSMOUNT $DEVNAME $mnt_path
					else					
						$MOUNT  $DEVNAME $mnt_path
					fi
				fi
				break
#			else
#				echo "no mount !!!" > ${CONSOLE}
			fi
	done

	pid=`ps | grep $PROCESS | head -1 | awk '{print $1}'`
	kill -s SIGUSR1 $pid

}

###########################################################################################
# add usb devices
add_usb ()
{
	local dev_host=$1
	local dev_channel=$2
	local part_index=$3

#	echo "$part_index" > ${CONSOLE}
	if [ "$part_index" == "" ]; then #hjh.milesight 20160505 modify,for usb disk and USB sata
		return
	fi
#	local dev_host=`echo $dev_column | cut -f1 -d':'`
#	local dev_channel=`echo $dev_column | cut -f2 -d':'`
#	echo "dev_column: ${dev_column}" > ${CONSOLE}
#	echo "dev_host: ${dev_host},dev_channel: ${dev_channel}" > ${CONSOLE}

	cat $HDDMAPFILE | grep -v '^[#t]' | awk -F ':' '{print $1, $4}' | while read line
	do
			local map=`echo $line | awk '{print $1}'`
			local mnt_path=`echo $line | awk '{print $2}'`
			local dev_type=`echo $map | cut -f1 -d'-'`
			local host_channel=`echo $map | cut -f2 -d'-'`
			local host=`echo $host_channel | cut -f1 -d'.'`
			local channel=`echo $host_channel | cut -f2 -d'.'`
			echo "map: ${map},type: ${dev_type},host: ${host},channel:${channel}" > ${CONSOLE}
			if [ "$dev_type" == "u" -a  "$host" -eq "$dev_host" -a "$channel" -eq "$dev_channel" ]; then
				mkdir -p $mnt_path                                                                                         
				local fs_part=`get_fs_type`
				if [ $fs_part == $part_index ]; then
					echo "============= NTFS ,fs_part:${fs_part}, part_index:${part_index}========================" > ${CONSOLE}
					$NTFSMOUNT $DEVNAME $mnt_path
				else					
					echo "============= NOT NTFS ,fs_part:${fs_part}, part_index:${part_index}========================" > ${CONSOLE}
					$MOUNT  $DEVNAME $mnt_path
				fi
				break
			fi
	done

	pid=`ps | grep $PROCESS | head -1 | awk '{print $1}'`
	sleep 1
	kill -s SIGUSR1 $pid
}


################################################################################
# remove sata and usb 
remove_disk ()
{
	for mnt in `cat /proc/mounts | grep "$DEVNAME" | cut -f 2 -d " " `
	do
		$UMOUNT -l $mnt
	done

	#local guid=`ps |  grep gui |head -1 | awk '{print $1}'`
	#sleep 1
	#kill -s SIGUSR2 $guid

	test -e "/tmp/.automount-$name" && rm_dir "/media/$name"

}
################################################################################
# hotplug_usb 
hotplug_usb ()
{
	if [ ss"${SUBSYSTEM}" != ss"block" ]; then
		return 1
	fi

	if [ ss"$(echo ${DEVPATH} | grep -r "usb")" = ss"" ]; then
		return 1
	fi

	local HOST=$(echo ${DEVPATH:41:1})
	local CHANNEL=$(echo ${DEVPATH:43:1})

	local PART_INDEX=`echo $DEVNAME | sed -e 's/\/dev\/sd.//'`

	case "${ACTION}" in
	"add"|"change"    )
		echo "==${DEVPATH}=${HOST} ${CHANNEL}====add usb====" > $CONSOLE 
		add_usb "${HOST}" "${CHANNEL}" "${PART_INDEX}"
	;;
	"remove" )
		remove_disk 
	;;
	* )
		echo "ACTION:${ACTION}" > ${CONSOLE}
	;;
	esac

	return 0
}

######################################################################
# hotplug_sata 
hotplug_sata ()
{

	if [ ss"${SUBSYSTEM}" != ss"block" ]; then
		return 1
	fi

	if [ ss"$(echo ${DEVPATH} | grep -r "ata")" = ss"" ]; then
		return 1
	fi

	local DEV_COLUMN=$(echo ${DEVPATH} | sed -e 's/\/block.*//g')
	DEV_COLUMN=$(echo ${DEV_COLUMN} | sed -e 's/\/devices.*\///g')
	local PART_INDEX=`echo $DEVNAME | sed -e 's/\/dev\/sd.//'`

	case "${ACTION}" in
	"add"|"change"    )
		add_disk   "${PART_INDEX}" "${DEV_COLUMN}"
	;;
	"remove" )
		remove_disk 
	;;
	* )
		echo "ACTION:${ACTION}" > ${CONSOLE}
	;;
	esac

	return 0
}
################################################################################
#show_env
hotplug_usb
hotplug_sata
