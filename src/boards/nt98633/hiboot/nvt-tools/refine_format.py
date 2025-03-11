#!/usr/bin/python3
# pylint: disable=locally-disabled, line-too-long, unused-argument, unused-variable, pointless-string-statement, too-many-locals, missing-docstring, too-many-statements, invalid-name, wrong-import-position, too-many-arguments, too-many-branches
# Copyright Novatek Microelectronics Corp. 2016.  All rights reserved.

"""Implements a test item for script """
import os
import sys
import re

def count_format(format_line):
    format_list = list()
    for i in range(len(format_line)-2):
        ch0 = format_line[i]
        ch1 = format_line[i+1]
        if i < len(format_line)-2:
            ch2 = format_line[i+2]
        else:
            ch2 = ''
        if i < len(format_line)-3:
            ch3 = format_line[i+3]
        else:
            ch3 = ''
        if ch0 == r'%':
            if ch1 == 's' or ch1 == 'S':
                format_list.append('s')
            elif ch1 == 'd' or ch1 == 'D':
                format_list.append('d')
            elif ch1 == 'u' or ch1 == 'U':
                format_list.append('u')
            elif ch1 == 'x' or ch1 == 'X':
                format_list.append('x')
            elif ch1 == 'l' or ch1 == 'L':
                if ch2 == 'd' or ch2 == 'D':
                    format_list.append('ld')
                elif ch2 == 'u' or ch2 == 'U':
                    format_list.append('lu')
                elif ch2 == 'x' or ch2 == 'X':
                    format_list.append('lx')
                elif ch2 == 'l' or ch2 == 'L':
                    if ch3 == 'd' or ch3 == 'D':
                        format_list.append('lld')
                    elif ch3 == 'u' or ch3 == 'U':
                        format_list.append('llu')
                    elif ch3 == 'x' or ch3 == 'X':
                        format_list.append('llx')
    return format_list

def count_param(param_line):
    param_list = list()
    re_param_list = [
        re.compile(r'(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*),(.*)'),
        re.compile(r'(.*),(.*)'),
        re.compile(r'(.*)')
    ]

    for re_param in re_param_list:
        match = re.search(re_param, param_line)
        if match:
            for i in range(re_param.groups):
                param_list.append(match.group(i+1).strip())
            break
    return param_list

def modify_line(format_line, format_list, param_list):
    line_changed = False
    new_line = None
    for i in range(len(format_list)):
        format_tag = format_list[i]
        format_param = param_list[i]
        if format_tag == 's':
            continue
        if "int)"in format_param:
            continue
        if format_tag == 'd':
            param_list[i] = "(int)(" + format_param + ")"
        elif format_tag == 'u' or format_tag == 'x':
            param_list[i] = "(unsigned int)(" + format_param + ")"
        elif format_tag == 'ld':
            param_list[i] = "(long)(" + format_param + ")"
        elif format_tag == 'lu' or format_tag == 'lx':
            param_list[i] = "(unsigned long)(" + format_param + ")"
        elif format_tag == 'lld':
            param_list[i] = "(long long)(" + format_param + ")"
        elif format_tag == 'llu' or format_tag == 'llx':
            param_list[i] = "(unsigned long long)(" + format_param + ")"
        if param_list[i] != format_param:                        
            #print(format_param, param_list[i])
            line_changed = True
    if line_changed:
        new_line = '("' + format_line + '", '
        for i in range(len(format_list)):
            format_param = param_list[i]
            new_line = new_line + format_param
            if i != len(format_list)-1:
                new_line = new_line + ", "
            else:
                new_line = new_line + ")"
    return new_line

def refine(file_path):
    context_changed = False
    re_quotation = re.compile(r'\("(.*)",(.*)\)')
    with open(file_path, 'rt',encoding="Big5", errors='ignore') as fin:
        with open(file_path+".new", 'wt',encoding="Big5", errors='ignore') as fout:
            line = fin.readline()
            line = line.encode("ascii", errors="ignore").decode("ascii", errors="ignore")
            while line:            
                line_changed = False
                match = re.search(re_quotation, line)
                if not match:
                    fout.write(line)
                    line = fin.readline()
                    continue
                #print("{}, {}".format(match.group(1), match.group(2)))
                syntax_line = match.group(0)
                format_line = match.group(1)
                param_line = match.group(2)
                # check if sscanf or define
                if '&' in param_line or '#define' in line:
                    fout.write(line)
                    line = fin.readline()
                    continue
                # count fromat number of % and number of param should be matched
                format_list = count_format(format_line)
                param_list = count_param(param_line)           
                if len(format_list) == len(param_list):
                    #print("{}, {}, {}".format(line, format_list, param_list))
                    new_line = modify_line(format_line, format_list, param_list)
                    if new_line is not None:
                        line = line.replace(syntax_line, new_line)
                        context_changed = True
                fout.write(line)
                line = fin.readline()
    if not context_changed:
        os.remove(file_path+".new")
    else:
        os.remove(file_path)
        os.rename(file_path+".new", file_path)            
    return 0

def main(argv):
    print("processing {}".format(sys.argv[1]))
    refine(sys.argv[1])
    return 0

# ============================================================================================
# DO NOT EDIT FOLLOWING CODES
# ============================================================================================
if __name__ == "__main__":
    def local_run():
        if len (sys.argv) != 2:
            print("Usage: python3 refine_format.py [c file]");
            print("e.g: python3 refine_format.py myfile.c");
            return -1
        err = main(sys.argv)
        return err
    sys.exit(local_run())
# ============================================================================================
