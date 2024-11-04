First of all, before I began working on Project 4, I fixed the bugs from Project 3.
New features expand after Project 3:
Add instruction to handle bool type:
		I declared the bool type as numeric based on the lecture slides, if it’s true, then its numerical value is 1, otherwise is 0. Then when the bool variable is used in the condition statement in branches, I will compare it with 0. Similarly, I also handle bool values extracted from variables.
Branches
		For if and while, I basically implement it as the slides suggest, however, the slides are lacking the implementation details on how to find the corresponding position, such as where is the first instruction of the then-body or else-body. So what I did is instead of finding the “first” instruction, I found the “last” instruction for the previous section, then inserted the corresponding label of “while_lable” or “if_else_lable” after the “last” instruction I found. Then the rest of the implementation is just really similar to the slides.
Array
		For a one dimensional array, I get the exp register as index, then use it to create (call malloc) or access the array entry (by ldr itself again using the index). For the two dimensional array declaration, I used the same structure as the while loop to iteratively assign a two dimensional malloc address to the one dimensional array address. And for accessing the two dimensional array entries, I find the address for the one dimensional base address the access again as its another array.
	Then the rest of the processes are just spending a really long time debugging. 
