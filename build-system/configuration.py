#!/usr/bin/python

import os, sys

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

# the system specific configuration,
# contains info about operating system,
# compiler to use, etc
the_system_configuration = {}

# global project specific configuration,
# contains info about modules, search paths
# and such
the_global_project_configuration = {}

# build specific project configurations
# contains info about cflags, build paths, etc
# for each kind of build
the_project_build_configurations = {}
