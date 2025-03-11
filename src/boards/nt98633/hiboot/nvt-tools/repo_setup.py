#!/usr/bin/python3
# pylint: disable=locally-disabled, line-too-long, unused-argument, unused-variable, pointless-string-statement, too-many-locals, missing-docstring, too-many-statements, invalid-name, wrong-import-position, too-many-arguments, too-many-branches
# Copyright Novatek Microelectronics Corp. 2016.  All rights reserved.

"""Implements a test item for script """
import os
import sys
import re

def repo_setup(repo_dir):
    file_path = os.path.join(repo_dir, "manifest.xml")
    re_repo = re.compile(r'<project groups=".*"\s*name=.*\s*path="(.*)"\s*revision="(.*)"')
    with open(file_path, 'rt',encoding="Big5", errors='ignore') as fin:
        line = fin.readline()
        while line:
            line = line.encode("ascii", errors="ignore").decode("ascii", errors="ignore")
            match = re.search(re_repo, line)
            if not match:
                line = fin.readline()
                continue
            repo_name = match.group(1)
            branch_name = match.group(2)
            if branch_name == "":
                branch_name = "master"
            #print("{}, {}".format(repo_name, branch_name))
            git_dir = os.path.join(repo_dir, "../{}".format(repo_name))
            cmd = \
                "pushd {}; ".format(git_dir) + \
                "git checkout {}; ".format(branch_name) + \
                "popd"
            #print(cmd)
            os.system(cmd)
            line = fin.readline()
    return 0

def main(argv):
    repo_dir = sys.argv[1]
    if not os.path.isdir(repo_dir):
        print(".repo not exist: {}".format(repo_dir))
        return -1
    repo_setup(repo_dir)
    return 0

# ============================================================================================
# DO NOT EDIT FOLLOWING CODES
# ============================================================================================
if __name__ == "__main__":
    def local_run():
        if len (sys.argv) != 2:
            print("Usage: repo_setup.py [path_to .repo]");
            return -1
        err = main(sys.argv)
        return err
    sys.exit(local_run())
# ============================================================================================
