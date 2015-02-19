#!/usr/bin/python

import sys, os
import configtester
import configuration

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

def is_cpp_file(filename):
    return (filename.endswith('.cpp') or
            filename.endswith('.C') or
            filename.endswith('.cxx') or
            filename.endswith('.cc'))

def is_cpp_pass_fail_test(filename):
    return filename.endswith('-pass-fail.cpp')

def is_cpp_feature_test(filename):
    return filename.endswith('-feature-test.cpp')

def is_cpp_configuration_test(filename):
    return filename.endswith('-configuration-test.cpp')

def is_py_file(filename):
    return (filename.endswith('.py') and
            (not filename.endswith('-buildinfo.py')))

def is_py_pass_fail_test(filename):
    return filename.endswith('-pass-fail.py')

def is_py_feature_test(filename):
    return filename.endswith('-feature-test.py')

def is_py_configuration_test(filename):
    return filename.endswith('-configuration-test.py')

def find_buildinfo_for_cpp_file(cpp_filename, project_root):
    buildinfo_filename = os.path.join(project_root, os.path.splitext(cpp_filename))
    buildinfo_filename += '-buildinfo.py'
    if (os.stat(buildinfo_filename)):
        return buildinfo_filename
    else:
        return None

# we execute all the tests in the config-tests
# directory
def configure(kinara_root):
    project_root = configuration.the_global_project_configuration['project_root']
    filenames = os.listdir(os.path.join(project_root, 'config-tests'))
    for filename in filenames:
        if (is_cpp_feature_test(filename)):
            run_cpp_feature_test(filename)
        elif (is_cpp_pass_fail_test(filename)):
            run_cpp_pass_fail_test(filename)
        elif (is_cpp_configuration_test(filename)):
            run_cpp_configuration_test(filename)
        elif (is_py_configuration_test(filename)):
            run_py_configuration_test(filename)
        elif (is_py_feature_test(filename)):
            run_py_feature_test(filename)
        elif (is_py_pass_fail_test(filename)):
            run_py_pass_fail_test(filename)
        elif (is_cpp_file(filename) or is_py_file(filename)):
            print('Unknown config test named \"%s\" found, I\'m leaving it alone...')
        else:
            continue
