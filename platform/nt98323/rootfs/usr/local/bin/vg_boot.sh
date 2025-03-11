#boot_ver=v1.5
#chipver=`head -1 /proc/pmu/chipver`
#chipid=`echo $chipver | cut -c 1-4`
echo -----------------------------------------------------------
echo "        Boot NVR_8CH_max"
echo -----------------------------------------------------------
echo "/sbin/mdev" > /proc/sys/kernel/hotplug

mem=`awk '($1 == "MemTotal:"){print $2}' /proc/meminfo`
LOAD_DIR=/usr/local/bin
MODEL_PATH=$(dirname $MODEL)
MODEL_NAME="${MODEL_PATH##*/}"
echo This model is $MODEL_NAME

modprobe nvt_dmasys
modprobe log log_ksize=4096 crash_notify=/usr/local/bin/crash.sh	#crash to execute /usr/local/bin/crash.sh
echo /tmp > /proc/videograph/dumplog		#change log path to /tmp

modprobe ms max_channels=8
modprobe em max_channels=8

modprobe lcd_codec
modprobe nvt_tve100
modprobe nvt_hdmi20
modprobe flcd300-common
modprobe flcd300-pip suspend_state=1 
modprobe flcd200-common
modprobe flcd200-pip suspend_state=1

modprobe vpe321_drv
modprobe kflow_vpe mod_init=1 max_chip_num=1 max_eng_num=1 max_minor_num=255 max_md_lv_num=16 max_total_cam_ch=45

modprobe nvt_audio
modprobe kflow_audio

modprobe dei321_drv
modprobe kflow_di max_chip_num=1 max_eng_num=1 max_minor_num=64 max_total_cam_ch=36 max_h_size=960

modprobe kdrv_jpg
modprobe h26xenc h26x_enc_max_width=3840 h26x_enc_max_height=2164 max_total_cam_ch=45
modprobe nvt_vencrc
modprobe kflow_videoenc
modprobe h26xdec h26xd_max_width=4096 h26xd_max_height=4096 max_total_cam_ch=20
modprobe kflow_videodec

modprobe kdrv_osg
modprobe kflow_osg

modprobe gm2d
modprobe gs max_channels=8 
modprobe usr_proc
modprobe vpd quiet=1 max_channels=8
modprobe ddr_arb
echo doing mdev-s
mdev -s
echo done!!

#insmod nvt ko
insmod nvt_ivot_wdt.ko
insmod gpio_nvt.ko
insmod fb_ctrl_nvt.ko
insmod drv_io_nvt.ko
insmod ip181x_switch_nvt.ko
insmod nau88c10_nvt.ko
insmod rtc_pcf8563_nvt.ko 
insmod rtl8309n_switch_nvt.ko
insmod ip179h_switch_nvt.ko
hwclock -us

if [ $mem -lt 512000 ]; then
	/usr/local/bin/module_init /usr/local/bin/cfg_DVR_4CH_max.dtb 0 0 1 1 0 &
else
	/usr/local/bin/module_init /usr/local/bin/cfg_DVR_8CH_max.dtb 0 0 1 1 0 &
fi
sleep 2
# ch0: MCP100, VCAP316_M2, OSG, VPE316
# ch1: LCD, HEAVYLOAD
# ch2: VDEC
# ch3: VPE536, SSCA200, HEAVYLOAD_2
# ch4: CNN, NUE, NUE2, TSDE, HEAVLOAD_1
# ch5: CPU, DSP, DMA030
# ch6: VENC
# ch7: VCAP316_0, VCAP316_1
# cmd: echo w [grant/pri/dynamic_pri] [ch] [grant_count/priority/dynamic_pri] > /proc/nvt_drv_sys/dram_info
#      ch:0~7, grant_count:0~15, priority:0->high, 1->mid, 2->low, dynamic_pri:0->disable, 1-> mid, 2->high

echo w pri 5 0 > /proc/nvt_drv_sys/dram_info  # Set DDR CPU(ch5) high priority
echo w dynamic_pri 1 2 > /proc/nvt_drv_sys/dram_info  # Set DDR LCD(ch1) dynamic high priority

echo w grant 0 2 > /proc/nvt_drv_sys/dram_info  # set DDR JPG,OSG,VPE316(ch0) grant 2
echo w grant 1 2 > /proc/nvt_drv_sys/dram_info  # set DDR LCD(ch1) grant 2
echo w grant 2 8 > /proc/nvt_drv_sys/dram_info  # set DDR VDEC(ch2) grant 4
echo w grant 3 2 > /proc/nvt_drv_sys/dram_info  # set DDR VPE536(ch3) grant 2
echo w grant 4 2 > /proc/nvt_drv_sys/dram_info  # set DDR CNN,NUE(ch4) grant 2
echo w grant 5 2 > /proc/nvt_drv_sys/dram_info  # set DDR CPU,DSP(ch5) grant 2
echo w grant 6 2 > /proc/nvt_drv_sys/dram_info  # set DDR VENC(ch6) grant 8
echo w grant 7 2 > /proc/nvt_drv_sys/dram_info  # set DDR VCAP(ch7) grant 2
echo 1 > /proc/videograph/vpe/adapt_pipe #This function will invalidate the tmnr/mrnr/sharpen functions of vpe to increase the processing speed
