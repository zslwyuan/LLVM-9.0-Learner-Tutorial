The HI_IR2SourceCode pass is used to map the IRs back to the source code 
based on the debug information (DEWARF) in the IR code.

Here, the Pass tries to map each IR code to the specific line in the source code,
map each basic block / loop to specifc range in source code.

The major challenge is that when a function is inlined, it could be hard to do the mapping,
since the debug information might be mixed up. Based on iterating the search for different
subprogram, it seems the challenge is solved.