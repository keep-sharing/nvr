#boot_ver=v1.10
#chipver=`head -1 /proc/pmu/chipver`
#chipid=`echo $chipver | cut -c 1-4`
echo -----------------------------------------------------------
echo "        Boot DVR_16CH"
echo -----------------------------------------------------------
echo "/sbin/mdev" > /proc/sys/kernel/hotplug

MODULE_PATH="/lib/modules/4.19.148"
MODEL_PATH=$(dirname $MODEL)
MODEL_NAME="${MODEL_PATH##*/}"
AD_MODULE="techpoint"
echo This model is $MODEL_NAME

insmod $MODULE_PATH/vos/kwrap/kwrap.ko
insmod $MODULE_PATH/hdal/comm/hwcopy/kdrv_hwcopy.ko
insmod $MODULE_PATH/hdal/comm/nvt_dmasys/nvt_dmasys.ko
insmod $MODULE_PATH/hdal/comm/drv_sys/nvt_drv_sys.ko
insmod $MODULE_PATH/hdal/comm/util/log.ko log_ksize=4096 crash_notify=/mnt/mtd/crash.sh	#crash to execute /mnt/mtd/crash.sh
echo /tmp > /proc/videograph/dumplog		#change log path to /tmp

insmod $MODULE_PATH/hdal/kdrv_cc_event/kdrv_cc_event.ko
insmod $MODULE_PATH/hdal/kflow_common/ms/ms.ko max_channels=16
insmod $MODULE_PATH/hdal/kflow_common/em/em.ko max_channels=16

insmod $MODULE_PATH/hdal/kdrv_videoout/lcd_codec/lcd_codec.ko
insmod $MODULE_PATH/hdal/kdrv_videoout/tve100/nvt_tve100.ko
insmod $MODULE_PATH/hdal/kdrv_videoout/hdmi/nvt_hdmi20.ko
insmod $MODULE_PATH/hdal/kdrv_videoout/lcd310/flcd300-common.ko
insmod $MODULE_PATH/hdal/kdrv_videoout/lcd310/flcd300-pip.ko gui_ddr=0 suspend_state=1 
insmod $MODULE_PATH/hdal/kdrv_videoout/lcd310/lcd310_1/flcd300-pip1.ko gui_ddr=0 suspend_state=1
insmod $MODULE_PATH/hdal/kdrv_videoout/lcd210/flcd200-common.ko
insmod $MODULE_PATH/hdal/kdrv_videoout/lcd210/flcd200-pip.ko gui_ddr=0 suspend_state=1

insmod $MODULE_PATH/hdal/kdrv_videoprocess/vpe/kdrv_vpe.ko
insmod $MODULE_PATH/hdal/kflow_videoprocess/vpe/kflow_vpe.ko mod_init=1 max_chip_num=1 max_eng_num=2 max_minor_num=255 max_md_lv_num=16 max_total_cam_ch=85 sw_bal_mode=1

insmod $MODULE_PATH/hdal/kdrv_audioio/nvt_audio.ko
insmod $MODULE_PATH/hdal/kflow_audioio/kflow_audio.ko

insmod $MODULE_PATH/hdal/kdrv_videoprocess/dei/kdrv_dei.ko sw_bal_mode=1
insmod $MODULE_PATH/hdal/kflow_videoprocess/dei/kflow_dei.ko max_chip_num=1 max_eng_num=1 max_minor_num=255 max_total_cam_ch=85

insmod $MODULE_PATH/hdal/kdrv_videojpeg/kdrv_jpg.ko jpeg_enc_max_chn=53 jpeg_dec_max_chn=32
insmod $MODULE_PATH/hdal/kdrv_videojpeg/decoder_lib/kdrv_jpgdec.ko
insmod $MODULE_PATH/hdal/kdrv_videoenc/kdrv_venc.ko h26x_enc_max_width=2560 h26x_enc_max_height=1440 max_total_cam_ch=53
insmod $MODULE_PATH/hdal/nvt_vencrc/nvt_vencrc.ko
insmod $MODULE_PATH/hdal/kflow_videoenc/kflow_videoenc.ko
insmod $MODULE_PATH/hdal/kdrv_videodec/h26xdec.ko h26xd_max_width=8192 h26xd_max_height=8192 max_total_cam_ch=32
insmod $MODULE_PATH/hdal/kflow_videodec/kflow_videodec.ko

insmod $MODULE_PATH/hdal/kdrv_videoosg/kdrv_osg.ko
insmod $MODULE_PATH/hdal/kflow_videoosg/kflow_osg.ko
insmod $MODULE_PATH/hdal/kdrv_gfx2d/ssca/kdrv_ssca.ko
insmod $MODULE_PATH/hdal/kdrv_gfx2d/age/kdrv_age.ko
insmod $MODULE_PATH/hdal/kflow_gfx/kflow_gfx.ko

insmod $MODULE_PATH/hdal/kdrv_ai/kdrv_ai.ko
insmod $MODULE_PATH/hdal/kflow_ai/kflow_ai.ko

insmod $MODULE_PATH/hdal/kflow_common/gs/gs.ko max_channels=16
insmod $MODULE_PATH/hdal/kflow_common/usr/usr_proc.ko
insmod $MODULE_PATH/hdal/kflow_common/vpd/vpd.ko quiet=1 max_channels=16
insmod $MODULE_PATH/hdal/comm/ddr_arb/ddr_arb.ko


#insmod extdrv
insmod gpio_nvt.ko
insmod drv_io_nvt.ko
insmod fb_ctrl_nvt.ko

box=`mschip_update | tail -1` 
box=${box:12:4}
if [ $box == 5008 ]; then
	#This is 5008
	insmod ip181x_switch_special_nvt.ko
else
	#This is 5016
	insmod ip181x_switch_nvt.ko
fi

insmod nau88c10_nvt.ko
#update time
hwclock -us

### when use cryptodev
modprobe cryptodev

echo doing mdev-s
mdev -s
echo done!!

# Syntax: module_init  [dtb_file] [is_dump_dts] [is_init_videocap] [is_init_videoout] [is_init_audio] [is_show_logo]
if [ "$AD_MODULE" == "techpoint" ]; then
	/usr/local/bin/module_init  /usr/local/bin/cfg_DVR_16CH.dtb 1 1 1 1 0 &
else
	/usr/local/bin/module_init  /usr/local/bin/cfg_DVR_16CH.dtb 0 2 1 1 0 &
fi

