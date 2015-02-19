#!/usr/bin/python

import os, sys
import configuration
import configtester

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

# we assume that the existence of a c/c++ compiler
# implies the existence of ar/ld
def find_c_compiler(is_really_cxx = False):
    cc_name = 'CC' if not is_really_cxx else 'CXX'
    cc = os.environ.get(cc_name)
    if (cc != None):
        configuration.the_global_project_configuration[cc_name] = cc
        return
    # we now look for a predefined bunch of compilers
    gcc_cmd = 'gcc' if not is_really_cxx else 'g++'
    tester = configtester.CommandExecuteTest('gcc finder', '%s --version' % gcc_cmd)
    result = tester.execute_test()
    if (result[0] == 0):
        configuration.the_global_project_configuration[cc_name] = gcc_cmd
        return
    # try clang
    clang_cmd = 'clang' if not is_really_cxx else 'clang++'
    tester = configtester.CommandExecuteTest('clang finder', '%s --version' % clang_cmd)
    result = tester.execute_test()
    if (result[0] == 0):
        configuration.the_global_project_configuration[cc_name] = clang_cmd
        return

    print ('Could not find a suitable C/C++ compiler. Please specify one with the CC/CXX environment variables')

def find_bison():
    tester = configtester.CommandExecuteTest('bison finder', 'bison --version')
    result = tester.execute_test()
    if (result[0] == 0):
        configuration.the_global_project_configuration['BISON'] = 'bison'
        return
    print ('Could not find bison. Please specify one with the BISON environment variable')

def find_flex():
    tester = configtester.CommandExecuteTest('flex finder', 'flex --version')
    result = tester.execute_test()
    if (result[0] == 0):
        configuration.the_global_project_configuration['FLEX'] = 'flex'
        return
    print ('Could not find flex. Please specify one with the FLEX environment variable')

def find_basic_builders():
    find_c_compiler(False);
    find_c_compiler(True)
    configuration.the_global_project_configuration['AR'] = 'ar'
    configuration.the_global_project_configuration['LD'] = 'ld'
    # we also insert any additional cxxflags, ccflags here
    configuration.the_global_project_configuration['CXXFLAGS'] = os.getenv('CXXFLAGS', '')
    configuration.the_global_project_configuration['CFLAGS'] = os.getenv('CFLAGS', '')
    configuration.the_global_project_configuration['LDFLAGS'] = os.getenv('LDFLAGS', '')
    configuration.the_global_project_configuration['BISONFLAGS'] = os.getenv('BISONFLAGS', '')
    configuration.the_global_project_configuration['FLEXFLAGS'] = os.getenv('FLEXFLAGS', '')
