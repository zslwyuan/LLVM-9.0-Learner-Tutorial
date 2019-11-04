HI_LoadALAP will try to reorganize the calculation order of a sequence of additions or multiplications 
to make a load can be occurred as late as possible.

example:

a = load
b1 = a + b0
b2 = c1 + b1



after transform: 
(condition: b1,b2 has only one user)
(condition: c1 is not a load instruction)

a = load
b1 = c1 + b0
b2 = a + b1

