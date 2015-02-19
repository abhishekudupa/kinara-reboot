
#!/usr/bin/python

import sys, os, subprocess, shutil, shlex, uuid, re

if __name__ == '__main__':
    print('This module is intended to be used as a library')
    sys.exit(1)

# classes for configuration tests
# as required by the user

class ConfigurationTestBase(object):
    def __init__(self, name):
        self.name = name

    def execute_test(self):
        pass

# a compilation test.
# inputs are source file name and the compiler command
# along with the flags as a string
class CompileTest(ConfigurationTestBase):
    def __init__(self, name, compiler_command, source_file_name, compiler_flags, executable_name = '', keep_executable = False):
        super(CompileTest, self).__init__(name)
        self.compiler_command = compiler_command
        self.source_file_name = source_file_name
        self.compiler_flags = compiler_flags
        self.executable_name = 'kinara-configurator-temp-' + str(uuid.uuid4()) if executable_name == '' else executable_name

        self.executable_name = './' + self.executable_name
        self.keep_executable = keep_executable

    def execute_test(self):
        if (isinstance(self.compiler_flags, str)):
            flags_list = shlex.split(self.compiler_flags)
        else:
            flags_list = self.compiler_flags
        command_args = [self.compiler_command] + flags_list + [self.source_file_name] + ['-o', self.executable_name]
        test_process = subprocess.Popen(command_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (compile_output, compile_error) = test_process.communicate()

        if (not self.keep_executable):
            os.remove(self.executable_name)

        return (test_process.returncode, compile_output, compile_error)

class CompileAndExecuteTest(CompileTest):
    def __init__(self, name, compiler_command, source_file_name, compiler_flags,
                 executable_name = '', keep_executable = False, executable_args = '', expected_output = ''):
        super(CompileAndExecuteTest, self).__init__(name, compiler_command, source_file_name,
                                                    compiler_flags, executable_name, keep_executable)
        self.executable_args = executable_args
        self.expected_output = expected_output;
        self.actual_keep_executable = self.keep_executable
        self.keep_executable = True

    def execute_test(self):
        (compile_status, compile_output, compile_error) = super(CompileAndExecuteTest, self).execute_test()
        self.keep_executable = self.actual_keep_executable
        if (compile_status != 0):
            return (compile_status, compile_output, compile_error)
        if (isinstance(self.executable_args, str)):
            executable_args = shlex.split(self.executable_args)
        else:
            executable_args = self.executable_args

        command_args = [self.executable_name] + executable_args
        test_process = subprocess.Popen(command_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (test_output, test_error) = test_process.communicate()

        if (not self.keep_executable):
            os.remove(self.executable_name)

        if (self.expected_output != ''):
            return (test_output.strip() == self.expected_output)

        return (test_process.returncode, test_output, test_error,
                compile_status, compile_output, compile_error)

class CommandExecuteTest(ConfigurationTestBase):
    def __init__(self, name, command_args):
        super(CommandExecuteTest, self).__init__(name)
        self.command_args = command_args if (isinstance(command_args, list)) else shlex.split(command_args)

    def execute_test(self):
        test_process = subprocess.Popen(self.command_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (test_output, test_error) = test_process.communicate()
        return (test_process.returncode, test_output, test_error)

class CommandVersionTest(CommandExecuteTest):
    def __init__(self, name, command_args):
        super(CommandVersionTest, self).__init__(name, command_args)

    def execute_test(self):
        (test_status, test_output, test_error) = super(CommandVersionTest, self).execute_test()
        if (test_status != 0):
            return (test_status, test_output, test_error)
        # scrape the output and/or error for a version number
        matches = re.findall('(\d+([.]\d+)+)', test_output)
        return matches[0][0]
