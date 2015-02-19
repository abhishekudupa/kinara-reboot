#!/usr/bin/python

import os, sys
import configuration

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

class ActionBase(object):
    def __init__(self, name):
        self.name = name

def CompileActionBase(ActionBase):
    def __init__(self, name, config_name, compile_command, compile_flags):
        super(CompileActionBase, self).__init__(name)
        self.config_name = config_name
        self.compile_command = compile_command
        self.compile_flags = compile_flags

def CXXCompileAction(CompileActionBase):
    def __init__(self, config_name):
        cxx_command = configuration.the_global_project_configuration['CXX']
        cxx_flags = configuration.the_project_build_configurations[config_name]['CXXFLAGS']
        super(CXXCompileAction, self).__init__('CXXCompileAction', config_name, cxx_command, cxx_flags)

def CCompileAction(CompileActionBase):
    def __init__(self, config_name):
        cc_command = configuration.the_global_project_configuration['CC']
        cc_flags = configuration.the_project_build_configurations[config_name]['CFLAGS']
        super(CCompileAction, self).__init__('CCompileAction', config_name, cc_command, cc_flags)

def YCompileAction(CompileActionBase):
    def __init__(self, config_name):
        bison_command = configuration.the_global_project_configuration['BISON']
        bison_flags = configuration.the_project_build_configurations[config_name]['BISONFLAGS']
