import os
import sys

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

sys.path.append(WORK_DIR)
sys.path.append('%s/oem/tool/' % WORK_DIR)

def init_config(cfg_file):
    import oem_config
    oem_config.init(cfg_file)

def build(cfg_file):
    from oem_build import OemAreaBuilder, OemImageBuilder
    from tee_build import TeeAreaBuilder
    import oem_config

    # get configuration and scenario
    cfgtor = oem_config.load(cfg_file)
    scen = cfgtor.scenario()
    scen.log()

    owner = scen.tee_owner().lower()

    # determine the paths of input and ouput file
    out_dir = 'image/oem'
    tmp_dir = 'oem/tmp'
    tee_path = {'vendor': 'vendor', 'oem': tmp_dir}

    boot_image_file = '%s/boot_image.bin' % out_dir
    tee_image_file  = '%s/tee_image.bin' % out_dir

    oem_root_pub_key_area_chksum_file ='%s/oem_root_public_key_area_checksum.txt' % tmp_dir

    oem_ib = OemImageBuilder(cfgtor)
    oem_ib.vendor_root_pub_key_area_file ='vendor/vendor_root_public_key_area.bin'
    oem_ib.oem_root_pub_key_area_file  ='%s/oem_root_public_key_area.bin' % tmp_dir
    oem_ib.boot_key_area_file          = '%s/boot_key_area.bin' % tmp_dir
    oem_ib.boot_params_area_file       = '%s/boot_params_area.bin' % tmp_dir
    oem_ib.boot_area_file              = '%s/boot_area.bin' % tmp_dir
    oem_ib.unchk_area_for_vendor_file  = '%s/unchecked_area_for_vendor.bin' % tmp_dir
    oem_ib.gsl_key_area_file           = '%s/gsl_key_area.bin' % tee_path[owner]
    oem_ib.gsl_code_area_file          = '%s/gsl_code_area.bin' % tee_path[owner]
    oem_ib.tee_key_area_file           = '%s/tee_key_area.bin' % tee_path[owner]
    oem_ib.atf_area_file               = '%s/atf_area.bin' % tee_path[owner]
    oem_ib.tee_code_area_file          = '%s/tee_code_area.bin' % tee_path[owner]

    if not os.path.exists(tmp_dir):
        os.mkdir(tmp_dir)

    # build area
    oem_ab = OemAreaBuilder(cfgtor)
    oem_ab.build_root_pub_key_area(oem_ib.oem_root_pub_key_area_file, oem_root_pub_key_area_chksum_file)
    oem_ab.build_boot_key_area(oem_ib.boot_key_area_file)
    oem_ab.build_boot_area(oem_ib.boot_area_file)
    oem_ab.build_boot_params_area(oem_ib.boot_params_area_file)
    oem_ab.build_unchecked_area_for_vendor(oem_ib.unchk_area_for_vendor_file)

    if scen.tee_owner_is_oem():
        tee_ab = TeeAreaBuilder(cfgtor)
        tee_ab.build_gsl_code_area(oem_ib.gsl_code_area_file)
        tee_ab.build_gsl_key_area(oem_ib.gsl_key_area_file)
        if scen.is_tee_enbale():
            tee_ab.build_atf_area(oem_ib.atf_area_file)
            tee_ab.build_tee_code_area(oem_ib.tee_code_area_file)
            tee_ab.build_tee_key_area(oem_ib.tee_key_area_file)

    # build image
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    oem_ib.build_images(boot_image_file, tee_image_file)

    print('Done.')
    return

def check(otp_file, boot_image, tee_image=None):
    from oem_check import OemChecker
    checker = OemChecker(otp_file)
    checker.check(boot_image, tee_image)
    print('Done')
    return

def tips(script):
    print('\nUsage: $ python3 %s OPTION' % script)
    print('OPTION:')
    print('  gencfg    Genearte a configuration file.')
    print('  build     Build images based on the input configuartion file.')
    print('  check     Check images using the input OTP values.')
    print('Examples:')
    print('  $ python3 %s gencfg oem_config.json' % script)
    print('  $ python3 %s build oem_config.json' % script)
    print('  $ python3 %s check oem/otp_check.json image/oem/boot_image.bin image/oem/tee_image.bin' % script)
    print('')

def main(argv):
    if len(argv) > 2 and argv[1] == 'gencfg':
        init_config(cfg_file=argv[2])
    elif len(argv) > 2 and argv[1] == 'build':
        build(cfg_file=argv[2])
    elif len(argv) > 3 and argv[1] == 'check':
        tee_image = None
        if len(argv) > 4:
            tee_image = argv[4]
        check(otp_file=argv[2], boot_image=argv[3], tee_image=tee_image)
    else:
        tips(argv[0])
        sys.exit(1)
    return

if __name__ == "__main__":
    os.chdir(WORK_DIR)
    main(sys.argv)
    sys.exit(0)

