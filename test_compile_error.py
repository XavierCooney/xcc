#!/usr/bin/python3
# A small script to test that the compiler successfully errors when it needs to

import os
import subprocess
import sys

COMPILE_TEST_DIR = 'more_tests'
IS_VERBOSE = '-v' in sys.argv
NO_MAKE = '--no-make' in sys.argv

if not NO_MAKE:
    subprocess.run(['make', 'xcc'], check=True)

failed_a_test = False

test_pass_output = []

for test_file_name in os.listdir(COMPILE_TEST_DIR):
    if test_file_name.endswith('.expected'): continue
    with open(os.path.join(COMPILE_TEST_DIR, test_file_name)) as test_file:
        captured_output = subprocess.run(['./xcc'], stdin=test_file, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stderr_decoded = captured_output.stderr.decode('utf-8')
    verbose_info_printed = False

    def print_verbose_dbg_info():
        global verbose_info_printed
        if verbose_info_printed: return
        verbose_info_printed = True
        print('FILE', test_file_name)
        print('STDERR')
        print(stderr_decoded)
        print('STDOUT')
        print(captured_output.stdout.decode('utf-8'))
        print('RETURN CODE')
        print(captured_output.returncode)

    if IS_VERBOSE:
        print_verbose_dbg_info()

    num_expectations_passed = 0
    failed_this_test = False
    with open(os.path.join(COMPILE_TEST_DIR, test_file_name)) as test_file:
        for line_num, line in enumerate(test_file):
            if 'EXPECT ' not in line: continue
            previous_part_of_line, expectation = line.split('EXPECT ', 1)
            cmd, contents = expectation.strip().split(' ', 1)

            if cmd == 'stderr':
                if contents.strip() not in stderr_decoded:
                    print("!!! On test", test_file_name, "failed to find contents of expectation on line", line_num + 1)
                    failed_this_test = True
                    break
            elif cmd == 'rc':
                if (captured_output.returncode == 0) ^ (contents == '0'):
                    print("!!! Wrong compiler return code", test_file_name, "line", line_num)
                    failed_this_test = True
                    break
            elif cmd == 'dbg':
                print_verbose_dbg_info()
            else:
                assert False, "command " + cmd + " not known in " + test_file_name
            num_expectations_passed += 1
        else:
            test_pass_output.append(
                f"{'✔' * num_expectations_passed}\t{num_expectations_passed} test(s) passed on {test_file_name}"
            )
            if not num_expectations_passed:
                print_verbose_dbg_info()
    if failed_this_test:
        print_verbose_dbg_info()
    failed_a_test = failed_a_test or failed_this_test

print(*test_pass_output, sep='\n')

if failed_a_test:
    sys.exit(1)
