//***************************************************************************************************************************************************
//* BSD 3-Clause License
//*
//* Copyright (c) 2016, Rene Thrane
//* All rights reserved.
//* 
//* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//* 
//* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
//*    documentation and/or other materials provided with the distribution.
//* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this 
//*    software without specific prior written permission.
//* 
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
//* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
//* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
//* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
//* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
//* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//***************************************************************************************************************************************************

#include <RAIIGen/Generator/Simple/SimpleGenerator.hpp>
#include <RAIIGen/Generator/FunctionNamePair.hpp>
#include <RAIIGen/Generator/MatchedFunctionPair.hpp>
#include <RAIIGen/Generator/Simple/AnalyzeMode.hpp>
#include <RAIIGen/Generator/Simple/GenerateMethodCode.hpp>
#include <RAIIGen/Generator/Simple/MemberVariable.hpp>
#include <RAIIGen/Generator/Simple/MethodArgument.hpp>
#include <RAIIGen/Generator/Simple/Snippets.hpp>
#include <RAIIGen/Generator/Simple/FullAnalysis.hpp>
#include <RAIIGen/Generator/Simple/ParamInStruct.hpp>
#include <RAIIGen/Generator/ConfigUtil.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/StringHelper.hpp>
#include <RAIIGen/IOUtil.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/IO/PathDeque.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <RAIIGen/Generator/Simple/Enum/EnumToStringLookup.hpp>
#include <RAIIGen/Generator/Simple/Format/FormatToCpp.hpp>
#include <RAIIGen/Generator/Simple/Struct/CStructToCpp.hpp>


using namespace Fsl;

namespace MB
{
  using namespace ConfigUtil;

  namespace
  {
    const auto DEFAULT_VALUE_NOT_FOUND = "FIX_DEFAULT_FOR_TYPE_NOT_DEFINED";
    typedef std::unordered_map<std::string, std::string> AddtionalContentMap;

    struct AdditionalContent
    {
      std::string Headers;
      std::string Methods;
    };

    typedef std::unordered_map<std::string, AdditionalContent> AddtionalFileContentMap;

    bool IsIgnoreParameter(const ParameterRecord& param, const std::vector<std::string>& forceNullParameter)
    {
      for (auto itr = forceNullParameter.begin(); itr != forceNullParameter.end(); ++itr)
      {
        if (*itr == param.Type.Name)
          return true;
      }
      return false;
    }


    std::string GetResourceArgumentName(const TypeRecord& type, const std::string& argumentName)
    {
      if (!type.IsPointer || argumentName.size() < 3 || !(argumentName[0] == 'p' && CaseUtil::IsUpperCase(argumentName[1])))
        return argumentName;

      std::string moddedName(argumentName.substr(1));
      return CaseUtil::LowerCaseFirstCharacter(moddedName);
    }

    std::string GetResourceArgumentName(const ParameterRecord& value)
    {
      return GetResourceArgumentName(value.Type, value.ArgumentName);
    }


    MemberVariable ToMemberVariable(const ParameterRecord& value, const std::string& memberName)
    {
      return MemberVariable(value.Type, value.Type.Name, "m_" + memberName, memberName, CaseUtil::UpperCaseFirstCharacter(memberName));
    }


    MemberVariable ToMemberVariable(const ParameterRecord& value)
    {
      return MemberVariable(value.Type, value.Type.Name, "m_" + value.ArgumentName, value.ArgumentName, CaseUtil::UpperCaseFirstCharacter(value.ArgumentName));
    }

    MemberVariable ToMemberVariable(const MemberRecord& value)
    {
      return MemberVariable(value.Type, value.Type.Name, "m_" + value.ArgumentName, value.ArgumentName, CaseUtil::UpperCaseFirstCharacter(value.ArgumentName));
    }


    MethodArgument CPPifyArgument(const TypeRecord& type, const std::string& argumentName, const bool allowPointerToReferenceConversion, const bool allowConstAdd=true)
    {
      MethodArgument result;
      result.FullType = type;
      result.ArgumentName = argumentName;
      result.ParameterValue = argumentName;

      if (type.IsConstQualified && type.IsStruct && type.IsPointer)
      {
        if (allowPointerToReferenceConversion)
        {
          result.FullTypeString = "const " + type.Name + "&";
          result.ArgumentName = GetResourceArgumentName(type, argumentName);
          result.ParameterValue = "&" + argumentName;
        }
        else
        {
          result.FullTypeString = type.Name + "*const";
          result.ArgumentName = argumentName;
          result.ParameterValue = argumentName;
        }
      }
      else if (type.IsConstQualified)
        result.FullTypeString = type.FullTypeString;
      else if (allowConstAdd && !type.IsPointer)
        result.FullTypeString = "const " + type.FullTypeString;
      else
        result.FullTypeString = type.FullTypeString;
      return result;
    }


    MethodArgument ToMethodArgument(const ParameterRecord& value, const bool allowPointerToRefConvert=true)
    {
      // C++'ify the arguments
      std::string fullTypeString;
      std::string argumentName(value.ArgumentName);
      std::string parameterName(value.ArgumentName);

      if (allowPointerToRefConvert && value.Type.IsConstQualified && value.Type.IsStruct && value.Type.IsPointer)
      {
        fullTypeString = "const " + value.Type.Name + "&";
        argumentName = GetResourceArgumentName(value);
        parameterName = "&" + argumentName;
      }
      else if (value.Type.IsConstQualified)
        fullTypeString = value.Type.FullTypeString;
      else if (! value.Type.IsPointer)
        fullTypeString = "const " + value.Type.FullTypeString;
      else
        fullTypeString = value.Type.FullTypeString;

      return MethodArgument(value.Type, fullTypeString, argumentName, parameterName);
    }


    MethodArgument ToMethodArgument(const MemberVariable& value)
    {
      return CPPifyArgument(value.FullType, value.ArgumentName, true);
    }


    MethodArgument ToMethodArgument(const MemberRecord& value, const bool allowPointerToReferenceConversion=true)
    {
      return CPPifyArgument(value.Type, value.ArgumentName, allowPointerToReferenceConversion);
    }


    std::string TypeToVariableName(const std::string& typeNamePrefix, const std::string& typeName)
    {
      if( typeName.find(typeNamePrefix) != 0 )
        return typeName;
      return StringHelper::EnforceLowerCamelCaseNameStyle(typeName.substr(typeNamePrefix.size()));
    }

    ParamInStruct LookupParameterInStruct(const Capture& capture, const std::deque<MethodArgument>& createParameters, const ParameterRecord& findParameter)
    {
      auto structDict = capture.GetStructDict();

      for (auto itr = createParameters.begin(); itr != createParameters.end(); ++itr)
      {
        if (itr->FullType.IsStruct)
        {
          const auto itrStruct = structDict.find(itr->FullType.Name);
          if (itrStruct != structDict.end())
          {
            for (auto itrStructMembers = itrStruct->second.Members.begin(); itrStructMembers != itrStruct->second.Members.end(); ++itrStructMembers)
            {
              if (itrStructMembers->Type == findParameter.Type && itrStructMembers->ArgumentName == findParameter.ArgumentName)
                return ParamInStruct(*itr, itrStruct->second, *itrStructMembers);
            }
          }
        }
      }
      throw NotSupportedException("The destroy parameter could not be found");
    }


    AnalysisResult AnalyzeCreate(std::unordered_map<std::string, MethodArgument>& rDstMap, const SimpleGeneratorConfig& config, const MatchedFunctionPair& functions, const std::vector<std::string>& forceNullParameter, const AnalyzeMode analyzeMode, const std::string& paramMemberArrayCountName, const bool singleElementDelete)
    {
      bool rResourceParameterFound = false;
      AnalysisResult result;
      for (auto itr = functions.Create.Parameters.begin(); itr != functions.Create.Parameters.end(); ++itr)
      {
        const auto typeName = itr->Type.Name;
        if (itr->ParamType == ParameterType::ErrorCode)
        {
          auto createArgument = ToMethodArgument(*itr);
          if (itr->Type.IsPointer)
            createArgument.ParameterValue = std::string("&") + createArgument.ParameterValue;
          result.CreateArguments.push_back(createArgument);
        }
        else
        {
          auto itrFind = std::find_if(functions.Destroy.Parameters.begin(), functions.Destroy.Parameters.end(), [typeName](const ParameterRecord& val) { return val.Type.Name == typeName; });
          if (itrFind != functions.Destroy.Parameters.end())
          {
            if (!IsIgnoreParameter(*itr, forceNullParameter))
            {
              if (!itr->Type.IsConstQualified && itr->Type.IsPointer)
              {
                rResourceParameterFound = true;
                // The non const pointer is normally the 'class' type of the create function
                std::cout << "  Param matched: " << itr->Type.Name << " (*)\n";
                const auto fixedArgumentName = GetResourceArgumentName(*itr);
                result.ResourceMemberVariable = ToMemberVariable(*itr, fixedArgumentName);
                result.IntermediaryName = result.ResourceMemberVariable.ArgumentName;


                { // Build the create argument
                  auto createArgument = ToMethodArgument(*itr);
                  if (itr->Type.IsPointer)
                  {
                    if (analyzeMode == AnalyzeMode::VectorInstance)
                    {
                      createArgument.ParameterValue = result.IntermediaryName + ".data()";
                      result.ResourceMemberVariable.IsMoveable = true;
                      result.ResourceMemberVariable.IsPOD = false;
                    }
                    else
                      createArgument.ParameterValue = std::string("&") + result.IntermediaryName;
                  }
                  else
                    createArgument.ParameterValue = result.IntermediaryName;
                  result.CreateArguments.push_back(createArgument);
                }
                { // Build the destroy argument
                  auto destroyArgument = ToMethodArgument(*itr);
                  if (itrFind->Type.IsPointer)
                  {
                    if (analyzeMode == AnalyzeMode::VectorInstance)
                      destroyArgument.ParameterValue = result.ResourceMemberVariable.Name + ".data()";
                    else
                      destroyArgument.ParameterValue = "&" + result.ResourceMemberVariable.Name;
                  }
                  else
                  {
                    if (analyzeMode == AnalyzeMode::VectorInstance && singleElementDelete)
                    {
                      destroyArgument.ParameterValue = result.ResourceMemberVariable.Name + "[i]";
                    }
                    else
                      destroyArgument.ParameterValue = result.ResourceMemberVariable.Name;
                  }
                  rDstMap[itrFind->ArgumentName] = destroyArgument;
                }
              }
              else
              {
                std::cout << "  Param matched: " << itr->Type.Name << "\n";

                auto member = ToMemberVariable(*itr);
                auto destroyArgument = ToMethodArgument(*itr);

                if (analyzeMode == AnalyzeMode::Normal ||  itr->ArgumentName != paramMemberArrayCountName)
                {
                  result.AdditionalMemberVariables.push_back(member);
                  result.MethodArguments.push_back(ToMethodArgument(*itr));
                  result.CreateArguments.push_back(ToMethodArgument(*itr));
                  destroyArgument.ParameterValue = member.Name;
                }
                else if (analyzeMode == AnalyzeMode::SingleInstance)
                {
                  result.AdditionalMemberVariables.push_back(member);
                  result.MethodArguments.push_back(ToMethodArgument(*itr));
                  result.CreateArguments.push_back(ToMethodArgument(*itr));
                  destroyArgument.ParameterValue = "1";
                }
                else
                {
                  result.AdditionalMemberVariables.push_back(member);
                  result.MethodArguments.push_back(ToMethodArgument(*itr));
                  result.CreateArguments.push_back(ToMethodArgument(*itr));
                  destroyArgument.ParameterValue = member.Name + ".size()";
                }
                rDstMap[itrFind->ArgumentName] = destroyArgument;
              }
            }
            else
            {
              std::cout << "  Param force ignored: " << itr->Type.Name << "\n";

              auto createArgument = ToMethodArgument(*itr);
              createArgument.ParameterValue = "nullptr";
              result.CreateArguments.push_back(createArgument);

              auto destroyArgument = ToMethodArgument(*itr);
              destroyArgument.ParameterValue = "nullptr";
              rDstMap[itrFind->ArgumentName] = destroyArgument;
            }
          }
          else
          {
            std::cout << "  Param not matched: " << itr->Type.Name << "\n";
            if (analyzeMode != AnalyzeMode::SingleInstance || itr->ArgumentName != paramMemberArrayCountName)
            {
              result.MethodArguments.push_back(ToMethodArgument(*itr));
              result.CreateArguments.push_back(ToMethodArgument(*itr));
            }
            else if (analyzeMode == AnalyzeMode::SingleInstance)
            {
              auto arg = ToMethodArgument(*itr);
              arg.ParameterValue = "1";
              result.CreateArguments.push_back(arg);
            }
          }
        }
      }

      if (!rResourceParameterFound)
      {
        // So no members matched the params to the destroy method, lets check if the return type matches
        const auto typeName = functions.Create.ReturnType.Name;
        auto itrFind = std::find_if(functions.Destroy.Parameters.begin(), functions.Destroy.Parameters.end(), [typeName](const ParameterRecord& val) { return val.Type.Name == typeName; });
        if (itrFind == functions.Destroy.Parameters.end())
          throw NotSupportedException(std::string("Could not find created resource parameter for method: ") + functions.Create.Name);

        // We found a return type that matches a destroy parameter
        result.IntermediaryName = TypeToVariableName(config.TypeNamePrefix, typeName);
        result.ResourceMemberVariable = MemberVariable(functions.Create.ReturnType, typeName, "m_" + result.IntermediaryName, result.IntermediaryName, CaseUtil::UpperCaseFirstCharacter(result.IntermediaryName));

        { // Build the destroy argument
          auto destroyArgument = ToMethodArgument(*itrFind);
          if (itrFind->Type.IsPointer)
          {
            if(analyzeMode == AnalyzeMode::VectorInstance)
              destroyArgument.ParameterValue = result.ResourceMemberVariable.Name + ".data()";
            else
              destroyArgument.ParameterValue = "&" + result.ResourceMemberVariable.Name;
          }
          else
            destroyArgument.ParameterValue = result.ResourceMemberVariable.Name;
          rDstMap[itrFind->ArgumentName] = destroyArgument;
        }

        std::cout << "  Return type used as resource: " << typeName << " (*)\n";
        // Check intermediary name for collisions
        for (auto itr = functions.Create.Parameters.begin(); itr != functions.Create.Parameters.end(); ++itr)
        {
          if (itr->Name == result.IntermediaryName)
            throw NotSupportedException("Intermediary name collides with parameter name");
        }
      }

      if (!result.ResourceMemberVariable.IsValid())
        throw NotSupportedException("The resource type could not be determined");
      return result;
    }


    void AnalyzeDestroy(AnalysisResult& rResult, const Capture& capture, const SimpleGeneratorConfig& config, const MatchedFunctionPair& functions, const AnalyzeMode analyzeMode, const std::unordered_map<std::string, MethodArgument>& dstMap)
    {
      const std::string createMethodName = functions.Create.Name;
      const auto itrCustom = std::find_if(config.RAIIClassCustomizations.begin(), config.RAIIClassCustomizations.end(), [createMethodName](const RAIIClassCustomization& val) { return val.SourceCreateMethod == createMethodName; });
      for (auto itr = functions.Destroy.Parameters.begin(); itr != functions.Destroy.Parameters.end(); ++itr)
      {
        auto itrFindDst = dstMap.find(itr->ArgumentName);
        if (itrFindDst != dstMap.end())
          rResult.DestroyArguments.push_back(itrFindDst->second);
        else
        {
          auto found = LookupParameterInStruct(capture, rResult.CreateArguments, *itr);
          auto asMember = ToMemberVariable(found.StructMember);
          asMember.SourceArgumentName = found.SourceParameter.ArgumentName + "." + asMember.ArgumentName;
          auto asArgument = ToMethodArgument(asMember);

          if (itrCustom != config.RAIIClassCustomizations.end() && itrCustom != config.RAIIClassCustomizations.end() && itrCustom->StructMemberArrayCountName == asMember.ArgumentName)
          {
            switch (analyzeMode)
            {
            case AnalyzeMode::SingleInstance:
              rResult.DestroyArguments.push_back(MethodArgument(asArgument.FullType, asArgument.FullTypeString, "1"));
              rResult.ResourceCountVariableName = asMember.SourceArgumentName;
              break;
            case AnalyzeMode::VectorInstance:
              rResult.DestroyArguments.push_back(MethodArgument(asArgument.FullType, asArgument.FullTypeString, std::string("static_cast<") + asArgument.FullType.Name + ">(m_" + itrCustom->ResourceName + ".size())"));
              rResult.ResourceCountVariableName = asMember.SourceArgumentName;
              break;
            default:
              throw NotSupportedException("Unsupported analyze mode");
            }
          }
          else
          {
            rResult.AdditionalMemberVariables.push_back(asMember);
            asArgument.ParameterValue = "m_" + asArgument.ParameterValue;
            rResult.DestroyArguments.push_back(asArgument);
          }
        }
      }
    }


    AnalysisResult Analyze(const Capture& capture, const SimpleGeneratorConfig& config, const MatchedFunctionPair& functions, const std::string& lowerCamelCaseClassName, const std::vector<std::string>& forceNullParameter, const AnalyzeMode analyzeMode, const std::string& paramMemberArrayCountName, const bool singleElementDelete)
    {
      std::unordered_map<std::string, MethodArgument> dstMap;

      AnalysisResult result = AnalyzeCreate(dstMap, config, functions, forceNullParameter, analyzeMode, paramMemberArrayCountName, singleElementDelete);
      AnalyzeDestroy(result, capture, config, functions, analyzeMode, dstMap);

      std::copy(result.AdditionalMemberVariables.begin(), result.AdditionalMemberVariables.end(), std::back_inserter(result.AllMemberVariables));
      result.AllMemberVariables.push_back(result.ResourceMemberVariable);

      result.ClassName = CaseUtil::UpperCaseFirstCharacter(lowerCamelCaseClassName);
      return result;
    }

    bool IsExactStartParameterMatch(const std::deque<MemberVariable>& startParameters, const FunctionRecord& function)
    {
      if (function.Parameters.size() < startParameters.size())
        return false;

      for (std::size_t paramIndex = 0; paramIndex < startParameters.size(); ++paramIndex)
      {
        if (function.Parameters[paramIndex].Type.Name != startParameters[paramIndex].Type)
          return false;

      }
      return true;
    }


    bool IsManagedType(const std::deque<FullAnalysis>& managed, const ParameterRecord& parameter)
    {
      auto func = [parameter](const FullAnalysis& val) 
      { 
        return val.Result.ResourceMemberVariable.Type == parameter.Type.Name; 
      };
      return std::find_if(managed.begin(), managed.end(), func) != managed.end();
    }


    void AddFunctionGuard(ClassMethod& rClassMethod, const std::vector<FunctionGuard>& functionGuards)
    {
      const auto itrFind = std::find_if(functionGuards.begin(), functionGuards.end(), [rClassMethod](const FunctionGuard& entry) { return entry.Name == rClassMethod.SourceFunction.Name; });
      if (itrFind == functionGuards.end())
        return;

      rClassMethod.GuardFunctions.push_back(itrFind->DefineContent);
    }


    void AddFunctionVersionGuard(ClassMethod& rClassMethod, const SimpleGeneratorConfig& config)
    {
      if (rClassMethod.SourceFunction.Version == VersionRecord() && config.VersionGuard.IsValid)
        return;

      std::string strGuardFunction = config.VersionGuard.ToGuardString(rClassMethod.SourceFunction.Version);
      rClassMethod.GuardFunctions.push_back(strGuardFunction);
    }



    void FindObjectFunctions(const SimpleGeneratorConfig& config, const FunctionAnalysis& functionAnalysis, std::deque<FullAnalysis>& managed, FullAnalysis& rResult)
    {
      std::cout << "Matching functions to " << rResult.Result.ClassName << "\n";

      CurrentEntityInfo currentEntityInfo(rResult.Result.ClassName);

      for (auto itr = functionAnalysis.Unmatched.begin(); itr != functionAnalysis.Unmatched.end(); ++itr)
      {
        // 1. The start parameter types and order must be exactly the same as this objects member variables
        // 2. Any following parameter must not be of a 'managed' type (one that we generate a object for)
        if (IsExactStartParameterMatch(rResult.Result.AllMemberVariables, *itr) )
        {
          const std::size_t numMembers = rResult.Result.AllMemberVariables.size();
          if (itr->Parameters.size() <= numMembers || !IsManagedType(managed, itr->Parameters[numMembers]))
          {
            std::cout << "+ " << itr->Name << "\n";
            std::string methodName = itr->Name;
            if (StringUtil::StartsWith(methodName, config.FunctionNamePrefix))
              methodName = methodName.substr(config.FunctionNamePrefix.size());

            if (!HasMatchingEntry(methodName, config.FunctionNameBlacklist, currentEntityInfo))
            {

              ClassMethod classMethod;
              classMethod.SourceFunction = *itr;
              classMethod.Name = methodName;

              AddFunctionGuard(classMethod, config.FunctionGuards);
              AddFunctionVersionGuard(classMethod, config);

              if (itr->ReturnType.Name == "void")
                classMethod.Template = ClassMethod::TemplateType::Void;
              else if (itr->ReturnType.Name == config.ErrorCodeTypeName)
                classMethod.Template = ClassMethod::TemplateType::Error;
              else
              {
                classMethod.Template = ClassMethod::TemplateType::Type;
                classMethod.ReturnType = CPPifyArgument(itr->ReturnType, "Return", false, false);
              }

              // Convert all the method arguments
              for (auto &param : itr->Parameters)
                classMethod.OriginalMethodArguments.push_back(ToMethodArgument(param, false));


              // Add the starting params (the members)
              for (std::size_t paramIndex = 0; paramIndex < rResult.Result.AllMemberVariables.size(); ++paramIndex)
              {
                auto argument = ToMethodArgument(rResult.Result.AllMemberVariables[paramIndex]);
                argument.ParameterValue = rResult.Result.AllMemberVariables[paramIndex].Name;
                classMethod.CombinedMethodArguments.push_back(argument);
              }
              // Add the following params (non members)
              for (std::size_t paramIndex = rResult.Result.AllMemberVariables.size(); paramIndex < itr->Parameters.size(); ++paramIndex)
              {
                const auto argument = ToMethodArgument(itr->Parameters[paramIndex], false);
                classMethod.MethodArguments.push_back(argument);
                classMethod.CombinedMethodArguments.push_back(argument);
              }
              rResult.ClassMethods.push_back(classMethod);
            }
          }
        }
      }
    }


    std::string GenerateParameterList(const std::deque<MethodArgument>& methodArguments)
    {
      std::string result;
      bool isFirst = true;
      for (auto itr = methodArguments.begin(); itr != methodArguments.end(); ++itr)
      {
        if (!isFirst)
          result += ", ";
        else
          isFirst = false;

        result += itr->FullTypeString + " " + itr->ArgumentName;
      }
      return result;
    }

    std::string GenerateParameterNameList(const std::deque<MethodArgument>& methodArguments)
    {
      std::string result;
      bool isFirst = true;
      for (auto itr = methodArguments.begin(); itr != methodArguments.end(); ++itr)
      {
        if (!isFirst)
          result += ", ";
        else
          isFirst = false;

        result += itr->ArgumentName;
      }
      return result;
    }


    std::string GenerateExpandedParameterNameList(const std::deque<MethodArgument>& methodArguments)
    {
      std::string result;
      bool isFirst = true;
      for (auto itr = methodArguments.begin(); itr != methodArguments.end(); ++itr)
      {
        if (!isFirst)
          result += ", ";
        else
          isFirst = false;

        result += itr->ParameterValue;
      }
      return result;
    }


    bool ContainsDefaultValue(const std::string snippetTemplate)
    {
      return snippetTemplate.find("##DEFAULT_VALUE##") != std::string::npos;
    }


    std::string GetDefaultValue(const std::unordered_map<std::string, std::string>& typeDefaultValues, const std::string& type)
    {
      const auto itr = typeDefaultValues.find(type);
      return itr != typeDefaultValues.end() ? itr->second : std::string(DEFAULT_VALUE_NOT_FOUND);
    }

    void ReplaceDefaultValue(std::string& rContent, const std::string& type, const std::unordered_map<std::string, std::string>& typeDefaultValues, const Snippets& snippets)
    {
      StringUtil::Replace(rContent, "##DEFAULT_VALUE##", snippets.DefaultValueMod);
      StringUtil::Replace(rContent, "##DEFAULT_VALUE##", GetDefaultValue(typeDefaultValues, type));
    }


    std::string GenerateForAllMembersMoveSupport(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const std::string snippetTemplate, const std::string& snippetMoveTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues, const bool useSourceArgument = false)
    {
      const bool scriptUsesDefaultValue = ContainsDefaultValue(snippetTemplate);

      std::string result;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = !itr->IsMoveable ? snippetTemplate : snippetMoveTemplate;
        if (snippet.size() > 0)
        {
          StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
          StringUtil::Replace(snippet, "##MEMBER_ARGUMENT_NAME##", useSourceArgument ? itr->SourceArgumentName : itr->ArgumentName);
          if (scriptUsesDefaultValue)
          {
            ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
          }
          result += END_OF_LINE + snippet;
        }
      }
      return result;
    }

    std::string GenerateForAllMembersInvalidate(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const SnippetContext& snippetTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues, const bool useSourceArgument = false)
    {
      std::string result;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = itr->IsPOD ? snippetTemplate.POD : snippetTemplate.Complex;
        if (snippet.size() > 0)
        {
          StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
          StringUtil::Replace(snippet, "##MEMBER_ARGUMENT_NAME##", useSourceArgument ? itr->SourceArgumentName : itr->ArgumentName);
          if (ContainsDefaultValue(snippet))
          {
            ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
          }
          result += END_OF_LINE + snippet;
        }
      }
      return result;
    }


    std::string GenerateForAllMembers(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const std::string& snippetTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues, const bool useSourceArgument = false)
    {
      return GenerateForAllMembersMoveSupport(snippets, allMemberVariables, snippetTemplate, snippetTemplate, typeDefaultValues, useSourceArgument);
    }


    std::string GenerateAssertForAllMembers(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const SnippetContext& snippetTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues, const bool useSourceArgument = false)
    {
      std::string result;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = itr->IsPOD ? snippetTemplate.POD : snippetTemplate.Complex;
        if (snippet.size() > 0)
        {
          std::string snippetCommand = snippets.ResetAssertCommand;
          StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
          StringUtil::Replace(snippet, "##MEMBER_ARGUMENT_NAME##", useSourceArgument ? itr->SourceArgumentName : itr->ArgumentName);
          if (ContainsDefaultValue(snippet))
          {
            ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
          }
          StringUtil::Replace(snippetCommand, "##ASSERT_CONDITION##", snippet);
          result += END_OF_LINE + snippetCommand;
        }
      }
      return result;
    }


    std::string GenerateConstructorInitializationMoveSupport(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const std::string& snippetTemplate, const std::string snippetTemplateMove, const std::unordered_map<std::string, std::string>& typeDefaultValues)
    {
      std::string result;
      std::string seperator = ":";
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = !itr->IsMoveable ? snippetTemplate : snippetTemplateMove;
        StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
        StringUtil::Replace(snippet, "##MEMBER_SEPERATOR##", seperator);
        if (ContainsDefaultValue(snippet))
        {
          ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
        }
        seperator = ",";
        result += END_OF_LINE + snippet;
      }
      return result;
    }


    std::string GenerateConstructorInitialization(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const SnippetContext& snippetInit, const std::unordered_map<std::string, std::string>& typeDefaultValues)
    {
      std::string result;
      std::string seperator = ":";
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = itr->IsPOD ? snippetInit.POD : snippetInit.Complex;
        StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
        StringUtil::Replace(snippet, "##MEMBER_SEPERATOR##", seperator);
        if (ContainsDefaultValue(snippet))
        {
          ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
        }
        seperator = ",";
        result += END_OF_LINE + snippet;
      }
      return result;
    }


    void CheckDefaultValues(std::unordered_set<std::string>& rTypesWithoutDefaultValues, const std::deque<MemberVariable>& allMemberVariables, const std::unordered_map<std::string, std::string>& typeDefaultValues)
    {
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        if (typeDefaultValues.find(itr->Type) == typeDefaultValues.end())
        {
          rTypesWithoutDefaultValues.insert(itr->Type);
        }
      }
    }


    void AbsorbFunctions(std::deque<FullAnalysis>& rFullAnalysis, const ClassFunctionAbsorb& absorb, FullAnalysis& rAbsorbAnalysis)
    {
      std::cout << "Class " << absorb.ClassName << " absorbing function " << absorb.FunctionName << "\n";

      const std::string functionName = absorb.FunctionName;
      auto itrFindAbsorb = std::find_if(rFullAnalysis.begin(), rFullAnalysis.end(), [functionName](const FullAnalysis& val) { return val.Pair.Create.Name == functionName; });
      if (itrFindAbsorb == rFullAnalysis.end())
      {
        std::cout << "WARNING: Could not find the requested class " + functionName << "\n";
        return;
      }

      // We found the function to absorb, so lets verify that its compatible
      if (rAbsorbAnalysis.Result.AllMemberVariables != itrFindAbsorb->Result.AllMemberVariables)
        throw UsageErrorException(std::string("A absorbed functions 'RAII' class should contain the same member variables as the absorbing one"));

      if (itrFindAbsorb->AbsorbedFunctions)
        throw UsageErrorException(std::string("A absorbed function can not absorb other functions"));

      // Alocate a container if needed
      if (!rAbsorbAnalysis.AbsorbedFunctions)
        rAbsorbAnalysis.AbsorbedFunctions = std::make_shared<std::deque<FullAnalysis> >();

      itrFindAbsorb->Result.ClassName = absorb.ClassName;

      rAbsorbAnalysis.AbsorbedFunctions->push_back(*itrFindAbsorb);

      // Remove the absorbed element from the analysis as its not needed anymore
      rFullAnalysis.erase(itrFindAbsorb);
    }



    void AbsorbFunctions(std::deque<FullAnalysis>& rFullAnalysis, const std::vector<ClassFunctionAbsorb>& classFunctionAbsorbtion)
    {
      for (auto itr = classFunctionAbsorbtion.begin(); itr != classFunctionAbsorbtion.end(); ++itr)
      {
        const std::string className = itr->ClassName;

        const auto itrFindClass = std::find_if(rFullAnalysis.begin(), rFullAnalysis.end(), [className](const FullAnalysis& val) { return val.Result.ClassName == className; });
        if (itrFindClass != rFullAnalysis.end())
        {
          AbsorbFunctions(rFullAnalysis, *itr, *itrFindClass);
        }
        else
        {
          if (!itr->AllowCreate)
            throw NotFoundException(std::string("Could not find class ") + className + " and not allowed to create, so function " + itr->FunctionName + " could not be absorbed as requested");

          const std::string functionName = itr->FunctionName;
          auto itrFindAltClass = std::find_if(rFullAnalysis.begin(), rFullAnalysis.end(), [functionName](const FullAnalysis& val) { return val.Pair.Create.Name == functionName; });
          if (itrFindAltClass == rFullAnalysis.end())
            throw NotFoundException(std::string("Could not find the requested class ") + functionName);

          std::cout << "Created class " << className << " from function " << itr->FunctionName << "\n";


          itrFindAltClass->Result.ClassName = className;
        }
      }
    }


    //! Analyze the create parameters and 'unroll' the structs 
    void AnalyzeCreateFunctionStructParameters(const Capture& capture, const SimpleGeneratorConfig& config, std::deque<FullAnalysis>& rFullAnalysis)
    {
      //std::cout << "Create function parameter struct analysis\n";
      {
        auto structDict = capture.GetStructDict();
        for (auto itr = rFullAnalysis.begin(); itr != rFullAnalysis.end(); ++itr)
        {
          const std::string createMethodName = itr->Pair.Create.Name;
          const auto itrCustom = std::find_if(config.RAIIClassCustomizations.begin(), config.RAIIClassCustomizations.end(), [createMethodName](const RAIIClassCustomization& val) { return val.SourceCreateMethod == createMethodName; });

          UnrolledCreateMethod unrolledCreateMethod;
          std::unordered_set<std::string> uniqueNames;
          for (auto itrParam = itr->Result.MethodArguments.begin(); itrParam != itr->Result.MethodArguments.end(); ++itrParam)
          {
            if (itrParam->FullType.IsStruct)
            {
              UnrolledStruct unrolledStruct(*itrParam);
              auto itrStruct = structDict.find(itrParam->FullType.Name);
              if (itrStruct != structDict.end())
              {
                std::cout << "  CreateFunction: " << itr->Pair.Create.Name << " found struct: " << itrParam->FullType.Name << "\n";
                const auto & structRecord = itrStruct->second;
                for (auto itrStruct = structRecord.Members.begin(); itrStruct != structRecord.Members.end(); ++itrStruct)
                {
                  if (itrStruct->Name != "sType" && itrStruct->Name != "pNext")
                  {
                    if (itrCustom != config.RAIIClassCustomizations.end() && itrCustom->StructMemberArrayCountName == itrStruct->ArgumentName )
                    {
                      const auto argument = ToMethodArgument(*itrStruct, false);
                      switch (itr->Mode)
                      {
                      case AnalyzeMode::SingleInstance:
                        unrolledStruct.Members.push_back(UnrolledStructMember(UnrollMode::Counter1, ToMethodArgument(*itrStruct, false)));
                        break;
                      case AnalyzeMode::VectorInstance:
                        unrolledStruct.Members.push_back(UnrolledStructMember(UnrollMode::Size, ToMethodArgument(*itrStruct, false)));
                        unrolledCreateMethod.MethodArguments.push_back(argument);
                        break;
                      default:
                        throw NotSupportedException("Unsupported AnalyzeMode");
                      }
                    }
                    else
                    {
                      const auto argument = ToMethodArgument(*itrStruct, false);
                      if (uniqueNames.find(argument.ArgumentName) != uniqueNames.end())
                        throw std::runtime_error("Unique name clash");
                      uniqueNames.insert(argument.ArgumentName);
                      unrolledCreateMethod.MethodArguments.push_back(argument);
                      unrolledStruct.Members.push_back(UnrolledStructMember(UnrollMode::Unrolled, ToMethodArgument(*itrStruct, false)));
                    }
                  }
                  else
                    unrolledStruct.Members.push_back(UnrolledStructMember(UnrollMode::Skipped, ToMethodArgument(*itrStruct, false)));
                  //std::cout << "    " << itrStruct->Type.FullTypeString << " " << itrStruct->Name << "\n";
                }
              }
              unrolledCreateMethod.UnrolledStructs.push_back(unrolledStruct);
            }
            else
            {
              if (uniqueNames.find(itrParam->ArgumentName) != uniqueNames.end())
                throw std::runtime_error("Unique name clash");
              uniqueNames.insert(itrParam->ArgumentName);
              unrolledCreateMethod.MethodArguments.push_back(*itrParam);
            }
          }
          if (unrolledCreateMethod.UnrolledStructs.size() > 0)
            itr->Result.UnrolledCreateMethod = std::move(unrolledCreateMethod);
        }
      }
    }


    std::deque<FullAnalysis> Analyze(const Capture& capture, const SimpleGeneratorConfig& config, const FunctionAnalysis& functionAnalysis, std::unordered_set<std::string>& rTypesWithoutDefaultValues)
    {
      std::deque<FullAnalysis> managed;
      for (auto itr = functionAnalysis.Matched.begin(); itr != functionAnalysis.Matched.end(); ++itr)
      {
        if (itr->MatchTypeInfo == MatchType::Manual)
          std::cout << "Manual matched: " << itr->Create.Name << " with " << itr->Destroy.Name << "\n";
        else
          std::cout << "Matched: " << itr->Create.Name << " with " << itr->Destroy.Name << "\n";

        const std::string createMethodName = itr->Create.Name;
        const auto itrCustom = std::find_if(config.RAIIClassCustomizations.begin(), config.RAIIClassCustomizations.end(), [createMethodName](const RAIIClassCustomization& val) { return val.SourceCreateMethod == createMethodName; });
        if (itrCustom == config.RAIIClassCustomizations.end())
        {
          const auto result = Analyze(capture, config, *itr, CaseUtil::LowerCaseFirstCharacter(itr->Name), config.ForceNullParameter, AnalyzeMode::Normal, "", true);
          CheckDefaultValues(rTypesWithoutDefaultValues, result.AllMemberVariables, config.TypeDefaultValues);
          managed.push_back(FullAnalysis(*itr, result, AnalyzeMode::Normal, SourceTemplateType::NormalResource));
        }
        else
        {
          auto result = Analyze(capture, config, *itr, itrCustom->SingleInstanceClassName, config.ForceNullParameter, AnalyzeMode::SingleInstance, itrCustom->ParamMemberArrayCountName, true);
          CheckDefaultValues(rTypesWithoutDefaultValues, result.AllMemberVariables, config.TypeDefaultValues);
          managed.push_back(FullAnalysis(*itr, result, AnalyzeMode::SingleInstance, SourceTemplateType::NormalResource));

          result = Analyze(capture, config, *itr, itrCustom->VectorInstanceClassName, config.ForceNullParameter, AnalyzeMode::VectorInstance, "", itrCustom->VectorInstanceTemplateType == SourceTemplateType::ArrayAllocationButSingleInstanceDestroy);
          CheckDefaultValues(rTypesWithoutDefaultValues, result.AllMemberVariables, config.TypeDefaultValues);
          managed.push_back(FullAnalysis(*itr, result, AnalyzeMode::VectorInstance, itrCustom->VectorInstanceTemplateType));
        }
      }

      // 
      AbsorbFunctions(managed, config.ClassFunctionAbsorbtion);

      if( config.UnrollCreateStructs )
        AnalyzeCreateFunctionStructParameters(capture, config, managed);

      for (auto itr = managed.begin(); itr != managed.end(); ++itr)
      {
        FindObjectFunctions(config, functionAnalysis, managed, *itr);
      }

      return managed;
    }


    std::string GenerateExtraCreate(const SimpleGeneratorConfig& config, const Snippets& snippets, const std::string& snippetFunction, 
                                    const std::string& snippetVoidFunction, const FullAnalysis& fullAnalysis, const bool allowAbsorb)
    {
      if (!allowAbsorb && fullAnalysis.AbsorbedFunctions && fullAnalysis.AbsorbedFunctions->size() > 0)
        throw NotSupportedException("Absorbed functions can not contain absorbed functions");

      const std::string& activeSnippet = (fullAnalysis.Result.MethodArguments.size() > 0 ? snippetFunction : snippetVoidFunction);

      std::string createMethodParameters = GenerateParameterList(fullAnalysis.Result.MethodArguments);
      std::string createMethodParameterNames = GenerateParameterNameList(fullAnalysis.Result.MethodArguments);
      const std::string createFunctionArguments = GenerateExpandedParameterNameList(fullAnalysis.Result.CreateArguments);
      const std::string resetParamValidation = GenerateForAllMembers(snippets, fullAnalysis.Result.AdditionalMemberVariables, snippets.ResetParamValidation, config.TypeDefaultValues, true);
      std::string resetParamAsserts = GenerateAssertForAllMembers(snippets, fullAnalysis.Result.AdditionalMemberVariables, snippets.ResetParamAssertCondition, config.TypeDefaultValues, true);

      if (fullAnalysis.Mode == AnalyzeMode::SingleInstance)
      {
        std::string snippetCommand = snippets.ResetAssertCommand;
        StringUtil::Replace(snippetCommand, "##ASSERT_CONDITION##", fullAnalysis.Result.ResourceCountVariableName + " == 1");
        resetParamAsserts += END_OF_LINE + snippetCommand;
      }

      std::string content(activeSnippet);
      StringUtil::Replace(content, "##CLASS_NAME##", fullAnalysis.Result.ClassName);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETERS##", createMethodParameters);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETER_NAMES##", createMethodParameterNames);
      StringUtil::Replace(content, "##CREATE_FUNCTION_ARGUMENTS##", createFunctionArguments);
      StringUtil::Replace(content, "##CREATE_FUNCTION##", fullAnalysis.Pair.Create.Name);
      StringUtil::Replace(content, "##RESET_PARAMETER_VALIDATION##", resetParamValidation);
      StringUtil::Replace(content, "##RESET_ASSERT_VALIDATION##", resetParamAsserts);
      StringUtil::Replace(content, "##SOURCE_FUNCTION_NAME##", fullAnalysis.Pair.Create.Name);
      return content;
    }


    GeneratedMethodCode GenerateExtraCreates(const SimpleGeneratorConfig& config, const Snippets& snippets, const std::string& snippetHeader, const std::string& snippetSource, 
                                             const std::string& snippetVoidHeader, const std::string& snippetVoidSource, const FullAnalysis& fullAnalysis)
    {
      std::string resultHeader = GenerateExtraCreate(config, snippets, snippetHeader, snippetVoidHeader, fullAnalysis, true);
      std::string resultSource = GenerateExtraCreate(config, snippets, snippetSource, snippetVoidSource, fullAnalysis, true);

      if (fullAnalysis.AbsorbedFunctions && fullAnalysis.AbsorbedFunctions->size() > 0)
      {
        for (auto itr = fullAnalysis.AbsorbedFunctions->begin(); itr != fullAnalysis.AbsorbedFunctions->end(); ++itr)
        {
          resultHeader += END_OF_LINE + END_OF_LINE + GenerateExtraCreate(config, snippets, snippetHeader, snippetVoidHeader, *itr, false);
          resultSource += END_OF_LINE + END_OF_LINE + GenerateExtraCreate(config, snippets, snippetSource, snippetVoidSource, *itr, false);
        }
      }
      return GeneratedMethodCode(resultHeader, resultSource);
    }




    std::string GenerateUnrolledStructLocalVariables(const std::string& snippet, const std::string& snippetSet, const FullAnalysis& fullAnalysis)
    {
      std::string result;
      for (auto itr = fullAnalysis.Result.UnrolledCreateMethod.UnrolledStructs.begin(); itr != fullAnalysis.Result.UnrolledCreateMethod.UnrolledStructs.end(); ++itr)
      {
        std::string setMembers;
        std::string variable = itr->Source.ArgumentName + ".";
        for (auto itrMember = itr->Members.begin(); itrMember != itr->Members.end(); ++itrMember)
        {
          std::string contentM = snippetSet;
          std::string value;
          switch (itrMember->Mode)
          {
          case UnrollMode::Skipped:
          {
            if (itrMember->Argument.ArgumentName == "sType")
              value = StringHelper::GenerateVulkanStructFlagName(itr->Source.FullType.Name);
            else if (itrMember->Argument.ArgumentName == "pNext")
              value = "nullptr";
            else
              value = itrMember->Argument.ParameterValue;
            break;
          case UnrollMode::Counter1:
            value = "1";
            break;
          case UnrollMode::Size:
            value = itrMember->Argument.ParameterValue;
            break;
          }
          default:
            value = itrMember->Argument.ParameterValue;
            break;
          }

          StringUtil::Replace(contentM, "##MEMBER_NAME##", variable + itrMember->Argument.ArgumentName);
          StringUtil::Replace(contentM, "##MEMBER_ARGUMENT_NAME##", value);
          setMembers += contentM + END_OF_LINE;
        }


        std::string content = snippet;
        StringUtil::Replace(content, "##STRUCT_TYPE##", itr->Source.FullType.Name);
        StringUtil::Replace(content, "##STRUCT_NAME##", itr->Source.ArgumentName);
        StringUtil::Replace(content, "##STRUCT_SET_MEMBERS##", setMembers);
        result += END_OF_LINE + content;
      }
      return result;
    }

    
    std::string GenerateUnrolledCreate(const SimpleGeneratorConfig& config, const Snippets& snippets, const std::string& snippetFunction, const FullAnalysis& fullAnalysis, const bool allowAbsorb, const bool isConstructor)
    {
      if (!allowAbsorb && fullAnalysis.AbsorbedFunctions && fullAnalysis.AbsorbedFunctions->size() > 0)
        throw NotSupportedException("Absorbed functions can not contain absorbed functions");

      const auto& unrolledCreateMethod = fullAnalysis.Result.UnrolledCreateMethod;
      if (fullAnalysis.Result.UnrolledCreateMethod.MethodArguments.size() <= 0)
        return std::string();

      std::string localVariables = GenerateUnrolledStructLocalVariables(snippets.ResetUnrollStructVariable, snippets.ResetSetMemberVariable, fullAnalysis);
      std::string createMethodParameters = GenerateParameterList(fullAnalysis.Result.UnrolledCreateMethod.MethodArguments);
      std::string createMethodParameterNames = GenerateParameterNameList(isConstructor ? fullAnalysis.Result.UnrolledCreateMethod.MethodArguments : fullAnalysis.Result.MethodArguments);
      const std::string createFunctionArguments = GenerateExpandedParameterNameList(fullAnalysis.Result.CreateArguments);

      std::string content(snippetFunction);
      StringUtil::Replace(content, "##LOCAL_VARIABLES##", localVariables);
      StringUtil::Replace(content, "##CLASS_NAME##", fullAnalysis.Result.ClassName);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETERS##", createMethodParameters);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETER_NAMES##", createMethodParameterNames);
      StringUtil::Replace(content, "##CREATE_FUNCTION_ARGUMENTS##", createFunctionArguments);
      StringUtil::Replace(content, "##CREATE_FUNCTION##", fullAnalysis.Pair.Create.Name);
      StringUtil::Replace(content, "##SOURCE_FUNCTION_NAME##", fullAnalysis.Pair.Create.Name);
      return content;
    }

    std::string UnrolledWrap(const std::string& source, const std::string& snippet)
    {
      if (source.size() <= 0)
        return source;

      std::string content(snippet);
      StringUtil::Replace(content, "##UNROLLED_METHODS##", source);
      return content;
    }

    GeneratedMethodCode GenerateUnrolledCreates(const SimpleGeneratorConfig& config, const Snippets& snippets, const std::string& snippetHeader, const std::string& snippetSource, const FullAnalysis& fullAnalysis, const bool isConstructor)
    {
      std::string resultHeader = GenerateUnrolledCreate(config, snippets, snippetHeader, fullAnalysis, true, isConstructor);
      std::string resultSource = GenerateUnrolledCreate(config, snippets, snippetSource, fullAnalysis, true, isConstructor);

      if (fullAnalysis.AbsorbedFunctions && fullAnalysis.AbsorbedFunctions->size() > 0)
      {
        for (auto itr = fullAnalysis.AbsorbedFunctions->begin(); itr != fullAnalysis.AbsorbedFunctions->end(); ++itr)
        {
          const auto tmpHeader = GenerateUnrolledCreate(config, snippets, snippetHeader, *itr, false, isConstructor);
          if(tmpHeader.size() > 0)
            resultHeader += END_OF_LINE + END_OF_LINE + tmpHeader;
          const auto tmpSource = GenerateUnrolledCreate(config, snippets, snippetSource, *itr, false, isConstructor);
          if (tmpSource.size() > 0)
            resultSource += END_OF_LINE + END_OF_LINE + tmpSource;
        }
      }

      resultHeader = UnrolledWrap(resultHeader, snippets.UnrolledWrap);
      resultSource = UnrolledWrap(resultSource, snippets.UnrolledWrap);

      return GeneratedMethodCode(resultHeader, resultSource);
    }


    std::string GenerateMemberAsParameters(const std::deque<MemberVariable>& allMemberVariables, const bool addClaimMode)
    {
      std::string content(addClaimMode ? "const ClaimMode claimMode, " : "");

      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        content += std::string("const ") + itr->Type + " " + itr->ArgumentName;
        if ((itr + 1) != allMemberVariables.end())
          content += ", ";
      }
      return content;
    }

    std::string GenerateMemberAsNames(const std::deque<MemberVariable>& allMemberVariables, const bool addClaimMode)
    {
      std::string content(addClaimMode ? "claimMode, " : "");
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        content += itr->ArgumentName;
        if ((itr + 1) != allMemberVariables.end())
          content += ", ";
      }
      return content;
    }

    bool IsResetModeRequired(const FullAnalysis& fullAnalysis)
    {
      if (fullAnalysis.AbsorbedFunctions)
      {
        for (auto itr = fullAnalysis.AbsorbedFunctions->begin(); itr != fullAnalysis.AbsorbedFunctions->end(); ++itr)
        {
          if (IsResetModeRequired(*itr))
            return true;
        }
      }

      if (fullAnalysis.Result.MethodArguments.size() <= 0)
        return true;
      return false;
    }

    bool IsResetModeRequired(const std::deque<FullAnalysis>& fullAnalysis)
    {
      for (auto itr = fullAnalysis.begin(); itr != fullAnalysis.end(); ++itr)
      {
        if (IsResetModeRequired(*itr))
          return true;
      }
      return false;
    }


    std::string GenerateAdditionalIncludes(const SimpleGeneratorConfig& config, const Snippets& snippets, const FullAnalysis& fullAnalysis, const std::string& additionalHeaders)
    {
      std::string result;

      if (IsResetModeRequired(fullAnalysis))
        result += END_OF_LINE + snippets.IncludeResetMode;

      if (additionalHeaders.size() > 0)
        result += END_OF_LINE + additionalHeaders;
      return result;
    }


    std::string AddContentGuards(const std::string& content, const std::deque<std::string>& guardFunctions)
    {
      if (guardFunctions.size() <= 0)
        return content;

      std::string newContent;
      for (const auto& guardFunction : guardFunctions)
        newContent += END_OF_LINE + "#if " + guardFunction;

      newContent += END_OF_LINE + content;

      for (const auto& guardFunction : guardFunctions)
        newContent += END_OF_LINE + "#endif";
      return newContent;
    }

    std::string GenerateAdditionalMethods(const SimpleGeneratorConfig& config, const FullAnalysis& fullAnalysis, const SnippetMethodContext& snippets)
    {
      std::string res;
      for (auto& method : fullAnalysis.ClassMethods)
      {
        const std::string methodParameters = GenerateParameterList(method.MethodArguments);
        const std::string functionArguments = GenerateExpandedParameterNameList(method.CombinedMethodArguments);

        std::string methodCode = method.SourceFunction.Name;
        std::string methodReturnType;

        std::string content;
        switch (method.Template)
        {
        case ClassMethod::TemplateType::Type:
          content = snippets.TypeReturn;
          methodReturnType = method.ReturnType.FullTypeString;
          break;
        case ClassMethod::TemplateType::Error:
          content = snippets.ErrorReturn;
          break;
        case ClassMethod::TemplateType::Void:
          content = snippets.VoidReturn;
          break;
        default:
          throw NotSupportedException("Unknown template type");
        }

        StringUtil::Replace(content, "##SOURCE_FUNCTION_NAME##", method.SourceFunction.Name);
        StringUtil::Replace(content, "##METHOD_RETURN_TYPE##", methodReturnType);
        StringUtil::Replace(content, "##METHOD_NAME##", method.Name);
        StringUtil::Replace(content, "##METHOD_PARAMETERS##", methodParameters);
        StringUtil::Replace(content, "##FUNCTION_ARGUMENTS##", functionArguments);

        content = AddContentGuards(content, method.GuardFunctions);

        res += END_OF_LINE + END_OF_LINE + content;
      }
      return res;
    }


    std::string GenerateContent(const SimpleGeneratorConfig& config, const FullAnalysis& fullAnalysis, const std::string& contentTemplate, const std::string*const pSnippetMemberVariable,
                                const std::string*const pSnippetMemberVariableGet, const Snippets& snippets, const AdditionalContent& additionalContent)
    {
      // Generate the RAII resource class header file
      std::string content = contentTemplate;

      std::string classAdditionalMemberVariables;
      std::string classAdditionalGetMemberVariablesMethods;
      {
        for (auto itr = fullAnalysis.Result.AdditionalMemberVariables.begin(); itr != fullAnalysis.Result.AdditionalMemberVariables.end(); ++itr)
        {
          if (pSnippetMemberVariable != nullptr)
          {
            std::string memberVariable = *pSnippetMemberVariable;
            StringUtil::Replace(memberVariable, "##MEMBER_TYPE##", itr->Type);
            StringUtil::Replace(memberVariable, "##MEMBER_NAME##", itr->Name);
            classAdditionalMemberVariables += END_OF_LINE + memberVariable;
          }

          if (pSnippetMemberVariableGet != nullptr)
          {
            std::string memberVariableGet = *pSnippetMemberVariableGet;
            StringUtil::Replace(memberVariableGet, "##MEMBER_TYPE##", itr->Type);
            StringUtil::Replace(memberVariableGet, "##MEMBER_NAME##", itr->Name);
            StringUtil::Replace(memberVariableGet, "##MEMBER_NICE_NAME##", itr->NiceNameUpperCamelCase);
            classAdditionalGetMemberVariablesMethods += END_OF_LINE + memberVariableGet;
          }
        }
      }


      std::string resetMemberHeader;
      std::string resetMemberSource;
      switch (fullAnalysis.Mode)
      {
      case AnalyzeMode::VectorInstance:
        resetMemberHeader = snippets.ResetMemberHeaderVector;
        resetMemberSource = snippets.ResetMemberSourceVector;
        break;
      default:
        resetMemberHeader = snippets.ResetMemberHeader;
        resetMemberSource = snippets.ResetMemberSource;
        break;
      }

      std::string additionalMethodsHeader = GenerateAdditionalMethods(config, fullAnalysis, snippets.AdditionalMethodHeader);
      const std::string additionalMethodsSource = GenerateAdditionalMethods(config, fullAnalysis, snippets.AdditionalMethodSource);

      if (additionalContent.Methods.size() > 0)
      {
        additionalMethodsHeader += END_OF_LINE + END_OF_LINE + END_OF_LINE + additionalContent.Methods;
      }

      const auto resourceAsArgument = ToMethodArgument(fullAnalysis.Result.ResourceMemberVariable);

      const std::string memberParameters = GenerateMemberAsParameters(fullAnalysis.Result.AllMemberVariables, config.OwnershipTransferUseClaimMode);
      const std::string memberParameterNames = GenerateMemberAsNames(fullAnalysis.Result.AllMemberVariables, config.OwnershipTransferUseClaimMode);
      const std::string resetSetMembers = GenerateForAllMembersMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetSetMemberVariable, snippets.ResetSetMemberVariableMove, config.TypeDefaultValues, true);
      const std::string resetSetMembersNormal = GenerateForAllMembersMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetSetMemberVariable, snippets.ResetSetMemberVariableMove, config.TypeDefaultValues);
      const std::string defaultConstructorInitialization = GenerateConstructorInitialization(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ConstructorMemberInitialization, config.TypeDefaultValues);
      const std::string moveConstructorMemberInitialization = GenerateConstructorInitializationMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveConstructorClaimMember, snippets.MoveConstructorClaimMemberMove, config.TypeDefaultValues);
      const std::string moveAssignmentClaimMembers = GenerateForAllMembersMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveAssignmentClaimMember, snippets.MoveAssignmentClaimMemberMove, config.TypeDefaultValues);
      const std::string moveAssignmentInvalidateMembers = GenerateForAllMembersMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveAssignmentInvalidateMember, "", config.TypeDefaultValues);
      const std::string moveConstructorInvalidateMembers = GenerateForAllMembersMoveSupport(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveConstructorInvalidateMember, "", config.TypeDefaultValues);
      const std::string resetInvalidateMembers = GenerateForAllMembersInvalidate(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetInvalidateMemberVariable, config.TypeDefaultValues);
      const std::string resetMemberAssertions = GenerateAssertForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetMemberAssertCondition, config.TypeDefaultValues);

      const std::string destroyFunctionArguments = GenerateExpandedParameterNameList(fullAnalysis.Result.DestroyArguments);

      auto classExtraConstructors = GenerateExtraCreates(config, snippets, snippets.CreateConstructorHeader, snippets.CreateConstructorSource, snippets.CreateVoidConstructorHeader, snippets.CreateVoidConstructorSource, fullAnalysis);
      auto classExtraResetMethods = GenerateExtraCreates(config, snippets, resetMemberHeader, resetMemberSource, snippets.ResetVoidMemberHeader, snippets.ResetVoidMemberSource, fullAnalysis);
      auto classUnrolledConstructors = GenerateUnrolledCreates(config, snippets, snippets.CreateConstructorHeader, snippets.CreateConstructorSource, fullAnalysis, true);
      auto classUnrolledResetMethods = GenerateUnrolledCreates(config, snippets, snippets.ResetUnrollMemberHeader, snippets.ResetUnrollMemberSource, fullAnalysis, false);

      if (classUnrolledConstructors.Header.size() > 0)
      {
        classExtraConstructors += GeneratedMethodCode(END_OF_LINE + END_OF_LINE);
        classExtraConstructors += classUnrolledConstructors;
      }
      if (classUnrolledResetMethods.Header.size() > 0)
      {
        classExtraResetMethods += GeneratedMethodCode(END_OF_LINE + END_OF_LINE);
        classExtraResetMethods += classUnrolledResetMethods;
      }

      const std::string additionalIncludes = GenerateAdditionalIncludes(config, snippets, fullAnalysis, additionalContent.Headers);

      StringUtil::Replace(content, "##ADDITIONAL_INCLUDES##", additionalIncludes);
      StringUtil::Replace(content, "##CLASS_EXTRA_CONSTRUCTORS_HEADER##", classExtraConstructors.Header);
      StringUtil::Replace(content, "##CLASS_EXTRA_CONSTRUCTORS_SOURCE##", classExtraConstructors.Source);
      StringUtil::Replace(content, "##CLASS_EXTRA_RESET_METHODS_HEADER##", classExtraResetMethods.Header);
      StringUtil::Replace(content, "##CLASS_EXTRA_RESET_METHODS_SOURCE##", classExtraResetMethods.Source);

      StringUtil::Replace(content, "##CLASS_NAME##", fullAnalysis.Result.ClassName);
      StringUtil::Replace(content, "##CLASS_NAME!##", CaseUtil::UpperCase(fullAnalysis.Result.ClassName));
      StringUtil::Replace(content, "##RESOURCE_TYPE##", fullAnalysis.Result.ResourceMemberVariable.Type);
      StringUtil::Replace(content, "##RESOURCE_TYPE_AS_PARAM##", resourceAsArgument.FullTypeString);
      StringUtil::Replace(content, "##RESOURCE_PARAM_NAME##", resourceAsArgument.ArgumentName);

      StringUtil::Replace(content, "##RESOURCE_MEMBER_NAME##", fullAnalysis.Result.ResourceMemberVariable.Name);
      StringUtil::Replace(content, "##CLASS_ADDITIONAL_MEMBER_VARIABLES##", classAdditionalMemberVariables);
      StringUtil::Replace(content, "##CLASS_ADDITIONAL_GET_MEMBER_VARIABLE_METHODS##", classAdditionalGetMemberVariablesMethods);
      ReplaceDefaultValue(content, fullAnalysis.Result.ResourceMemberVariable.Type, config.TypeDefaultValues, snippets);

      StringUtil::Replace(content, "##DESTROY_FUNCTION##", fullAnalysis.Pair.Destroy.Name);
      StringUtil::Replace(content, "##RESET_SET_MEMBERS##", resetSetMembers);
      StringUtil::Replace(content, "##RESET_SET_MEMBERS_NORMAL##", resetSetMembersNormal);
      StringUtil::Replace(content, "##RESOURCE_INTERMEDIARY_NAME##", fullAnalysis.Result.IntermediaryName);
      StringUtil::Replace(content, "##RESET_MEMBER_ASSERTIONS##", resetMemberAssertions);
      StringUtil::Replace(content, "##RESET_INVALIDATE_MEMBERS##", resetInvalidateMembers);
      StringUtil::Replace(content, "##MOVE_ASSIGNMENT_CLAIM_MEMBERS##", moveAssignmentClaimMembers);
      StringUtil::Replace(content, "##MOVE_ASSIGNMENT_INVALIDATE_MEMBERS##", moveAssignmentInvalidateMembers);
      StringUtil::Replace(content, "##MOVE_CONSTRUCTOR_MEMBER_INITIALIZATION##", moveConstructorMemberInitialization);
      StringUtil::Replace(content, "##MOVE_CONSTRUCTOR_INVALIDATE_MEMBERS##", moveConstructorInvalidateMembers);
      StringUtil::Replace(content, "##DEFAULT_CONSTRUCTOR_MEMBER_INITIALIZATION##", defaultConstructorInitialization);
      StringUtil::Replace(content, "##DESTROY_FUNCTION_ARGUMENTS##", destroyFunctionArguments);
      StringUtil::Replace(content, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      StringUtil::Replace(content, "##MEMBER_PARAMETERS##", memberParameters);
      StringUtil::Replace(content, "##MEMBER_PARAMETER_NAMES##", memberParameterNames);
      StringUtil::Replace(content, "##NAMESPACE_NAME##", config.NamespaceName);
      StringUtil::Replace(content, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
      StringUtil::Replace(content, "##HANDLE_CLASS_NAME##", snippets.HandleClassName);
      StringUtil::Replace(content, "##RESOURCE_COUNT##", fullAnalysis.Result.ResourceCountVariableName);

      StringUtil::Replace(content, "##ADDITIONAL_METHODS_HEADER##", additionalMethodsHeader);
      StringUtil::Replace(content, "##ADDITIONAL_METHODS_SOURCE##", additionalMethodsSource);

      const auto version = fullAnalysis.Pair.Create.Version;
      std::string strVersinGuardBegin;
      std::string strVersinGuardEnd;
      if (config.VersionGuard.IsValid && version != VersionRecord())
      {
        strVersinGuardBegin = fmt::format("\n#if {0}", config.VersionGuard.ToGuardString(version));
        strVersinGuardEnd = "\n#endif";
      }
      StringUtil::Replace(content, "##VERSION_GUARD_BEGIN##", strVersinGuardBegin);
      StringUtil::Replace(content, "##VERSION_GUARD_END##", strVersinGuardEnd);

      return content;
    }


    AddtionalContentMap LoadAdditional(const IO::Path& templateRoot, const std::string& dirName)
    {
      const auto pathDir = IO::Path::Combine(templateRoot, dirName);
      if (!IO::Directory::Exists(pathDir))
        return AddtionalContentMap();

      IO::PathDeque files;
      IO::Directory::GetFiles(files, pathDir, IO::SearchOptions::TopDirectoryOnly);
      if( files.size() <= 0 )
        return AddtionalContentMap();

      AddtionalContentMap map;

      for (auto& entry : files)
      {
        const auto content = IO::File::ReadAllText(*entry);
        const auto filename = IO::Path::GetFileName(*entry);
        map[filename.ToUTF8String()] = content;
      }
      return map;
    }

    void AddDummyEntries(AddtionalFileContentMap& rDst, const AddtionalContentMap& srcMap)
    {
      for (auto& entry : srcMap)
      {
        auto itrFind = rDst.find(entry.first);
        if (itrFind == rDst.end())
          rDst[entry.first] = AdditionalContent();
      }
    }

    AddtionalFileContentMap LoadAdditionalContent(const IO::Path& templateRoot)
    {
      auto methods = LoadAdditional(templateRoot, "AddMethods");
      auto headers = LoadAdditional(templateRoot, "AddHeaders");

      AddtionalFileContentMap content;
      AddDummyEntries(content, methods);
      AddDummyEntries(content, headers);

      for (auto& entry : methods)
        content[entry.first].Methods = entry.second;
      for (auto& entry : headers)
        content[entry.first].Headers = entry.second;
      return content;
    }


    Snippets LoadSnippets(const IO::Path& templateRoot)
    {
      const auto pathHeaderSnippetMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_MemberVariable.txt");
      const auto pathHeaderSnippetMemberVariableGet = IO::Path::Combine(templateRoot, "TemplateSnippet_MemberVariableGet.txt");
      const auto pathSnippetResetAssertCommand = IO::Path::Combine(templateRoot, "TemplateSnippet_AssertCommand.txt");
      const auto pathSnippetConstructorMemberInitialization = IO::Path::Combine(templateRoot, "TemplateSnippet_ConstructorMemberInitialization.txt");
      const auto pathSnippetConstructorMemberInitializationPOD = IO::Path::Combine(templateRoot, "TemplateSnippet_ConstructorMemberInitializationPOD.txt");
      const auto pathSnippetAdditionalErrorMethodHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalErrorMethodHeader.txt");
      const auto pathSnippetAdditionalErrorMethodSource = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalErrorMethodSource.txt");
      const auto pathSnippetAdditionalTypeMethodHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalTypeMethodHeader.txt");
      const auto pathSnippetAdditionalTypeMethodSource = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalTypeMethodSource.txt");
      const auto pathSnippetAdditionalVoidMethodHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalVoidMethodHeader.txt");
      const auto pathSnippetAdditionalVoidMethodSource = IO::Path::Combine(templateRoot, "TemplateSnippet_AdditionalVoidMethodSource.txt");
      const auto pathSnippetCreateConstructorHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateConstructorHeader.txt");
      const auto pathSnippetCreateConstructorSource = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateConstructorSource.txt");
      const auto pathSnippetResetSetMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetSetMemberVariable.txt");
      const auto pathSnippetResetSetMemberVariableMove = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetSetMemberVariableMove.txt");
      const auto pathSnippetResetInvalidateMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetInvalidateMemberVariable.txt");
      const auto pathSnippetResetInvalidateMemberVariablePOD = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetInvalidateMemberVariablePOD.txt");
      const auto pathSnippetResetMemberAssertConditionPOD = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberAssertConditionPOD.txt");
      const auto pathSnippetResetMemberAssertConditionVector = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberAssertConditionVector.txt");
      const auto pathSnippetResetMemberHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberHeader.txt");
      const auto pathSnippetResetMemberSource = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberSource.txt");
      const auto pathSnippetResetMemberHeaderVector = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberHeaderVector.txt");
      const auto pathSnippetResetMemberSourceVector = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberSourceVector.txt");
      const auto pathSnippetResetParamValidation = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetParamValidation.txt");
      const auto pathSnippetResetParamAssertConditionComplex = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetParamAssertConditionVector.txt");
      const auto pathSnippetResetParamAssertConditionPOD = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetParamAssertConditionPOD.txt");
      const auto pathSnippetMoveAssignmentClaimMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveAssignmentClaimMember.txt");
      const auto pathSnippetMoveAssignmentClaimMemberMove = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveAssignmentClaimMemberMove.txt");
      const auto pathSnippetMoveAssignmentInvalidateMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveAssignmentInvalidateMember.txt");
      const auto pathSnippetMoveConstructorInvalidateMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveConstructorInvalidateMember.txt");
      const auto pathSnippetMoveConstructorClaimMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveConstructorClaimMember.txt");
      const auto pathSnippetMoveConstructorClaimMemberMove = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveConstructorClaimMemberMove.txt");
      const auto pathSnippetHandleClassName = IO::Path::Combine(templateRoot, "TemplateSnippet_HandleClassName.txt");
      const auto pathSnippetCreateVoidConstructorHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateVoidConstructorHeader.txt");
      const auto pathSnippetCreateVoidConstructorSource = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateVoidConstructorSource.txt");
      const auto pathSnippetResetVoidMemberHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetVoidMemberHeader.txt");
      const auto pathSnippetResetVoidMemberSource = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetVoidMemberSource.txt");
      const auto pathSnippetResetUnrollMemberHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetUnrollMemberHeader.txt");
      const auto pathSnippetResetUnrollMemberSource = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetUnrollMemberSource.txt");
      const auto pathSnippetResetUnrollStructVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetUnrollStructVariable.txt");
      const auto pathSnippetDefaultValueMod = IO::Path::Combine(templateRoot, "TemplateSnippet_DefaultValueMod.txt");
      const auto pathSnippetIncludeResetMode = IO::Path::Combine(templateRoot, "TemplateSnippet_IncludeResetMode.txt");
      const auto pathSnippetUnrolledWrap = IO::Path::Combine(templateRoot, "TemplateSnippet_UnrolledWrap.txt");

      Snippets snippets(templateRoot);
      snippets.ConstructorMemberInitialization.Complex = IO::File::ReadAllText(pathSnippetConstructorMemberInitialization);
      snippets.ConstructorMemberInitialization.POD = IO::File::ReadAllText(pathSnippetConstructorMemberInitializationPOD);

      SnippetMethodContext additionalMethodHeader;
      additionalMethodHeader.ErrorReturn = IO::File::ReadAllText(pathSnippetAdditionalErrorMethodHeader);
      additionalMethodHeader.TypeReturn = IO::File::ReadAllText(pathSnippetAdditionalTypeMethodHeader);
      additionalMethodHeader.VoidReturn = IO::File::ReadAllText(pathSnippetAdditionalVoidMethodHeader);
      SnippetMethodContext additionalMethodSource;
      additionalMethodSource.ErrorReturn = IO::File::ReadAllText(pathSnippetAdditionalErrorMethodSource);
      additionalMethodSource.TypeReturn = IO::File::ReadAllText(pathSnippetAdditionalTypeMethodSource);
      additionalMethodSource.VoidReturn = IO::File::ReadAllText(pathSnippetAdditionalVoidMethodSource);
      snippets.AdditionalMethodHeader = additionalMethodHeader;
      snippets.AdditionalMethodSource = additionalMethodSource;
      snippets.ResetAssertCommand = IO::File::ReadAllText(pathSnippetResetAssertCommand);
      snippets.CreateConstructorHeader = IO::File::ReadAllText(pathSnippetCreateConstructorHeader);
      snippets.CreateConstructorSource = IO::File::ReadAllText(pathSnippetCreateConstructorSource);
      snippets.ResetSetMemberVariable = IO::File::ReadAllText(pathSnippetResetSetMemberVariable);
      snippets.ResetSetMemberVariableMove = IO::File::ReadAllText(pathSnippetResetSetMemberVariableMove);
      snippets.ResetInvalidateMemberVariable.Complex = IO::File::ReadAllText(pathSnippetResetInvalidateMemberVariable);
      snippets.ResetInvalidateMemberVariable.POD = IO::File::ReadAllText(pathSnippetResetInvalidateMemberVariablePOD);
      snippets.ResetMemberAssertCondition.Complex = IO::File::ReadAllText(pathSnippetResetMemberAssertConditionVector);
      snippets.ResetMemberAssertCondition.POD = IO::File::ReadAllText(pathSnippetResetMemberAssertConditionPOD);
      snippets.ResetMemberHeader = IO::File::ReadAllText(pathSnippetResetMemberHeader);
      snippets.ResetMemberSource = IO::File::ReadAllText(pathSnippetResetMemberSource);
      snippets.ResetMemberHeaderVector = IO::File::ReadAllText(pathSnippetResetMemberHeaderVector);
      snippets.ResetMemberSourceVector = IO::File::ReadAllText(pathSnippetResetMemberSourceVector);
      snippets.ResetParamValidation = IO::File::ReadAllText(pathSnippetResetParamValidation);
      snippets.ResetParamAssertCondition.Complex = IO::File::ReadAllText(pathSnippetResetParamAssertConditionComplex);
      snippets.ResetParamAssertCondition.POD = IO::File::ReadAllText(pathSnippetResetParamAssertConditionPOD);
      snippets.MoveAssignmentClaimMember = IO::File::ReadAllText(pathSnippetMoveAssignmentClaimMember);
      snippets.MoveAssignmentClaimMemberMove = IO::File::ReadAllText(pathSnippetMoveAssignmentClaimMemberMove);
      snippets.MoveAssignmentInvalidateMember = IO::File::ReadAllText(pathSnippetMoveAssignmentInvalidateMember);
      snippets.MoveConstructorInvalidateMember = IO::File::ReadAllText(pathSnippetMoveConstructorInvalidateMember);
      snippets.MoveConstructorClaimMember = IO::File::ReadAllText(pathSnippetMoveConstructorClaimMember);
      snippets.MoveConstructorClaimMemberMove = IO::File::ReadAllText(pathSnippetMoveConstructorClaimMemberMove);
      snippets.HandleClassName = IO::File::ReadAllText(pathSnippetHandleClassName);
      snippets.CreateVoidConstructorHeader = IO::File::ReadAllText(pathSnippetCreateVoidConstructorHeader);
      snippets.CreateVoidConstructorSource = IO::File::ReadAllText(pathSnippetCreateVoidConstructorSource);
      snippets.ResetVoidMemberHeader = IO::File::ReadAllText(pathSnippetResetVoidMemberHeader);
      snippets.ResetVoidMemberSource = IO::File::ReadAllText(pathSnippetResetVoidMemberSource);
      snippets.ResetUnrollMemberHeader = IO::File::ReadAllText(pathSnippetResetUnrollMemberHeader);
      snippets.ResetUnrollMemberSource = IO::File::ReadAllText(pathSnippetResetUnrollMemberSource);
      snippets.ResetUnrollStructVariable = IO::File::ReadAllText(pathSnippetResetUnrollStructVariable);
      snippets.DefaultValueMod = IO::File::ReadAllText(pathSnippetDefaultValueMod);
      snippets.IncludeResetMode = IO::File::ReadAllText(pathSnippetIncludeResetMode);
      snippets.UnrolledWrap = IO::File::ReadAllText(pathSnippetUnrolledWrap);
      snippets.HeaderSnippetMemberVariable = IO::File::ReadAllText(pathHeaderSnippetMemberVariable);
      snippets.HeaderSnippetMemberVariableGet = IO::File::ReadAllText(pathHeaderSnippetMemberVariableGet);
      return snippets;
    }


    std::string ResolveMethodOverride(const IO::Path& snippetRoot, const std::string& overrideValue, const std::string& defaultValue)
    {
      if(overrideValue.size() <= 0)
        return defaultValue;

      const auto fullPath = IO::Path::Combine(snippetRoot, overrideValue);
      return IO::File::ReadAllText(fullPath);
    }


    RAIIClassMethodOverrides ResolveOverrides(const RAIIClassMethodOverrides& methodOverrides, const Snippets& snippets)
    {
      RAIIClassMethodOverrides result;
      result.ResetHeader = ResolveMethodOverride(snippets.SnippetRoot, methodOverrides.ResetHeader, snippets.ResetMemberHeader);
      result.ResetSource = ResolveMethodOverride(snippets.SnippetRoot, methodOverrides.ResetSource, snippets.ResetMemberSource);
      return result;
    }


    std::unordered_map<std::string, RAIIClassMethodOverrides> ResolveOverrides(const std::unordered_map<std::string, RAIIClassMethodOverrides>& classMethodOverrides, const Snippets& snippets)
    {
      std::unordered_map<std::string, RAIIClassMethodOverrides> map;
      for (auto& value : classMethodOverrides)
      {
        map[value.first] = ResolveOverrides(value.second, snippets);
      }
      return map;
    }


    AdditionalContent GetAdditionalContent(const AddtionalFileContentMap& additionalFileContent, const IO::Path& fileName)
    {
      const auto itrFind = additionalFileContent.find(IO::Path::GetFileName(fileName).ToUTF8String());
      return itrFind != additionalFileContent.end() ? itrFind->second : AdditionalContent();
    }
  }


  // FIX: 
  // - Handle 'create array' of resources, example: CommandBuffers
  // - Add helper methods 

  SimpleGenerator::SimpleGenerator(const Capture& capture, const SimpleGeneratorConfig& config, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : Generator(capture, config)
  {
    const auto additionalFileContent = LoadAdditionalContent(templateRoot);

    const Snippets snippets = LoadSnippets(templateRoot);
    const auto resolvedClassMethodOverrides = ResolveOverrides(config.ClassMethodOverrides, snippets);

    const auto pathHeader0 = IO::Path::Combine(templateRoot, "Template_header0.hpp");
    const auto pathSource0 = IO::Path::Combine(templateRoot, "Template_source0.cpp");
    const auto pathHeader1Vector = IO::Path::Combine(templateRoot, "Template_header1Vector.hpp");
    const auto pathSource1Vector = IO::Path::Combine(templateRoot, "Template_source1Vector.cpp");
    const auto pathHeader2Vector = IO::Path::Combine(templateRoot, "Template_header2Vector.hpp");
    const auto pathSource2Vector = IO::Path::Combine(templateRoot, "Template_source2Vector.cpp");
    const auto headerTemplate0 = IO::File::ReadAllText(pathHeader0);
    const auto sourceTemplate0 = IO::File::ReadAllText(pathSource0);
    const auto headerTemplate1Vector = IO::File::ReadAllText(pathHeader1Vector);
    const auto sourceTemplate1Vector = IO::File::ReadAllText(pathSource1Vector);
    const auto headerTemplate2Vector = IO::File::ReadAllText(pathHeader2Vector);
    const auto sourceTemplate2Vector = IO::File::ReadAllText(pathSource2Vector);

    std::unordered_set<std::string> typesWithoutDefaultValues;

    std::deque<std::string> headerTemplates;
    std::deque<std::string> sourceTemplates;
    headerTemplates.push_back(headerTemplate0);
    headerTemplates.push_back(headerTemplate1Vector);
    headerTemplates.push_back(headerTemplate2Vector);
    sourceTemplates.push_back(sourceTemplate0);
    sourceTemplates.push_back(sourceTemplate1Vector);
    sourceTemplates.push_back(sourceTemplate2Vector);

    auto fullAnalysis = Analyze(capture, config, m_functionAnalysis, typesWithoutDefaultValues);


    for (auto itr = fullAnalysis.begin(); itr != fullAnalysis.end(); ++itr)
    {
      Snippets classSnippets(snippets);
      // check if there is any class method overrids and apply the to the snippets if they exist
      const auto itrFindMethodOverride = resolvedClassMethodOverrides.find(itr->Result.ClassName);
      if (itrFindMethodOverride != resolvedClassMethodOverrides.end())
      {
        classSnippets.ResetMemberHeader = itrFindMethodOverride->second.ResetHeader;
        classSnippets.ResetMemberSource = itrFindMethodOverride->second.ResetSource;
      }

      assert(static_cast<std::size_t>(itr->TemplateType) < headerTemplates.size());
      const auto activeHeaderTemplate = headerTemplates[static_cast<std::size_t>(itr->TemplateType)];
      {
        auto fileName = IO::Path::Combine(dstPath, itr->Result.ClassName + ".hpp");
        
        const auto additionalContent = GetAdditionalContent(additionalFileContent, fileName);
        auto headerContent = GenerateContent(config, *itr, activeHeaderTemplate, &classSnippets.HeaderSnippetMemberVariable, &classSnippets.HeaderSnippetMemberVariableGet, classSnippets, additionalContent);
        IOUtil::WriteAllTextIfChanged(fileName, headerContent);
      }
      assert(static_cast<std::size_t>(itr->TemplateType) < sourceTemplates.size());
      const auto activeSourceTemplate = sourceTemplates[static_cast<std::size_t>(itr->TemplateType)];
      if (activeSourceTemplate.size() > 0)
      {
        auto fileName = IO::Path::Combine(dstPath, itr->Result.ClassName + ".cpp");
        const auto additionalContent = GetAdditionalContent(additionalFileContent, fileName);
        auto sourceContent = GenerateContent(config, *itr, activeSourceTemplate, nullptr, nullptr, classSnippets, additionalContent);
        IOUtil::WriteAllTextIfChanged(fileName, sourceContent);
      }
    }

    const auto copyRoot = IO::Path::Combine(templateRoot, "copy");
    IO::PathDeque files;
    if (IO::Directory::TryGetFiles(files, copyRoot, IO::SearchOptions::AllDirectories))
    {
      if (files.size() > 0)
      {
        for (auto itr = files.begin(); itr != files.end(); ++itr)
        {
          auto content = IO::File::ReadAllText(**itr);

          StringUtil::Replace(content, "##API_NAME##", config.APIName);
          StringUtil::Replace(content, "##API_VERSION##", config.APIVersion);
          StringUtil::Replace(content, "##PROGRAM_NAME##", config.Program.Name);
          StringUtil::Replace(content, "##PROGRAM_VERSION##", config.Program.Version);
          StringUtil::Replace(content, "##NAMESPACE_NAME##", config.NamespaceName);
          StringUtil::Replace(content, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
          StringUtil::Replace(content, "##AG_TOOL_STATEMENT##", config.ToolStatement);

          auto relativeName = (*itr)->ToUTF8String().erase(0, copyRoot.GetByteSize() + 1);
          auto dstDirectory = IO::Path::Combine(dstPath, IO::Path::GetDirectoryName(relativeName));
          auto dstFileName = IO::Path::Combine(dstDirectory, IO::Path::GetFileName(**itr));
          IO::Directory::CreateDirectory(dstDirectory);
          IOUtil::WriteAllTextIfChanged(dstFileName, content);
        }
      }
    }

    const auto pathResetModeHeader = IO::Path::Combine(templateRoot, "TemplateResetMode_header.hpp");
    const auto resetModeHeaderTemplate = IO::File::ReadAllText(pathResetModeHeader);
    if (resetModeHeaderTemplate.size() > 0 && IsResetModeRequired(fullAnalysis))
    {
      std::string content(resetModeHeaderTemplate);
      StringUtil::Replace(content, "##API_NAME##", config.APIName);
      StringUtil::Replace(content, "##API_VERSION##", config.APIVersion);
      StringUtil::Replace(content, "##PROGRAM_NAME##", config.Program.Name);
      StringUtil::Replace(content, "##PROGRAM_VERSION##", config.Program.Version);
      StringUtil::Replace(content, "##NAMESPACE_NAME##", config.NamespaceName);
      StringUtil::Replace(content, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
      StringUtil::Replace(content, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      auto dstFileName = IO::Path::Combine(dstPath, "ResetMode.hpp");
      IOUtil::WriteAllTextIfChanged(dstFileName, content);
    }

    // Write 'Readme.txt'
    {
      std::string content("Auto-generated ##API_NAME## ##API_VERSION## C++11 RAII classes by ##PROGRAM_NAME## ##PROGRAM_VERSION## (https://github.com/Unarmed1000/RAIIGen)" + END_OF_LINE);

      StringUtil::Replace(content, "##API_NAME##", config.APIName);
      StringUtil::Replace(content, "##API_VERSION##", config.APIVersion);
      StringUtil::Replace(content, "##PROGRAM_NAME##", config.Program.Name);
      StringUtil::Replace(content, "##PROGRAM_VERSION##", config.Program.Version);
      StringUtil::Replace(content, "##NAMESPACE_NAME##", config.NamespaceName);
      StringUtil::Replace(content, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
      StringUtil::Replace(content, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      auto dstFileName = IO::Path::Combine(dstPath, "RAIIGenVersion.txt");
      IOUtil::WriteAllTextIfChanged(dstFileName, content);
    }


    for (auto itr = m_functionAnalysis.MissingDestroy.begin(); itr != m_functionAnalysis.MissingDestroy.end(); ++itr)
      std::cout << "WARNING: No match found for: " << *itr << "\n";

    std::vector<std::string> sortedTypesWithoutDefaultValues(typesWithoutDefaultValues.begin(), typesWithoutDefaultValues.end());
    std::sort(sortedTypesWithoutDefaultValues.begin(), sortedTypesWithoutDefaultValues.end());
    for (auto itr = sortedTypesWithoutDefaultValues.begin(); itr != sortedTypesWithoutDefaultValues.end(); ++itr)
      std::cout << "WARNING: Missing default value for type: " << *itr << "\n";

    if (config.IsVulkan)
    {
      auto dstFileNameStructTypes = IO::Path::Combine(dstPath, "Vk/Types.hpp");
      CStructToCpp test(capture, config.ToolStatement, config.NamespaceName, templateRoot, dstFileNameStructTypes);

      auto dstFileNameDebugStrings = IO::Path::Combine(dstPath, "Debug/Strings");
      EnumToStringLookup::Process(capture, config, templateRoot, dstPath, dstFileNameDebugStrings, true);

      //auto dstFileNameFormat = IO::Path::Combine(dstPath, "Vk/Formats.hpp");
      //FormatToCpp test2(capture, config.NamespaceName, templateRoot, dstFileNameFormat);
    }
  }
}
