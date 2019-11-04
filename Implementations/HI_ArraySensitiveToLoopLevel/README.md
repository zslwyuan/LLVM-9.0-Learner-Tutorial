HI_ArraySensitiveToLoopLevel pass is used to check that whether some loops
are strongly sensitive to the partitioning of some dimensions of some arrays.

Find them and when we unroll some of the loops, we will also partition the specific
dimension of the array.