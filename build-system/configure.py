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
    buildinfo_filename = os.path.join(project_root, os.path.splitext(cpp_filename)[0])
    buildinfo_filename += '-buildinfo'
    try:
        os.stat(buildinfo_filename)
        return buildinfo_filename
    except OSError:
        return None

def run_cpp_feature_test(filename, project_root):
    buildinfo_filename = find_buildinfo_for_cpp_file(filename, project_root)
    if (buildinfo_filename != None):
        command_line_args = (open(buildinfo_filename, 'r')).read().strip()
    else:
        command_line_args = ''
    tester = configtester.CompileAndExecuteTest('cpp-feature-test',
                                                configuration.the_global_project_configuration['CXX'],
                                                filename, command_line_args)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Feature test with source file: %s failed' % filename)
    key_value_pairs = result[1].strip().split('\n')
    for kvpair in key_value_pairs:
        lhs_rhs = kvpair.split('=')
        lhs = lhs_rhs[0].strip()
        rhs = lhs_rhs[1].strip()
        value = configuration.the_global_project_configuration.get(lhs, '')
        value += (' ' + rhs)
        configuration.the_global_project_configuration[lhs] = value

def run_cpp_pass_fail_test(filename, project_root):
    buildinfo_filename = find_buildinfo_for_cpp_file(filename, project_root)
    if (buildinfo_filename != None):
        command_line_args = (open(buildinfo_filename, 'r')).read().strip()
    else:
        command_line_args = ''
    tester = configtester.CompileAndExecuteTest('cpp-pass-fail-test',
                                                configuration.the_global_project_configuration['CXX'],
                                                filename, command_line_args)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Pass/fail test with source file: %s failed' % filename)
        print (result[1])
        print (result[2])

def run_cpp_configuration_test(filename, project_root):
    buildinfo_filename = find_buildinfo_for_cpp_file(filename, project_root)
    if (buildinfo_filename != None):
        command_line_args = (open(buildinfo_filename, 'r')).read().strip()
    else:
        command_line_args = ''
    tester = configtester.CompileAndExecuteTest('cpp-config-test',
                                                configuration.the_global_project_configuration['CXX'],
                                                filename, command_line_args)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Configuration test with source file: %s failed' % filename)
    key_value_pairs = result[1].strip().split('\n')
    defines_map = configuration.the_global_project_configuration.get('.defines')
    for kvpair in key_value_pairs:
        lhs_rhs = kvpair.split('=')
        lhs = lhs_rhs[0].strip()
        rhs = lhs_rhs[1].strip()
        if (defines_map.get(lhs) != None):
            print ('Warning: overwriting a configuration option %s' % lhs)
        defines_map[lhs] = rhs

def run_py_feature_test(filename, project_root):
    tester = configtester.CommandExecuteTest('py-feature-test', sys.executable + ' ' + filename)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Configuration test with source file: %s failed' % filename)
    key_value_pairs = result[1].strip().split('\n')
    for kvpair in key_value_pairs:
        lhs_rhs = kvpair.split('=')
        lhs = lhs_rhs[0].strip()
        rhs = lhs_rhs[1].strip()
        value = configuration.the_global_project_configuration.get(lhs, '')
        print configuration.the_global_project_configuration
        print lhs
        print value
        value += (' ' + rhs)
        configuration.the_global_project_configuration[lhs] = value

def run_py_pass_fail_test(filename, project_root):
    tester = configtester.CommandExecuteTest('py-pass-fail-test', filename)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Pass/fail test with source file: %s failed' % filename)

def run_py_configuration_test(filename, project_root):
    tester = configtester.CommandExecuteTest('py-pass-fail-test', filename)
    result = tester.execute_test()
    if (result[0] != 0):
        print ('Configuration test with source file: %s failed' % filename)
    key_value_pairs = result[1].strip().split('\n')
    defines_map = configuration.the_global_project_configuration.get('.defines')
    for kvpair in key_value_pairs:
        lhs_rhs = kvpair.split('=')
        lhs = lhs_rhs[0].strip()
        rhs = lhs_rhs[1].strip()
        if (defines_map.get(lhs) != None):
            print ('Warning: overwriting a configuration option %s' % lhs)
        defines_map[lhs] = rhs

# we execute all the tests in the config-tests
# directory
def configure():
    project_root = configuration.the_global_project_configuration['project_root']
    filenames = os.listdir(os.path.join(project_root, 'config-tests'))
    for filename in filenames:
        filename = os.path.join(project_root, 'config-tests/', filename)
        if (is_cpp_feature_test(filename)):
            run_cpp_feature_test(filename, project_root)
        elif (is_cpp_pass_fail_test(filename)):
            run_cpp_pass_fail_test(filename, project_root)
        elif (is_cpp_configuration_test(filename)):
            run_cpp_configuration_test(filename, project_root)
        elif (is_py_configuration_test(filename)):
            run_py_configuration_test(filename, project_root)
        elif (is_py_feature_test(filename)):
            run_py_feature_test(filename, project_root)
        elif (is_py_pass_fail_test(filename)):
            run_py_pass_fail_test(filename, project_root)
        elif (is_cpp_file(filename) or is_py_file(filename)):
            print('Unknown config test named \"%s\" found, I\'m leaving it alone...')
        else:
            continue
