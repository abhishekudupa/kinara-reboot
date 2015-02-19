#!/usr/bin/python

import subprocess, re

if __name__ == '__main__':
    cat_proc = subprocess.Popen(['cat', '/proc/cpuinfo'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (cat_out, cat_err) = cat_proc.communicate()

    if (re.search('sse4_2', cat_out)):
        print('-msse4.2')
