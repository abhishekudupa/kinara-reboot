#!/usr/bin/python

import sys

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

# classes for representing compilable objects

class ComponentBase(object):
    def __init__(self, name):
        self.name = name

    def generate_makefile(self, output_file):
        pass

class ObjectFile(ComponentBase):
    def __init__(self, name, source_filepath, object_filepath):
        super(ObjectFile, self).__init__(name)
        self.source_filepath = source_filepath
        self.object_filepath = object_filepath

    def generate_makefile(self. output_file):
        pass
