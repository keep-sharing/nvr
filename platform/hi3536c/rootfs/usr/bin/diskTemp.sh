#! /bin/sh

last_disk_temp=0
for i in `lsscsi | awk '{print $NF}'`
do
	#echo $i
	disk_temp=`smartctl -A $i | grep ^194 | awk '{print $10}'`
	#echo $disk_temp
	if [ -z $disk_temp ]; then
		continue
	fi

	if [ $disk_temp -gt $last_disk_temp ]; then
		last_disk_temp=$disk_temp
	fi
done
echo "The max temperature : $last_disk_temp"
