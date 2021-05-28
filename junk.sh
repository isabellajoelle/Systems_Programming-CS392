###############################################################################
# Author: Isabella Cruz and Julia Liszka
# Date: 2/16/2021
# Pledge: "I pledge my honor that I have abided by the Stevens Honor System."
# Description: A simple bash script to provide the basic functionality of a recycle bin
###############################################################################
#!/bin/bash

j="/home/junk/junk.sh"
readonly junk=".junk"

#creation of the heredoc which holds the usage statement
(
cat << ENDOFTEXT
Usage: $(basename "$j") [-hlp] [list of files]
	  -h: Display help.
	  -l: List junked files.
	  -p: Purge all files.
	  [list of files] with no other arguments to junk those files.
ENDOFTEXT
)>Usage.txt

# will return Usage Error is no inputs are given
if [ $# -eq 0 ]; then
	cat Usage.txt
	rm Usage.txt
	exit 1
fi

number_of_flags=0
flag_name=0

# counts the number of flags inputted (if any) and returns an error if an unknown flag is inputted
while getopts ":hlp" option; do
	((number_of_flags++))
	case "$option" in
		l) flag_name=1
		;;
		h) flag_name=2
		;;	
		p) flag_name=3
		;;
		?) printf "Error: Unknown option '-${OPTARG}'.\n"
			cat Usage.txt
			exit 1
			;;
	esac
done

# if the number of flags given is greater than 1, this will return a Usage Error
if [ $number_of_flags -gt 1 ]; then
	printf "Error: Too many options enabled.\n"
	cat Usage.txt
	rm Usage.txt
	exit 1
fi

shift "$(( OPTIND-1 ))"

index=0
declare -a filenames

#counts the number of files inputted (if any)
for f in "$@"; do   
	filenames[$index]="$f"   
	(( ++index ))
done

# checks if flags and files were inputted at the same time, if true this will return a Usage Error
if [ $number_of_flags -eq 1 ] && [ $index -gt 0 ]; then
	printf "Error: Too many options enabled.\n"
	cat Usage.txt
	rm Usage.txt
	exit 1
fi

# creates the .junk directory
cd ~
if [ ! -d "$junk" ]; then
	mkdir $junk
fi
cd junk

# if files are given, this will loop through them
if [ $index -gt 0 ]; then

	for file in ${filenames[@]}; do
		#if the file is not found, print that it is not found and continue the loop
		if [ ! -f "$file" ] && [ ! -d "$file" ]; then
			printf "Warning: '${file}' not found.\n"
			continue
		#if the file is found, move it to the .junk directory
		else
			mv $file ~/.junk
		fi
	done
# else if a flag is given ...
else
	# carry out a specific action for each flag
	case "$flag_name" in
		1) cd ../.junk
		   ls -lAF
		   cd ../junk
		;;
		2) cat Usage.txt
		;;	
		3) rm -rf ../.junk*
		;;
	esac	
fi
	

rm Usage.txt
exit 0
