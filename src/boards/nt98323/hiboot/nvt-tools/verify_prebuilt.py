#!/usr/bin/python3
import os
import sys
import subprocess
import re
import time
import os
import colorama
import logging
import argparse
import pickle
import json
from pathlib import Path
from datetime import datetime
from collections import OrderedDict


def system_call(command, get_stdout=True, show_er=True):
    if get_stdout:
        process = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        stdout = process.communicate()[0].decode(
            "ascii", errors="ignore").splitlines()
        er = process.returncode
    else:
        er = os.system(command)
        stdout = ""
    if er != 0:
        logging.error("failed to {}, er={}".format(command, er))
    return er, stdout


def parse_args(argv):
    parser = argparse.ArgumentParser(description='check prebuilt object')
    parser.add_argument('-f', '--files',
                        type=argparse.FileType('r'),
                        nargs=2,
                        help='compare 2 files directly')
    parser.add_argument('-j', '--json', metavar='[.json]',
                        dest='json', action='store',
                        help='load json configure file')
    parser.add_argument('-d', '--driver',
                        action="store_true",
                        help='check driver listed in json')
    parser.add_argument('-g', '--glibc',
                        action="store_true",
                        help='check glibc listed in json')
    parser.add_argument('-u', '--uclibc',
                        action="store_true",
                        help='check uclibc listed in json')
    parser.add_argument('-r', '--rtos',
                        action="store_true",
                        help='check rtos listed in json')
    args = parser.parse_args()
    return args


class ConsoleFormatter(logging.Formatter):
    # MSG_FMT = "%(class)s%(asctime)s: %(message)s\033[0m"
    MSG_FMT = "%(class)s%(message)s\033[0m"
    CSS_CLASSES = {'WARNING': '\033[1;93m',
                   'INFO': '\033[0m',
                   'DEBUG': '\033[0m',
                   'CRITICAL': '\033[1;91m',
                   'ERROR': '\033[1;91m'}

    def __init__(self):
        super().__init__()

    def format(self, record):
        try:
            class_name = self.CSS_CLASSES[record.levelname]
        except KeyError:
            class_name = "info"
        tm_now = datetime.now()
        asctime = "{:02d}:{:02d}".format(
            tm_now.hour,
            tm_now.minute)
        message = record.msg
        return self.MSG_FMT % {
            "class": class_name,
            "message": message,
            "asctime": asctime
        }


def initiate():
    colorama.init()
    # set log file
    rootLogger = logging.getLogger()
    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(ConsoleFormatter())
    rootLogger.addHandler(consoleHandler)
    rootLogger.setLevel(logging.DEBUG)
    return 0


def gen_list():
    hdal_real_dir = os.path.realpath(os.getenv("NVT_HDAL_DIR"))
    cmd = "find {} -name '*.o'".format(hdal_real_dir)
    er, text = system_call(cmd)
    if er != 0:
        return er
    with open('objs.pkl', 'wb') as f:
        pickle.dump(text, f)
    return 0


def chk_one(path):
    hdal_real_dir = os.path.realpath(os.getenv("NVT_HDAL_DIR"))
    file_name = os.path.basename(path)
    cmd = "find {} -name '*.o'".format(hdal_real_dir)
    er, _ = system_call(cmd)
    if er != 0:
        return er

    cmd = "find {} -name {}".format(hdal_real_dir, file_name)
    er, files = system_call(cmd)
    if er != 0:
        return er

    if len(files) < 2:
        logging.info("{} skipped, no object pair".format(file_name))
        return 0
    elif len(files) > 2:
        logging.error(
            "{} exists 3 locations".format(file_name))
        return -1

    crc32 = [0, 0]
    for idx in range(len(files)):
        cmd = \
            "$STRIP --strip-debug {} -o tmp.o && ".format(files[idx]) + \
            "$OBJDUMP -S tmp.o | " + \
            "sed -e '/file format elf/d' > tmp.asm && " + \
            "crc32 tmp.asm"
        er, text = system_call(cmd)
        if er != 0:
            return er
        crc32[idx] = text[0]

    if crc32[0] != crc32[1]:
        logging.error("{} NG, value=0x{} vs value=0x{}".format(
            file_name, crc32[0], crc32[1]))
        return -1

    logging.warning("{} OK, value=0x{} vs value=0x{}".format(
            file_name, crc32[0], crc32[1]))

    return 0


def find_pair_in_hdal(path):
    hdal_real_dir = os.path.realpath(os.getenv("NVT_HDAL_DIR"))
    file_name = os.path.basename(path)
    cmd = "find {} -name '*.o'".format(hdal_real_dir)
    er, _ = system_call(cmd)
    if er != 0:
        return er, None

    cmd = "find {} -name {}".format(hdal_real_dir, file_name)
    er, files = system_call(cmd)
    return er, files


def compare_two(filepath1, filepath2):
    crc32 = [0, 0]
    files = [filepath1, filepath2]
    for idx in range(len(files)):
        cmd = \
            "$STRIP --strip-debug {} -o tmp.o && ".format(files[idx]) + \
            "$OBJDUMP -S tmp.o | " + \
            "sed -e '/file format elf/d' > tmp{}.asm && ".format(idx) + \
            "crc32 tmp{}.asm".format(idx)
        er, text = system_call(cmd)
        if er != 0:
            return er
        crc32[idx] = text[0]

    # reduce path prefix string for debug message
    dir1 = os.path.dirname(filepath1)
    dir2 = os.path.dirname(filepath2)
    n = len(dir1) if len(dir1) < len(dir2) else len(dir2)
    last_slash = 0
    for i in range(n):
        if dir1[i] == dir2[i] and dir1[i] == '/':
            last_slash = i+1
        if dir1[i] != dir2[i]:
            break
    if last_slash == 0:
        short_filename1 = os.path.basename(filepath1)
        short_filename2 = os.path.basename(filepath2)
    else:
        short_filename1 = os.path.join(dir1[last_slash:], os.path.basename(filepath1))
        short_filename2 = os.path.join(dir2[last_slash:], os.path.basename(filepath2))

    logging.info("[0]: {}".format(short_filename1))
    logging.info("[1]: {}".format(short_filename2))

    if crc32[0] != crc32[1]:
        logging.error("[{}] NG, crc:({}!={})".format(
            os.path.basename(filepath1), crc32[0], crc32[1]))
        return -1
    logging.warning("[{}] OK, crc:({}=={})".format(
        os.path.basename(filepath1), crc32[0], crc32[1]))
    return 0

def chk_list(path_to_objs_pkl):
    text = None
    with open(path_to_objs_pkl, 'rb') as f:
        text = pickle.load(f)
    if text is None:
        logging.error("failed to load data from {}".format(path_to_objs_pkl))
        return -1

    total_er = 0
    for line in text:
        er, files = find_pair_in_hdal(line)
        if er != 0:
            return er

        if len(files) < 2:
            logging.info("{} skipped, no object pair".format(os.path.basename(files[0])))
            continue
        elif len(files) > 2:
            logging.error("{} exists 3 locations".format(os.path.basename(files[0])))
            return -1

        if "nvt_audlib_" in files[0]:
            continue

        er = compare_two(files[0], files[1])

        if er != 0:
            # we check all for fully report
            total_er = er
    return total_er

def load_json(json_path):
    cfg = None
    with open(json_path, encoding="utf-8", errors='ignore') as file:
        cfg = json.load(file, object_pairs_hook=OrderedDict)
        if cfg["jsontype"] != "verify_prebuilt":
            logging.error("this json file is not for this application.")
            return None
    return cfg

def process_json(cfg, item_name):
    # check environment
    if os.getenv("LINUX_BUILD_TOP") is None:
        logging.error("source build/envsetup.sh first")
        return -1

    logging.info("===== start checking {} =====".format(item_name))
    total_er = 0
    top_dir = os.getenv("LINUX_BUILD_TOP")
    items = cfg[item_name]
    for item in items:
        file1 = os.path.join(top_dir, item[0])
        file2 = os.path.join(top_dir, item[1])
        # check file exist
        if not os.path.exists(file1):
            logging.error("{} is not existing".format(item[0]))
            total_er = -1
            continue
        if not os.path.exists(file2):
            logging.error("{} is not existing".format(item[1]))
            total_er = -1
            continue
        er = compare_two(file1, file2)
        if er != 0:
            total_er = er
    logging.info("\n")
    return total_er

def rename_obj_to_no_tracer_obj(cfg, item_name):
    # check environment
    if os.getenv("LINUX_BUILD_TOP") is None:
        logging.error("source build/envsetup.sh first")
        return -1
    items = cfg[item_name]
    for item in items:
        item[0] = item[0].replace(".o", "_no_tracer.o")
        item[1] = item[1].replace(".o", "_no_tracer.o")
    return cfg

def rename_archive_to_so(cfg, item_name):
    # check environment
    if os.getenv("LINUX_BUILD_TOP") is None:
        logging.error("source build/envsetup.sh first")
        return -1
    items = cfg[item_name]
    for item in items:
        item[0] = item[0].replace(".a", ".so")
        item[1] = item[1].replace(".a", ".so")
    return cfg

def main(argv):
    er = initiate()
    if er != 0:
        return er
    # parse options
    args = parse_args(argv)
    if args.files:
        er = compare_two(args.files[0].name, args.files[1].name)
    if er != 0:
        return -1

    cfg = None
    if args.json:
        cfg = load_json(args.json)

    if args.driver or args.glibc or args.uclibc or args.rtos:
        if cfg is None:
            logging.error("lack -j to load json file.")
            return -1

    total_er = 0

    if args.driver:
        er = process_json(cfg, "driver")
        if er != 0:
            total_er = er
        # rename .o to _no_tracer.o and check again
        cfg = rename_obj_to_no_tracer_obj(cfg, "driver")
        er = process_json(cfg, "driver")
        if er != 0:
            total_er = er

    if args.glibc:
        er = process_json(cfg, "glibc")
        if er != 0:
            total_er = er
        # rename .a to .so and check again
        cfg = rename_archive_to_so(cfg, "glibc")
        er = process_json(cfg, "glibc")
        if er != 0:
            total_er = er

    if args.uclibc:
        er = process_json(cfg, "uclibc")
        if er != 0:
            total_er = er
        # rename .a to .so and check again
        cfg = rename_archive_to_so(cfg, "uclibc")
        er = process_json(cfg, "uclibc")
        if er != 0:
            total_er = er

    if args.rtos:
        er = process_json(cfg, "rtos")
        if er != 0:
            total_er = er

    return total_er


if __name__ == '__main__':
    sys.exit(main(sys.argv))
