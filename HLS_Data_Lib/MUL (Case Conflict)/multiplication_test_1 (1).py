import os
import sys
import re
import time
import json
import time
import random
start = time.time()

def search_file(path,name):
    for root, dirs, files in os.walk(path):  # path 为根目录
        if name in dirs or name in files:
            flag = 1      #判断是否找到文件
            root = str(root)
            dirs = str(dirs)
            return os.path.join(root, dirs)
    return -1

def find_value_in_report(line,target):
    target = "<"+target+" ="
    tmpA = line[line.find(target)+len(target)+1:]
    tmpB = tmpA[:tmpA.find(">")+len(">")-1]
    return tmpB

source_file_name = "../top.cc"
tcl_file_name = "run.tcl"
dump_resource_file_name = "resource.json"
dump_timing_file_name = "timing.json"
overview_file_name = "overview"

source_file = open(source_file_name, "w")
tcl_file = open(tcl_file_name, "w")

template_source_code = """
#include<cstdio>
#include<iostream>
#include<ap_int.h>
#include "ap_utils.h"

using namespace std;

ap_int<ZZZZZ> top(ap_int<XXXXX> a, ap_int<YYYYY> b)
{


	int i = 0;
	ap_int<ZZZZZ> c;
	c = b + a;
	return c;
}
"""

tcl_source_code = """
open_project simloop_frequency
set_top top
add_files FFFFF
open_solution "solution1"
set_part {xa7a12tcsg325-1q}
create_clock -period CLKCLK -name default
csynth_design
exit
"""

resource_dict = dict()
delay_dict = dict()

dump_resource_file = open(dump_resource_file_name,'a')
dump_timing_file = open(dump_timing_file_name,'a')
overview_file = open(overview_file_name,'w')

path_str_split = os.getcwd().split("/")

for it in path_str_split:
    if (it.find("simloop_frequency")>=0):
        N_it = int(it.replace("simloop_frequency",""))
        break

N_it = N_it * 1 + 1
rand1 = list(range(4*N_it, 66,2))
rand2 = list(range(8, 66,2))
rand3 = list(range(8, 66,2))

for oprandA in rand3:
    for oprandB in rand2:
        for oprandC in rand1[:2]:
            if (oprandC <= oprandB and oprandC <= oprandA):
                continue
            if (oprandC > oprandA + 8):
                continue
            if (oprandC > oprandB + 8):
                continue
            if (oprandA > oprandB):
                continue
            source_code_string = template_source_code.replace("XXXXX",str(oprandA)).replace("YYYYY",str(oprandB)).replace("ZZZZZ",str(oprandC))               
            check1 = ""
            check2 = ""
            check3 = ""
            check4 = ""
            for clock_rate in [3,4,5,6,7,8,9,10,12.5,15,16,17.5,20,25,30]:

                # generate source code and corresponging tcl commands

                print("==========================================================")
                print("trying "+str(oprandA)+"x"+str(oprandB)+"=>"+str(oprandC)+"    @clk_period="+str(clock_rate))
                print("==========================================================")

                source_file = open(source_file_name, "w")
                tcl_file = open(tcl_file_name, "w") 

                tcl_string = tcl_source_code.replace("CLKCLK",str(clock_rate)).replace("FFFFF",source_file_name)
                print(source_code_string,file=source_file)
                print(tcl_string,file=tcl_file)

                source_file.close()
                tcl_file.close()

                # execute HLS
                # os.system("vivado_hls -f "+tcl_file_name+"> hls_log")
                os.system("vivado_hls -f "+tcl_file_name+"> hls_log")

                # find the report
                result_rpt = str(search_file("./","top.verbose.rpt")).replace("[]","top.verbose.rpt")

                print("find report:        "+result_rpt)

                # get the cost in the report

                report_file = open(result_rpt, "r")
                report_content = []

                for line in report_file.readlines():
                    report_content.append(line)
                
                report_file.close()

                # resource

                for line in report_content:
                    if (line.find("Operation")>=0 and line.find("Functional Unit")>=0):
                        list_obj = line.split("|")
                for line in report_content:
                    if (line.find("|    add  ")>=0):
                        split_tmp = line.split("|")
                        DSP48E_N = 0
                        FF_N = 0
                        LUT_N = 0
                        for (num,obj) in zip(split_tmp,list_obj):
                            if (obj.find("DSP")>=0):
                                DSP48E_N = int(num)
                            if (obj.find("FF")>=0):
                                FF_N = int(num)
                            if (obj.find("LUT")>=0):
                                LUT_N = int(num)
                        resource_dict[str(oprandA)+" "+str(oprandB)+" "+str(clock_rate)] = [DSP48E_N,FF_N,LUT_N]
                        
                        print("DSP48E: "+str(DSP48E_N)+" --- FF: "+str(FF_N)+" --- LUT: "+str(LUT_N))
                        break

                # delay
                for line in report_content:
                    if (line.find(":1")>=0 and line.find("\'add\'")>=0):
                        lat_tmp = int(find_value_in_report(line,"Latency"))
                        delay_tmp = float(find_value_in_report(line,"Delay"))
                        II_tmp = int(find_value_in_report(line,"II"))
                        Core_tmp = str(find_value_in_report(line,"Core")).replace("\"","")
                        delay_dict[str(oprandA)+" "+str(oprandB)+" "+str(clock_rate)] = [lat_tmp, delay_tmp, II_tmp, Core_tmp]
                        print("Lat: "+str(lat_tmp)+" --- Delay: "+str(delay_tmp)+" --- II: "+str(II_tmp)+" --- Core: "+str(Core_tmp))
                        break
        
                check = str(oprandA)+" "+str(oprandB)+" "+str(oprandC)+" "+str(clock_rate)+" "+str(DSP48E_N)+" "+str(FF_N)+" "+str(LUT_N)+" "+str(lat_tmp)+" "+str(delay_tmp)+" "+str(II_tmp)+" "+str(Core_tmp)
                check5 = str(oprandA)+" "+str(oprandB)+" "+str(oprandC)+" "+str(DSP48E_N)+" "+str(FF_N)+" "+str(LUT_N)+" "+str(lat_tmp)+" "+str(delay_tmp)+" "+str(II_tmp)+" "+str(Core_tmp)
                
                overview_file.write(check+"\n")
                overview_file.write(str(oprandB)+" "+str(oprandA)+" "+str(oprandC)+" "+str(clock_rate)+" "+str(DSP48E_N)+" "+str(FF_N)+" "+str(LUT_N)+" "+str(lat_tmp)+" "+str(delay_tmp)+" "+str(II_tmp)+" "+str(Core_tmp)+"\n")
                print("===================== "+ check)
                
                if (check1 == check2 and check1 == check3 and check1 == check4 and check1 == check5):
                    if (delay_tmp<=clock_rate):
                        break   
                if (delay_tmp<=clock_rate):
                    check1 = check2
                    check2 = check3
                    check3 = check4
                    check4 = check5
                end = time.time()
                print("time:"+str((end - start)))

json.dump(resource_dict,dump_resource_file,ensure_ascii=False)
dump_resource_file.write('\n')
dump_resource_file.close()

json.dump(delay_dict,dump_timing_file,ensure_ascii=False)
dump_timing_file.write('\n')
dump_timing_file.close()

overview_file.close()
            


