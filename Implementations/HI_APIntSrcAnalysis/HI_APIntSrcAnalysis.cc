
#include "HI_APIntSrcAnalysis.h"

// print the detailed information of the type
void HI_APIntSrcAnalysis_Visitor::printTypeInfo(const Type *T)
{

    *parseLog << "    TypeClassName: " << T->getTypeClassName() << " ";
    if (const BuiltinType *BiT = dyn_cast<const BuiltinType>(T))
    {
        *parseLog << " BuiltinType : " << BiT->getNameAsCString(PP()) << "\n";
    }
    else if (const RecordType *RT = dyn_cast<const RecordType>(T))
    {
        // if (ST->isStructureType())
        *parseLog << " RecordType : " << RT->getDecl()->getKindName() << "\n";
    }
    else if (const TagType *TT = dyn_cast<const TagType>(T))
    {
        // if (ST->isStructureType())
        *parseLog << " TagType : " << TT->getDecl()->getKindName() << " at "
                  << TT->getDecl()->getBeginLoc().printToString(CI.getSourceManager()) << " \n";
    }
    else if (const TemplateSpecializationType *TST = dyn_cast<const TemplateSpecializationType>(T))
    {
        // if (ST->isStructureType())
        *parseLog << " TemplateSpecializationType : ";
        TemplateName TN = TST->getTemplateName();
        TN.print(*parseLog, PP());
        *parseLog << " with args: ";
        auto Args = llvm::makeArrayRef(TST->getArgs(), TST->getNumArgs());
        for (auto arg : Args)
        {
            arg.print(PP(), *parseLog);
            *parseLog << " ";
        }
        *parseLog << " \n";
    }
    parseLog->flush();
}

// check whether it is a template structure like XXXX<X>
bool HI_APIntSrcAnalysis_Visitor::isAPInt(VarDecl *VD)
{
    const Type *T = VD->getType().getTypePtr();
    if (const TemplateSpecializationType *TST = dyn_cast<const TemplateSpecializationType>(T))
    {
        return true;
    }
    return false;
}

// get tht template name
std::string HI_APIntSrcAnalysis_Visitor::getAPIntName(VarDecl *VD)
{
    const Type *T = VD->getType().getTypePtr();
    if (const TemplateSpecializationType *TST = dyn_cast<const TemplateSpecializationType>(T))
    {
        std::string tmp = "";
        llvm::raw_string_ostream APIntName(tmp);
        TST->getTemplateName().print(APIntName, PP());
        return APIntName.str();
    }
    return "NULL";
}