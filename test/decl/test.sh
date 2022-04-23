#!/bin/sh

DCC=../../bin/main

output="$(mktemp)"
errput="$(mktemp)"

red="\033[91m"
green="\033[92m"
resetcolors="\033[0m"

trap "rm -f '$output' '$errput'" EXIT

for i in *.c.in; do
    printf "Testing $i ........ "
    $DCC "$i" 2> "$errput" 1>"$output"
    if [[ $? ]]; then
	printf "${red}FAILED!${resetcolors}\n"
	cat "$output" "$errput"
	printf "\nrunning rr...\n\n"
	rr record -n $DCC "$i"
    else
	printf "${green}PASSED!${resetcolors}\n"
    fi
done
