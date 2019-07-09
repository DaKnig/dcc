#!/usr/bin/env python3
import sys
import subprocess

CLEAR_COLOR = "\033[0m"
GREEN = "\033[92m"
RED = "\033[91m"
if (len(sys.argv)<2):
	print ("usage:",sys.argv[0],"test_name1 test_name2 ...")

for test_name in sys.argv[1:]:
	print (CLEAR_COLOR,"Running ",'{:.<30}'.format(test_name),sep='',end='')
	try:
		test_info=subprocess.run(args=test_name,stderr=subprocess.PIPE,
			timeout=1,encoding='UTF8',stdout=subprocess.DEVNULL)
		if (test_info.returncode == 0):
			print(GREEN + "PASSED")
			continue
		err_msg="Error:"+str(test_info.returncode)+":\n"+test_info.stderr
	except subprocess.TimeoutExpired as te:
		err_msg=te+" : "+te.stderr
	except FileNotFoundError as fnf:
		err_msg=fnf
	print(RED,"FAILED. ",CLEAR_COLOR,err_msg,sep='')
else:
	print(CLEAR_COLOR)
