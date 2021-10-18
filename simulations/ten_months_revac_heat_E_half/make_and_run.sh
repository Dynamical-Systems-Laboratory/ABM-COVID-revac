#!/bin/bash

# Shell script for automated simulations of ABM with a 
# varying testing and vaccination rate  
# Creates directories for each testing rate, copies all
# the files and directories from templates to the new one,
# runs sub_rate.py to substitute target reopening rate,
# compiles, runs the simulations through MATLAB wrapper
# All this is done in one screen per rate

# This script also opens matlab and adjusts glibc path

# To shut down all screens later
# screen -ls | grep Detached | cut -d. -f1 | awk '{print $1}' | xargs kill

# Run as
# ./make_and_run

# Hardcoded Sy testing rates to consider
declare -a Syrates=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)
# E testing are half of Sy
declare -a Erates=(0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5)
# Vaccination rates
declare -a Vrates=(8 16 40 79 158 396 792 1584 3960)

for i in ${!Syrates[*]};
do
	for j in "${Vrates[@]}";
	do

		echo "Processing Sy testing rate ${Syrates[$i]}, E testing rate ${Erates[$i]}, and V rate $j"
		
		# Make the directory and copy all the neccessary files
	    mkdir "dir_${Syrates[$i]}_$j"
	    cp -r templates/input_data "dir_${Syrates[$i]}_$j/"
		cp -r templates/output "dir_${Syrates[$i]}_$j/"
		cp templates/*.* "dir_${Syrates[$i]}_$j/"
		cd "dir_${Syrates[$i]}_$j/"
	
		# Pre-process input
		# Substitute the correct value for reopeoning rate
		mv input_data/infection_parameters.txt input_data/temp.txt
	    python3.6 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "fraction to get tested" ${Syrates[$i]}
		mv input_data/infection_parameters.txt input_data/temp.txt
	    python3.6 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "average fraction to get tested" ${Syrates[$i]}
	    mv input_data/infection_parameters.txt input_data/temp.txt
		python3.6 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "exposed fraction to get tested" ${Erates[$i]}
		mv input_data/infection_parameters.txt input_data/temp.txt
		python3.6 sub_rate.py input_data/temp.txt input_data/infection_parameters.txt "vaccination rate" $j
	    
		# Compile
		python3.6 compilation.py  
		
		# Run (starts and exits screen)
		screen -d -m ./run_matlab.sh
		
		cd ..
	done
done
