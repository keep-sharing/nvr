import os
import sys
import oem_main
import common.clean

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

os.chdir(WORK_DIR)
common.clean.clean_output(WORK_DIR)
common.clean.clean_output('oem')
