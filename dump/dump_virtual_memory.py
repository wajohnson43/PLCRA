#!/usr/bin/env python

#################################################################################################
#Program:    system_memory_dump.py																					#
#Author:     djerfy																										#
#Purpose:    Dump all memory of a given program (by pid) into a file.									#
#Date:       2017																											#
#Note:       Modified by Will Johnson																				#
#Taken from: https://gist.github.com/djerfy/64b62d3b3af2431673d4ca3656fd231f							#
#################################################################################################

import ctypes, re, sys

#Set up ptrace as an int32
c_ptrace = ctypes.CDLL("libc.so.6").ptrace
c_pid_t = ctypes.c_int32
c_ptrace.argtypes = [ctypes.c_int, c_pid_t, ctypes.c_void_p, ctypes.c_void_p]

##call ptrace, given a bool (attach) and a pid
def ptrace(attach, pid):
	
	#set everything up
	op = ctypes.c_int(16 if attach else 17)   #Decide which opcode to use (are we attaching or detaching)
	c_pid = c_pid_t(pid)								#Set up the pid as a int32 from C
	null = ctypes.c_void_p()						#set null
	
	#call ptrace
	err = c_ptrace(op, c_pid, null, null)
	
	#raise an exception if ptrace throws an error
	if err != 0: raise Exception( 'ptrace '+ str(err))
	
if __name__ == "__main__":
	#validate input
	if(len(sys.argv) < 2):
		print "%s <pid>" % sys.argv[0]
		sys.exit(-1)

	#get the pid and call ptrace		
	pid = sys.argv[1]
	ptrace(True, int(pid))
	
	#open all files
	dump_file = open("./%s.dump" % pid, 'w')
	maps_file = open("/proc/%s/maps" % pid, 'r')
	mem_file = open("/proc/%s/mem" % pid, 'r', 0)
	
	line_count = 0
	chunk_count = 0
	pass_count = 0
	#for each line in the map file...
	for line in maps_file.readlines():
		
		line_count+= 1
		#get the memory bounds
		m = re.match(r'([0-9A-Fa-f]+)-([0-9A-Fa-f]+) ([-r])', line)
		
		#if the memory is readable...
		#if m.group(3) == 'r':
			
			#try to get the memory chunk.
		try:
			start = int(m.group(1), 16)
			end = int(m.group(2), 16)
			mem_file.seek(start)
			chunk = mem_file.read(end - start)
			dump_file.write(chunk)
			chunk_count+= 1
			
			#else, pass
		except:
			pass_count += 1
			print(line)
			pass
				
	#close all the files.
	maps_file.close()
	mem_file.close()
	dump_file.close()
	ptrace(False, int(pid))
	
	
	print("Number of lines: " + str(line_count) + "\t Number of chunks: " + str(chunk_count) + "\t Number of passes: " + str(pass_count))