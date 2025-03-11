#!/bin/sh
 
MNT_PATH=/mnt

MNT_DIR=
SD1_DIR=sd
EMMC1_DIR=emmc1
EMMC2_DIR=emmc2
CACHE_DIR=cache
DRIVE_A=""
DRIVE_B=""

my_umount()
{
	FOLDER=`grep "/dev/$1" /proc/mounts | cut -d ' ' -f 2`
	if [ ! -z "$FOLDER" ]; then
		umount -l "$FOLDER";
	fi
}
 
my_mount()
{
	MMCBUSNUM=`echo $DEVPATH | cut -d '/' -f 4`
	MNT_DIR=$SD1_DIR

	if [ -b /dev/$1 ]; then
		MOUNTDEV="/dev/$1"
		if [ -b "/dev/$1p1" ]; then
			MOUNTDEV="/dev/$1p1"
		fi
	fi

	time_offset_sig=`date +%z | cut -c 1`
	time_offset_h=`date +%z | cut -c 2-3`
	time_offset_m=`date +%z | cut -c 4-5`
	time_offset=`expr $local_time - $utc_time`
	if [ $time_offset_sig == + ]; then
		time_offset_sig="";
	fi
	time_offset_total_m=$time_offset_sig`expr $time_offset_h \* 60 + $time_offset_m`

	mkdir -p "${MNT_PATH}/${MNT_DIR}" || exit 1
	fat_type=`blkid "$MOUNTDEV" | awk -F'TYPE=' '{print $NF}'`
	if [ "${fat_type}" == "\"vfat\"" ]; then
		if ! mount -o dirsync,time_offset=$time_offset_total_m "$MOUNTDEV" "${MNT_PATH}/${MNT_DIR}" 2>&1 | tee -a /tmp/mountstat; then
			echo "$MOUNTDEV $MNT_PATH/$MNT_DIR ignore defaults 0 0" >> /tmp/.nvt_mounts
			return 1
		fi
	elif [ "${fat_type}" == "\"exfat\"" ]; then
		if ! mount -t exfat "$MOUNTDEV" "${MNT_PATH}/${MNT_DIR}" 2>&1 | tee -a /tmp/mountstat; then
			echo "$MOUNTDEV $MNT_PATH/$MNT_DIR ignore defaults 0 0" >> /tmp/.nvt_mounts
			continue
		fi
	else
		echo "$MOUNTDEV FAIL" >> /tmp/.nvt_mounts
		return 1
	fi
}

check_mmc_ready()
{
	MMCBUSPATH="/sys/bus/mmc/devices/"
	x=0
	timeout=100
	while [ "$x" -lt "$timeout" -a ! -d $MMCBUSPATH ]; do
		x=$((x+1))
		sleep .1
	done
	if [ "$x" -ge "$timeout" ]; then
		return -1;
	fi
	return 0;
}

if [ -z $DEVPATH ]; then
	# This is for boot stage handling
	MMCBUSPATH="/sys/bus/mmc/devices/"
	check_mmc_ready
	if [ $? != 0 ]; then
		exit -1;
	fi
	MMCDEVLIST=`ls $MMCBUSPATH`
	time_offset_sig=`date +%z | cut -c 1`
	time_offset_h=`date +%z | cut -c 2-3`
	time_offset_m=`date +%z | cut -c 4-5`
	time_offset=`expr $local_time - $utc_time`
	if [ $time_offset_sig == + ]; then
		time_offset_sig="";
	fi
	time_offset_total_m=$time_offset_sig`expr $time_offset_h \* 60 + $time_offset_m`

	for n in $MMCDEVLIST
	do
		# Check if it is not SD device
		SD_TYPE=`cat $MMCBUSPATH/$n/type`
		if [ $SD_TYPE == SDIO ]; then
			continue
		fi

		if [ $SD_TYPE == MMC ]; then
			# To check if it's the emmc storage device
			if [ -f $MMCBUSPATH/$n/bga ]; then
				BGA=`cat $MMCBUSPATH/$n/bga`
				if [ 1 == $BGA ]; then
					# Get the block device name
					BLOCKDEV=`ls $MMCBUSPATH/$n/block`

					# Check if device is mounted
					MOUNTED=`grep $BLOCKDEV /proc/mounts`
					if [ ! -z "$MOUNTED" ]; then
						continue
					fi

					if [ "$NVT_ROOTFS_TYPE" == "NVT_ROOTFS_TYPE_EMMC" ]; then
						if [ -b "/dev/${BLOCKDEV}p2" ]; then
							if ! mount -t vfat /dev/${BLOCKDEV}p2 /mnt/${CACHE_DIR}; then
								echo yes | mkfs.vfat /dev/${BLOCKDEV}p2;
								if ! mount -t vfat /dev/${BLOCKDEV}p2 /mnt/${CACHE_DIR}; then
									exit 1;
								fi
							fi
						fi
						if [ -b "/dev/${BLOCKDEV}p5" ]; then
							if ! mount -t ext4 /dev/${BLOCKDEV}p5 /mnt/${EMMC1_DIR}; then
								echo yes | mkfs.ext4 /dev/${BLOCKDEV}p5;
								if ! mount -t ext4 /dev/${BLOCKDEV}p5 /mnt/${EMMC1_DIR}; then
									exit 1;
								fi
							fi
						fi
						if [ -b "/dev/${BLOCKDEV}p6" ]; then
							if ! mount -t ext4 /dev/${BLOCKDEV}p6 /mnt/${EMMC2_DIR}; then
								echo yes | mkfs.ext4 /dev/${BLOCKDEV}p6;
								if ! mount -t ext4 /dev/${BLOCKDEV}p6 /mnt/${EMMC2_DIR}; then
									exit 1;
								fi
							fi
						fi
					else
						# Using fdisk to check if it needs to be partitioned
						if [ -f /etc/autofdisk.sh ]; then
							mknod /dev/${BLOCKDEV} b `cat /sys/block/${BLOCKDEV}/dev | sed "s/:/\ /g"`
							/etc/autofdisk.sh ${BLOCKDEV}
							if [ $? != 0 ]; then
								echo -e "\e[1;31m\rUpdate rootfs failed. #1\r\e[0m"
								exit 1;
							fi
						fi
						sync
						sleep 1
						if [ -b "/dev/${BLOCKDEV}p1" ]; then
							if ! mount -t ext4 /dev/${BLOCKDEV}p1 /mnt/${EMMC1_DIR}; then
								echo yes | mkfs.ext4 /dev/${BLOCKDEV}p1;
								if ! mount -t ext4 /dev/${BLOCKDEV}p1 /mnt/${EMMC1_DIR}; then
									exit 1;
								fi
							fi
						fi
						if [ -b "/dev/${BLOCKDEV}p2" ]; then
							if ! mount -t ext4 /dev/${BLOCKDEV}p2 /mnt/${EMMC2_DIR}; then
								echo yes | mkfs.ext4 /dev/${BLOCKDEV}p2;
								if ! mount -t ext4 /dev/${BLOCKDEV}p2 /mnt/${EMMC2_DIR}; then
									exit 1;
								fi
							fi
						fi
					fi
				fi
			fi
			continue
		fi

		# Get the block device name
		BLOCKDEV=`ls $MMCBUSPATH/$n/block`
		# Check if device is mounted
		MOUNTED=`grep $BLOCKDEV /proc/mounts`

		# Create folder
		MNT_DIR=$SD1_DIR

		if [ ! -z "$MOUNTED" ]; then
			continue
		fi

		# Check if /dev/mmcblk* exists
		if [ -b /dev/$BLOCKDEV ]; then
			MOUNTDEV="/dev/$BLOCKDEV"
			if [ -b "/dev/${BLOCKDEV}p1" ]; then
				MOUNTDEV="/dev/${BLOCKDEV}p1"
			fi
		else
			continue
		fi

		# Inserted but can't be mounted
		fat_type=`blkid "$MOUNTDEV" | awk -F'TYPE=' '{print $NF}'`
		if [ "${fat_type}" == "\"vfat\"" ]; then
			if ! mount -o dirsync,time_offset=$time_offset_total_m "$MOUNTDEV" "${MNT_PATH}/${MNT_DIR}" 2>&1 | tee -a /tmp/mountstat; then
				echo "$MOUNTDEV $MNT_PATH/$MNT_DIR ignore defaults 0 0" >> /tmp/.nvt_mounts
				continue
			fi
		elif [ "${fat_type}" == "\"exfat\"" ]; then
			if ! mount -t exfat "$MOUNTDEV" "${MNT_PATH}/${MNT_DIR}" 2>&1 | tee -a /tmp/mountstat; then
				echo "$MOUNTDEV $MNT_PATH/$MNT_DIR ignore defaults 0 0" >> /tmp/.nvt_mounts
				continue
			fi
		else
			echo "Unkown SD type!!!"
			echo "$MOUNTDEV $MNT_PATH/$MNT_DIR ignore defaults 0 0" >> /tmp/.nvt_mounts
			continue
		fi

	done
	touch /tmp/.nvt_mounts
else
	# This is for booted up stage
	case "${ACTION}" in
	add|"")
		my_umount ${MDEV}
		my_mount ${MDEV}
		;;
	remove)
		my_umount ${MDEV}
		;;
	esac
fi
