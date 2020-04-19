Hi, all the testcases I made are in runtest.c I created one giant testcase
that tests all the outputs of functions of part 1 except repack, read and write
I created another testcase, that assesses all the outputs from read and write functions
And another solely for repack
And the last one for fletcher and hash tree

***All the binary files were made in runtest.c, and it all exists here,
		even though they are not shown on the side viewing panel***
	


For my assignment code, I have written the comments down inside each function,
explaining each step.

ALSO, I did not make a seprate "check if file exists" function, because I use the
algorithm to sometimes find the offset, or size or maybe both, and also sometimes 
for more than one file. So, I concluded that creating a seperate function for it
would be unnecessary

