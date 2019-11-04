# example : python microTest_dmulGen.py -n 12 -p xc7z020clg484-1
instName = "dmul"
startBitwidth = -1
endBitwidth = -1

import os
import sys
import re
import time
import json  
import time
from optparse import OptionParser  
from subprocess import Popen,PIPE
from multiprocessing import Process

def search_file(path,name):
    for root, dirs, files in os.walk(path):  # path 为根目录
        if name in dirs or name in files:
            flag = 1      #判断是否找到文件
            root = str(root)
            dirs = str(dirs)
            return os.path.join(root, dirs)
    return -1

def runMicroBenchmarks(pyScriptName):
    print('python %s' %pyScriptName)
    os.system("python " + pyScriptName)

parser = OptionParser()  
parser.add_option("-n", "--num_threads", 
                  type="int",
                  dest="num_threads",  
                  help="how many thread to execute micro-benchmark to collect instruction", 
                  metavar="FILE")  
parser.add_option("-p", "--platform", 
                  dest="platform_name",  
                  help="specify the platform", 
                  metavar="PLATFORM")  
  
(options, args) = parser.parse_args()  

path_str_split = os.path.abspath(sys.argv[0]).split("/")
pathHead = os.path.abspath(sys.argv[0]).replace("microTest_" + instName + "Gen.py","")

if (endBitwidth - startBitwidth==0):
    intervalLen = 1
else:
    intervalLen = (endBitwidth - startBitwidth + options.num_threads - 1) // options.num_threads

interval_head = startBitwidth



std_microBenchPyName = pathHead + "std_microTest_" + instName + ".py"
microBenchPyName = "microTest_" + instName +".py"

os.system(("(cd PATH; mkdir PLATFORM; cd PLATFORM; rm * -rf )")     \
                        .replace("PATH", pathHead)                  \
                        .replace("PLATFORM",options.platform_name))  

scripts_ForMicroBenchmarks = []
results_ForMicroBenchmarks = []
while (interval_head <= endBitwidth):
    interval_end = interval_head + intervalLen - 1
    if (interval_end > endBitwidth):
        interval_end = endBitwidth

    dirName = str("INSTNAME_STARTBITWIDTH_ENDBITWIDTH_PLATFORM")        \
                        .replace("PATH",pathHead)                       \
                        .replace("INSTNAME",instName)                   \
                        .replace("STARTBITWIDTH",str(interval_head))    \
                        .replace("ENDBITWIDTH",str(interval_end))       \
                        .replace("PLATFORM",options.platform_name)

    os.system(("(cd PATH; cd PLATFORM; mkdir DIRNAME; cp STD_MICROBENCHPYNAME DIRNAME/MICROBENCHPYNAME)")\
                            .replace("PATH", pathHead)                                      \
                            .replace("PLATFORM",options.platform_name)                      \
                            .replace("DIRNAME",dirName)                                     \
                            .replace("STD_MICROBENCHPYNAME",std_microBenchPyName)           \
                            .replace("MICROBENCHPYNAME",microBenchPyName))
    scripts_ForMicroBenchmarks.append(pathHead+"/"+options.platform_name+"/"+dirName+"/"+microBenchPyName)
    results_ForMicroBenchmarks.append(pathHead+"/"+options.platform_name+"/"+dirName+"/overview")
    interval_head += intervalLen 

processes_ForMicroBenchmarks = []
for script in scripts_ForMicroBenchmarks:
    processes_ForMicroBenchmarks.append(Process(target=runMicroBenchmarks,args=(script,)))

for tmp_proc in processes_ForMicroBenchmarks:
    tmp_proc.start()
    
for tmp_proc in processes_ForMicroBenchmarks:
    tmp_proc.join()

resultDataStrs = []
for result_file_name in results_ForMicroBenchmarks:
    subResultFile = open(result_file_name, 'r')
    resultDataStrs = resultDataStrs + subResultFile.readlines()
    subResultFile.close()

resultFile = open(pathHead+"/"+instName, 'w')
for data in resultDataStrs:
    resultFile.write(data)
resultFile.close()