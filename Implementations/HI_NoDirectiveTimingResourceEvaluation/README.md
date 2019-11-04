HI_NoDirectiveTimingResourceEvaluation pass is tested by LLVM_exp12_HI_NoDirectiveTimingResourceEvaluation

Compared to HI_SimpleTimingEvaluation, HI_NoDirectiveTimingResourceEvaluation will access 
the database to get the accurate information of the instruction, including timing and 
resource, to achieve accurate timing evaluation.

However, in this pass, the HLS directives will still not work, which will be enabled in 
the next step.

In the implementation, the high-level idea is similar to HI_SimpleTimingEvaluation.
However, the class inst_timing_resource_info and timingBase are used to handle the detailed 
information for the timing and resource of instructions in a nicer way.

Related operators are overloaded, e.g. +, * and >, for the caluation of critical path and 
the latency of loop/block/function. For each instruction, HI_NoDirectiveTimingResourceEvaluation 
will inquiry the map variables to get the result of timing and resource.

Besides, the chaining of operations for DSP utilization is considered.

The memory hazard is not considered in this pass. The hazard is considered in HI_WithDirectiveTimingResourceEvaluation.
This detection relies on SCEV.