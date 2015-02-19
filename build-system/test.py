#!/usr/bin/python

import configtester

if __name__ == '__main__':
    print('Testing version of C++ compiler...')
    tester = configtester.CommandVersionTest('C++ tester', 'g++ --version')
    test_result = tester.execute_test()
    print(test_result)

    tester = configtester.CompileAndExecuteTest('64 bit tester', 'g++', '../config-tests/Test64Bit.cpp',
                                                '-std=c++11', '', True, [], '8')
    test_result = tester.execute_test()
    print(test_result)
