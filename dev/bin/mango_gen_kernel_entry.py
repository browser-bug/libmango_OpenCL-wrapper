#!/usr/bin/env python2
from string import split, strip, join

from sys import argv
arch  = argv[-2]
fname = argv[-1]

kernel_proto = 'not found'

with open(fname,'r') as fin :
	lines = fin.readlines()
	for l in range(len(lines)) : 
		if strip(lines[l])=='#pragma mango_kernel' :
			kernel_proto=strip(lines[l+1])
if kernel_proto == 'not found' :
	raise Exception, 'Missing mango_kernel pragma'

kernel_proto =split(kernel_proto, '{')[0]

kp=split(kernel_proto,'(')
kname = split(kp[0])[-1]
kargs = split(split(kp[1],')')[0],',')
kargs = [ split(a) for a in kargs ]
kargs = [ k[:-1] + ([ '*', k[-1][1:] ] if '*' in k[-1] else [k[-1]])  for k in kargs ]

#print kname
#print kargs

template_start = """
#include "dev/mango_hn.h"
#include <stdlib.h>
"""

template_main="""
int main(int argc, char **argv){
	mango_init(argv);
"""
template_end   = """
	mango_close(42);
}"""

out = template_start
out+='extern '+kernel_proto+';\n'
out+= template_main

i=5	# TODO REMOVE ME
if arch == "GN":
	i=6
if arch == "PEAK":
	i=5

for a in kargs :
	if '*' in join(a[:-1]) : 
		out+='\t'+join(a)+' = ('+join(a[:-1])+')mango_memory_map(strtol(argv['+`i`+'],NULL,16));\n'
	elif a[0]=='mango_event_t' : 
		out+='\tmango_event_t '+a[-1]+';\n\t'+a[-1]+'.vaddr = (uint32_t *)mango_memory_map(strtol(argv['+`i`+'],NULL,16));\n'
	elif join(a[:-1]) in [ 'int', 'unsigned int', 'signed int', 'long', 'signed long', 'unsigned long', 'int32_t', 'uint32_t' ] :
		out+='\t'+join(a)+' = strtol(argv['+`i`+'],NULL,16);\n'
	elif join(a[:-1]) == 'float': 
		out+='\t'+join(a)+' = strtof(argv['+`i`+'],NULL,16);\n'
	elif join(a[:-1]) == 'double': 
		out+='\t'+join(a)+' = strtod(argv['+`i`+'],NULL,16);\n'
	else :
		print 'Unrecognized type', a
	i+=1
out+='\n\t'+kname+'('
for a in kargs :
	out+=a[-1]+', '
out=out[:-2]
out+=');\n'
out+=template_end

#print out

with open("main.c", "w") as fout :
	fout.write(out)

