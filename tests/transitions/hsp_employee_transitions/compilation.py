import subprocess, glob, os

#
# Input 
#

# Path to the main directory
path = '../../../src/'
# Compiler options
cx = 'g++'
std = '-std=c++11'
opt = '-O0'
# Common source files
src_files = path + 'abm.cpp' 
src_files += ' ' + path + 'data_management_interface.cpp'
src_files += ' ' + path + 'agent.cpp' 
src_files += ' ' + path + 'infection.cpp'
src_files += ' ' + path + 'mobility.cpp'
src_files += ' ' + path + 'testing.cpp'
src_files += ' ' + path + 'vaccinations.cpp'
src_files += ' ' + path + 'contributions.cpp'
src_files += ' ' + path + 'contact_tracing.cpp'
src_files += ' ' + path + 'transitions/transitions.cpp'
src_files += ' ' + path + 'transitions/regular_transitions.cpp'
src_files += ' ' + path + 'transitions/hsp_employee_transitions.cpp'
src_files += ' ' + path + 'transitions/hsp_patient_transitions.cpp'
src_files += ' ' + path + 'transitions/flu_transitions.cpp'
src_files += ' ' + path + 'states_manager/states_manager.cpp'
src_files += ' ' + path + 'states_manager/regular_states_manager.cpp'
src_files += ' ' + path + 'states_manager/hsp_employee_states_manager.cpp'
src_files += ' ' + path + 'flu.cpp'
src_files += ' ' + path + 'utils.cpp'
src_files += ' ' + path + 'three_part_function.cpp'
src_files += ' ' + path + 'four_part_function.cpp'
src_files += ' ' + path + 'places/place.cpp'
src_files += ' ' + path + 'places/household.cpp'
src_files += ' ' + path + 'places/workplace.cpp'
src_files += ' ' + path + 'places/school.cpp'
src_files += ' ' + path + 'places/hospital.cpp'
src_files += ' ' + path + 'places/retirement_home.cpp'
src_files += ' ' + path + 'places/transit.cpp'
src_files += ' ' + path + 'places/leisure.cpp'
src_files += ' ' + path + 'io_operations/FileHandler.cpp'
src_files += ' ' + path + 'io_operations/load_parameters.cpp'
tst_files = '../../common/test_utils.cpp'
# Directory with files for testing
data_dir = './test_data/'

#
# Tests
#

# Test 1
# Name of the executable
exe_name = 'hsp_em_tr_test'
# Files needed only for this build
spec_files = 'hsp_employee_transitions_tests.cpp '
compile_com = ' '.join([cx, std, opt, '-o', exe_name, spec_files, tst_files, src_files])
subprocess.call([compile_com], shell=True)

# Test 2
# Name of the executable
exe_name = 'hsp_em_tr_test_v2'
# Files needed only for this build
spec_files = 'sim_set_hsp_employee_transitions_tests.cpp '
compile_com = ' '.join([cx, std, opt, '-o', exe_name, spec_files, tst_files, src_files])
subprocess.call([compile_com], shell=True)

