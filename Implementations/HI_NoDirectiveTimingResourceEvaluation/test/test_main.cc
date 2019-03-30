#include "HI_NoDirectiveTimingResourceEvaluation_TEST.h"

int main()
{
    HI_NoDirectiveTimingResourceEvaluation_TEST* testObj = new HI_NoDirectiveTimingResourceEvaluation_TEST("config.txt");
    testObj->Parse_Config();
    testObj->Load_Instruction_Info();
    while (1)
    {
        int opBitWid, outBitWid;
        std::string ClockPerid,opcode;
        std::cin >> opcode >> opBitWid >> outBitWid >> ClockPerid;
        // int DSP = testObj->get_N_DSP(opcode, opBitWid , outBitWid, ClockPerid);
        // int FF = testObj->get_N_FF(opcode, opBitWid , outBitWid, ClockPerid);
        // int LUT = testObj->get_N_LUT(opcode, opBitWid , outBitWid, ClockPerid);
        // int Lat = testObj->get_N_Lat(opcode, opBitWid , outBitWid, ClockPerid);
        // double Delay = testObj->get_N_Delay(opcode, opBitWid , outBitWid, ClockPerid);
        testObj->get_inst_info(opcode, opBitWid , outBitWid, ClockPerid).print();
    }
}
