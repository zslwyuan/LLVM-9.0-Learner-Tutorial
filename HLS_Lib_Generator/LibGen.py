# example : python LibGen.py -n 12 -p xc7z020clg484-1

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
pathHead = os.path.abspath(sys.argv[0]).replace( "LibGen.py","")



os.system(("(cd PATH; mkdir PLATFORM_lib)")                  \
                        .replace("PATH", pathHead)                  \
                        .replace("PLATFORM",options.platform_name))       

TopLibDir = pathHead + "/" + options.platform_name + "_lib"

inst2python = {
    "dmul":"/NonBitSensitive/dmul_LibGen/microTest_dmulGen.py",
    "fmul":"/NonBitSensitive/fmul_LibGen/microTest_fmulGen.py",
    "fdiv":"/NonBitSensitive/fdiv_LibGen/microTest_fdivGen.py",
    "dcmp":"/NonBitSensitive/dcmp_LibGen/microTest_dcmpGen.py",
    "store":"/NonBitSensitive/store_LibGen/microTest_storeGen.py",
    "fadd":"/NonBitSensitive/fadd_LibGen/microTest_faddGen.py",
    "load":"/NonBitSensitive/load_LibGen/microTest_loadGen.py",
    "dadd":"/NonBitSensitive/dadd_LibGen/microTest_daddGen.py",
    "ddiv":"/NonBitSensitive/ddiv_LibGen/microTest_ddivGen.py",
    "fcmp":"/NonBitSensitive/fcmp_LibGen/microTest_fcmpGen.py",
    "fptosi":"/BitSensitive/fptosi_LibGen/microTest_fptosiGen.py", 
    "ashr":"/BitSensitive/ashr_LibGen/microTest_ashrGen.py",   
    "tadd":"/BitSensitive/tadd_LibGen/microTest_taddGen.py",
    "add":"/BitSensitive/add_LibGen/microTest_addGen.py",  
    "fptoui":"/BitSensitive/fptoui_LibGen/microTest_fptouiGen.py", 
    "and":"/BitSensitive/and_LibGen/microTest_andGen.py",  
    "sdiv":"/BitSensitive/sdiv_LibGen/microTest_sdivGen.py", 
    "shl":"/BitSensitive/shl_LibGen/microTest_shlGen.py",
    "mac":"/BitSensitive/mac_LibGen/microTest_macGen.py",
    "icmp":"/BitSensitive/icmp_LibGen/microTest_icmpGen.py", 
    "lshr":"/BitSensitive/lshr_LibGen/microTest_lshrGen.py", 
    "uitofp":"/BitSensitive/uitofp_LibGen/microTest_uitofpGen.py", 
    "sitofp":"/BitSensitive/sitofp_LibGen/microTest_sitofpGen.py", 
    "mul":"/BitSensitive/mul_LibGen/microTest_mulGen.py"
}

inst2replica = {
    "dadd": ["dsub"],
    "fadd": ["fsub"],
    "add": ["sub"],
    "sdiv" : ["srem", "udiv", "urem"],
    "and" : ["xor", "or"]
}


#generate bash shell scripts for lib generation

bashScript = open(pathHead+"/runLibGenerators.sh",'w')
bashScript_path = pathHead+"/runLibGenerators.sh"
beginLines = """

SECONDS=0
mkdir GenLog

"""


endLines = """

if (( $SECONDS > 3600 )) ; then
    let "hours=SECONDS/3600"
    let "minutes=(SECONDS%3600)/60"
    let "seconds=(SECONDS%3600)%60"
    echo "Completed in $hours hour(s), $minutes minute(s) and $seconds second(s)" 
elif (( $SECONDS > 60 )) ; then
    let "minutes=(SECONDS%3600)/60"
    let "seconds=(SECONDS%3600)%60"
    echo "Completed in $minutes minute(s) and $seconds second(s)"
else
    echo "Completed in $SECONDS seconds"
fi

"""

bashScript.write(beginLines)
for inst in inst2python.keys():
    bashScript.write(("python .EXEC -n PROCS -p PLATFORM > GenLog/microTest_INSTGen\n")   \
                                        .replace("EXEC", inst2python[inst])               \
                                        .replace("PROCS", str(options.num_threads))
                                        .replace("PLATFORM", options.platform_name)
                                        .replace("INST", inst)
                                        )  

bashScript.write(endLines)
bashScript.close()

os.system(("(cd PATHHEAD ; bash runLibGenerators.sh)") \
                    .replace("PATHHEAD", pathHead)     )  

# copy those non-constant libs
for inst in inst2python.keys():
    instdir_comp = inst2python[inst].split("/")
    inst_dir = pathHead + "/" + instdir_comp[0] + "/" + instdir_comp[1] + "/" + instdir_comp[2] + "/"
    os.system(("(cd TOPLIB ; mkdir INSTDIR)")                                          \
                        .replace("TOPLIB", TopLibDir)               \
                        .replace("INSTDIR", inst))  

    os.system(("(cp FROM TO)")                                          \
                        .replace("FROM", inst_dir + "/" + inst)               \
                        .replace("TO", TopLibDir + "/" + inst + "/" + inst))  

    if (inst in inst2replica.keys()):
        for replic_inst in inst2replica[inst]:
            os.system(("(cd TOPLIB ; mkdir INSTDIR)")                                          \
                                .replace("TOPLIB", TopLibDir)               \
                                .replace("INSTDIR", replic_inst))  

            os.system(("(cp FROM TO)")                                          \
                                .replace("FROM", inst_dir + "/" + inst)               \
                                .replace("TO", TopLibDir + "/" + replic_inst + "/" + replic_inst))  

# copy those non-constant lib
os.system(("(cd HEADPATH/Constant ; cp * -r TOPLIB)")                                          \
                    .replace("TOPLIB", TopLibDir)               \
                    .replace("HEADPATH", pathHead))  


# SECONDS=0


# mkdir GenLog
# python ./NonBitSensitive/dmul_LibGen/microTest_dmulGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_dmulGen 
# python ./NonBitSensitive/fmul_LibGen/microTest_fmulGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_fmulGen 
# python ./NonBitSensitive/fdiv_LibGen/microTest_fdivGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_fdivGen 
# python ./NonBitSensitive/dcmp_LibGen/microTest_dcmpGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_dcmpGen 
# python ./NonBitSensitive/store_LibGen/microTest_storeGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_storeGen
# python ./NonBitSensitive/fadd_LibGen/microTest_faddGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_faddGen 
# python ./NonBitSensitive/load_LibGen/microTest_loadGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_loadGen 
# python ./NonBitSensitive/dadd_LibGen/microTest_daddGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_daddGen 
# python ./NonBitSensitive/ddiv_LibGen/microTest_ddivGen.py -n 12 -p xc7z020clg484-1 >  GenLog/microTest_ddivGen
# python ./NonBitSensitive/fcmp_LibGen/microTest_fcmpGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_fcmpGen 
# python ./BitSensitive/fptosi_LibGen/microTest_fptosiGen.py -n 12 -p xc7z020clg484-1 > GenLog/microTest_fptosiGen 
# python ./BitSensitive/ashr_LibGen/microTest_ashrGen.py -n 12 -p xc7z020clg484-1  > GenLog/microTest_ashrGen  
# python ./BitSensitive/tadd_LibGen/microTest_taddGen.py -n 12 -p xc7z020clg484-1   > GenLog/microTest_taddGen 
# python ./BitSensitive/add_LibGen/microTest_addGen.py -n 12 -p xc7z020clg484-1     > GenLog/microTest_addGen  
# python ./BitSensitive/fptoui_LibGen/microTest_fptouiGen.py -n 12 -p xc7z020clg484-1   > GenLog/microTest_fptouiGen 
# python ./BitSensitive/and_LibGen/microTest_andGen.py -n 12 -p xc7z020clg484-1    > GenLog/microTest_andGen  
# python ./BitSensitive/sdiv_LibGen/microTest_sdivGen.py -n 12 -p xc7z020clg484-1    > GenLog/microTest_sdivGen  
# python ./BitSensitive/shl_LibGen/microTest_shlGen.py -n 12 -p xc7z020clg484-1     > GenLog/microTest_shlGen 
# python ./BitSensitive/mac_LibGen/microTest_macGen.py -n 12 -p xc7z020clg484-1     > GenLog/microTest_macGen 
# python ./BitSensitive/icmp_LibGen/microTest_icmpGen.py -n 12 -p xc7z020clg484-1    > GenLog/microTest_icmpGen  
# python ./BitSensitive/lshr_LibGen/microTest_lshrGen.py -n 12 -p xc7z020clg484-1    >   GenLog/microTest_lshrGen 
# python ./BitSensitive/uitofp_LibGen/microTest_uitofpGen.py -n 12 -p xc7z020clg484-1   > GenLog/microTest_uitofpGen 
# python ./BitSensitive/sitofp_LibGen/microTest_sitofpGen.py -n 12 -p xc7z020clg484-1    > GenLog/microTest_sitofpGen 
# python ./BitSensitive/mul_LibGen/microTest_mulGen.py -n 12 -p xc7z020clg484-1    >  GenLog/microTest_mulGen 

# if (( $SECONDS > 3600 )) ; then
#     let "hours=SECONDS/3600"
#     let "minutes=(SECONDS%3600)/60"
#     let "seconds=(SECONDS%3600)%60"
#     echo "Completed in $hours hour(s), $minutes minute(s) and $seconds second(s)" 
# elif (( $SECONDS > 60 )) ; then
#     let "minutes=(SECONDS%3600)/60"
#     let "seconds=(SECONDS%3600)%60"
#     echo "Completed in $minutes minute(s) and $seconds second(s)"
# else
#     echo "Completed in $SECONDS seconds"
# fi