#!/usr/bin/python
#
# Quick and dirty sine value table generator
#

from math import sin

f = open("sine_table.c", "w")

f.write("const int sine[360] = {\n")

for i in range(360):
    #f.write( "\t"+str(int(256*sin(i * 3.1415 / 180))) )
    f.write( "\t"+str(int(128 + (127.5*sin(i * 3.1415 / 180)))) )
    if i < 359:
        f.write( ",\n" )
    
f.write("\n};\n")
