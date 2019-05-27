HI_WithDirectiveTimingResourceEvaluation pass is used to evaluate the timing and resource of the application, considering the effect of HLS directives.

Here, I mainly consider array partitioning (cyclic), loop pipelining and loop unrolling.

Basic implementation idea:

1. Array partitioning: Using SCEV, the pass can get the pattern of array accesses. Assuming that the access pattern will not change inside a loop. Based on the access pattern, the map from accesses to partitions can be determined. According to the map and the port limitation of BRAMs, we can schedule the array accesses.