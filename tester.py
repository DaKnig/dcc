#!/usr/bin/env python3
import sys
import subprocess

CLEAR_COLOR = "\033[0m"
GREEN = "\033[92m"
RED = "\033[91m"
if (len(sys.argv)<2):
	print ("usage:",sys.argv[0],"test_name1 test_name2 ...")

potential_test = None

for test_name in sys.argv[1:]:
	print (CLEAR_COLOR,"Running ",'{:.<30}'.format(test_name),sep='',end='')
	try:
		test_info=subprocess.run(args=test_name,stderr=subprocess.PIPE,
			timeout=1,stdout=subprocess.DEVNULL)
		if (test_info.returncode == 0):
			print(GREEN + "PASSED")
			continue
		err_msg="Error:"+str(test_info.returncode)+":\n" + \
                    str(test_info.stderr,errors='backslashreplace')
	except subprocess.TimeoutExpired as te:
		err_msg=str(te)+" :\n"+str(te.stderr,errors='backslashreplace')
	except FileNotFoundError as fnf:
		err_msg=str(fnf)
	print(RED,"FAILED. ",CLEAR_COLOR,err_msg,sep='')
	potential_test = test_name
else:
	print(CLEAR_COLOR)

if potential_test:
	print("launch `gdb %s`? [y/n]"%potential_test)
	try:
		answer = input()
	except KeyboardInterrupt:
		exit()
	if "y" in answer:
		import os
		os.system("gdb "+potential_test)
