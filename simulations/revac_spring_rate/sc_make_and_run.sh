#!/bin/bash

# Shell script for automated simulations of ABM with a 
# fixed vaccination rate and variable testing rates 
# Creates directories for each testing rate, copies all
# the files and directories from templates to the new one,
# runs sub_rate.py to substitute target reopening rate,
# Submits a job to slurm which
# compiles, runs the simulations through MATLAB wrapper
# All this is done on one core per rate
# Symptomatic rate is 2x exposed

# This script also opens matlab 

# Run as
# ./sc_make_and_run

# Hardcoded Sy testing rates to consider
declare -a Syrates=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
declare -a Erates=(0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5)

for i in ${!Syrates[*]};
do
	echo "Processing Sy testing rate ${Syrates[$i]} and E testing rate ${Erates[$i]}"
	
	# Make the directory and copy all the neccessary files
    mkdir "dir_$i"
    cp -r templates/input_data "dir_$i/"
	cp -r templates/output "dir_$i/"
	cp templates/*.* "dir_$i/"
	cd "dir_$i/"

	# Pre-process input
	# Substitute the correct value for reopeoning rate
	mv input_data/infection_parameters.txt input_data/temp.txt
    python3 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "fraction to get tested" ${Syrates[$i]}
	mv input_data/infection_parameters.txt input_data/temp.txt
    python3 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "average fraction to get tested" ${Syrates[$i]}
	mv input_data/infection_parameters.txt input_data/temp.txt
    python3 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "exposed fraction to get tested" ${Erates[$i]}

	subprocess.call(['sbatch abm_submission.sh'], shell=True)
	
  	cd ..
done
