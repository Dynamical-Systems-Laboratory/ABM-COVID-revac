import subprocess

import sys
py_path = '../../../scripts/'
sys.path.insert(0, py_path)

import utils as ut
from colors import *

py_version = 'python3'

#
# Compile and run all the abm class specific tests
#

# Compile
subprocess.call([py_version + ' compilation.py'], shell=True)

# Test suite 1
ut.msg('RegularTransitions class tests', CYAN)
subprocess.call(['./reg_tr_test'], shell=True)

# Test suite 2
ut.msg('RegularTransitions class tests - collective setup', CYAN)
subprocess.call(['./reg_tr_test_v2'], shell=True)
