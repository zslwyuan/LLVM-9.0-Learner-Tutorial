#include "ConfigGen.h"

std::string findSlowestLastLevelLoop_InSlowestTopLoop(
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
    )
{
    int max_latency = -1;
    std::string slowest_sub_loop = "";
    for (auto IRLoop_label_pair : IRLoop2LoopLabel)
    {
        std::string irName = IRLoop_label_pair.first;
        std::string loopLabel = IRLoop_label_pair.second;
        if (LoopIRName2Depth[irName] == 1)
        {
            if (LoopLabel2Latency.find(IRLoop2LoopLabel[irName]) != LoopLabel2Latency.end())
            {
                int curLatency = LoopLabel2Latency[IRLoop2LoopLabel[irName]].latency;
                if (curLatency > max_latency)
                {
                    max_latency = curLatency;
                    slowest_sub_loop = irName;
                }
            }
        }
    }
    
    assert(slowest_sub_loop!= "");

    if (LoopIRName2NextLevelSubLoopIRNames.find(slowest_sub_loop) != LoopIRName2NextLevelSubLoopIRNames.end())
        return findTheSlowestLoop(slowest_sub_loop, IRLoop2LoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    else
        return slowest_sub_loop;    
}


std::string findTheSlowestLoop(
                    std::string cur_LoopIRName,
                    std::map<std::string, std::string> &IRLoop2LoopLabel,
                    std::map<std::string, HI_WithDirectiveTimingResourceEvaluation::timingBase> &LoopLabel2Latency, 
                    std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                    std::map<std::string, int> &LoopIRName2Depth
                    )
{
    if (LoopIRName2NextLevelSubLoopIRNames.find(cur_LoopIRName) != LoopIRName2NextLevelSubLoopIRNames.end())
    {
        int max_latency = -1;
        std::string slowest_sub_loop = "";
        for (auto next_level_IRname : LoopIRName2NextLevelSubLoopIRNames[cur_LoopIRName])
        {
            if (LoopLabel2Latency.find(IRLoop2LoopLabel[next_level_IRname]) != LoopLabel2Latency.end())
            {
                int curLatency = LoopLabel2Latency[IRLoop2LoopLabel[next_level_IRname]].latency;
                if (curLatency > max_latency)
                {
                    max_latency = curLatency;
                    slowest_sub_loop = next_level_IRname;
                }
            }
        }
        if (slowest_sub_loop == "")
            return cur_LoopIRName;
        assert(slowest_sub_loop!= "");
        return findTheSlowestLoop(slowest_sub_loop, IRLoop2LoopLabel, LoopLabel2Latency, LoopIRName2NextLevelSubLoopIRNames, LoopIRName2Depth);
    }
    else
    {
        if (LoopLabel2Latency.find(IRLoop2LoopLabel[cur_LoopIRName]) != LoopLabel2Latency.end())
        {
            return cur_LoopIRName;
        }
    }
    assert(false && "should not reach here");
}



std::map<std::string, std::vector<int>> getPotentialLoopUnrollFactor(
                                            std::map<std::string, std::string> &IRLoop2LoopLabel,
                                            std::map<std::string, std::vector<std::string>> &LoopIRName2NextLevelSubLoopIRNames,
                                            std::map<std::string, int> &LoopIRName2Depth,
                                            std::map<std::string, int> &IRLoop2OriginTripCount
                                        )
{
    std::map<std::string, std::vector<int>> res_map;
    res_map.clear();
    
    std::string slowest_sub_loop = "";
    for (auto IRLoop_label_pair : IRLoop2LoopLabel)
    {
        std::vector<int> unrollFactorForLoop;
        int tripCount = IRLoop2OriginTripCount[IRLoop_label_pair.first];
        for (int i=2; i<=64; i++)
        {
            if (isPowerOf2_32(i) || tripCount%i==0 )
            {
                unrollFactorForLoop.push_back(i);
            }
        }
        res_map[IRLoop_label_pair.first]=unrollFactorForLoop;
    }
    return res_map;
}

int findNextUnrollFactor(   std::string IRLoopName,
                            int cur_loopUnrollFactor,
                            std::map<std::string, std::vector<int>> &loop2PotentialUnrollFactors
                         )
{
    for (auto nextFactor : loop2PotentialUnrollFactors[IRLoopName])
    {
        if (nextFactor > cur_loopUnrollFactor)
        {
            return nextFactor;
        }
    }
    return -1;
}