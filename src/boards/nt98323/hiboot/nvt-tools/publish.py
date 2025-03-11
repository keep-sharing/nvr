#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import json
import argparse
import posixpath
from collections import OrderedDict

def dbg_err(msg):
    print(msg, file=sys.stderr)

def parse_args(argv):
    parser = argparse.ArgumentParser(description='module publish')
    parser.add_argument('-j', '--json',metavar='.json', required=True,
                    dest='json', action='store',
                    help='json file')
    parser.add_argument('-s', '--src', 
                    action="store_true",
                    help='remove non published source files')

    args = parser.parse_args()
    return args

def load_json(json_path):
    if not os.path.isfile(json_path):
        print("cannot find {}".format(json_path), file=sys.stderr)
        return None

    with open(json_path, encoding="utf-8", errors='ignore') as file:
        cfg = json.load(file, object_pairs_hook=OrderedDict)
        if cfg["jsontype"] != "module_publish":
            logging.error("this json file is not for this application.")
            return None
        return cfg
    return None

def remove_source(json_cfg, root_dir):
    # speical case to keep all source
    if len(json_cfg["publish_src"]) > 0 and json_cfg["publish_src"][0] == '*':
        return 0
    publish_src = []
    if len(root_dir) == 0:
        root_dir = "."
    # remove "./" e.g: ./abc/xxx.c => abc/xxx.c
    for src in json_cfg["publish_src"]:
        publish_src.append(src.replace("./", ""))

    skip_root = "skip_root" # for subdir has publish.json, we skip this subfolder and it's recursive one
    for root, dirs, files in os.walk(root_dir):
        if skip_root in root:
            continue
        if root!=root_dir and "publish.json" in files:
            skip_root = root
            continue
        for file in files:
            main_name, ext_name = os.path.splitext(file)
            if ext_name == ".c" or ext_name == ".cpp":
                keep_file = False
                fullpath = os.path.join(root, file)
                # use relative_path to compare publish.json
                relative_path = posixpath.join(os.path.relpath(root, root_dir), file).replace("./", "")
                for src in publish_src:
                    if src == relative_path:
                        keep_file = True
                        break
                if keep_file:
                    print("keep {}".format(relative_path))
                else:
                    print("remove {}".format(relative_path))
                    os.remove(fullpath)
    return 0

def main(argv):
    if 0:
        argv.append(r"-s")
        argv.append(r"-j")
        argv.append(r"Z:\firmware\na51055_dual_sdk\na51055_linux_sdk\base\hdal\drivers\k_driver\source\comm\publish.json")

    args = parse_args(argv)    

    try:
        json_cfg = load_json(args.json)
    except json.decoder.JSONDecodeError as exp:
        print("unable to parse " + args.json)
        print(exp)
        return -1
    if json_cfg is None:
        return -1

    root_dir = os.path.dirname(args.json)
    if remove_source(json_cfg, root_dir) != 0:
        return -1
    return 0


if __name__ == '__main__':
    try:
        er = main(sys.argv)
    except Exception as exp:
        er = -1
        print(exp, file=sys.stderr)
        raise exp
    sys.exit(er)

