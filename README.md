timestamp_compression
=====================

A log of timestamps, which is on an average 8mb of size,  from a system is compressed to 750kb (average) for the purpose of transmission


On linux systems, 
compile as shown below

"gcc -o ts ts.c -lm"

Run as,

"./ts"

It shows two options:
  		1: Encode
			2: Decode
Choose anyone
In any option, u have to provide input and output filenames, when asked for.

