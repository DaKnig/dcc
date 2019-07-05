#!/usr/bin/env python3
import sys
import subprocess

if (len(sys.argv)<2):
	print ("usage:",sys.argv[0],"test_name1 test_name2 ...")

for test_name in sys.argv[1:]:
	print ("Running '%s': ........"%(test_name),end=' ')
	try:
		test_info=subprocess.run(args=test_name,stderr=subprocess.PIPE,
			timeout=1,encoding='UTF8',stdout=subprocess.DEVNULL)
		if (test_info.returncode != 0):
			print("FAILED. Error:",test_info.returncode,end=' ')
			print(":",test_info.stderr)
		else:
			print("PASSED.")
	except subprocess.TimeoutExpired as te:
		print("FAILED:",te,":",	te.stderr)
	except FileNotFoundError as fnf:
		print("FAILED:",fnf)
	# add something more generic
