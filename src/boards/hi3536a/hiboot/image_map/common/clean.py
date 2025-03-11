import os
import sys
import shutil

def do_clean(dirname, rm_ftypes=[], rm_dirs=[]):
    if not os.path.exists(dirname):
        return
    print('Cleaning %s' % dirname)
    for entry in os.listdir(dirname):
        path = os.path.join(dirname, entry)
        if os.path.isfile(path):
            if os.path.splitext(entry)[-1] not in rm_ftypes:
                continue
            print('Remove %s' % path)
            os.remove(path)
        elif os.path.isdir(path):
            if entry not in rm_dirs:
                continue
            print('Remove %s/' % path)
            shutil.rmtree(path)
    return

def clean_output(owner):
    do_clean(dirname='image', rm_dirs=[owner])
    do_clean(dirname=owner, rm_ftypes=['.pyc', '.txt', '.bin'], rm_dirs=['tmp', 'out'])
    do_clean(dirname='%s/tool' % owner, rm_ftypes=['.pyc'], rm_dirs=['__pycache__'])
    do_clean(dirname='common', rm_ftypes=['.pyc'], rm_dirs=['__pycache__'])