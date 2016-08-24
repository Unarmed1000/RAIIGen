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
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/StringHelper.hpp>
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


using namespace Fsl;

namespace MB
{

  namespace
  {
    const auto DEFAULT_VALUE_NOT_FOUND = "FIX_DEFAULT_FOR_TYPE_NOT_DEFINED";

    const auto END_OF_LINE = std::string("\r\n");

    struct Snippets
    {
      std::string ConstructorMemberInitialization;
      std::string CreateConstructorHeader;
      std::string CreateConstructorSource;
      std::string ResetSetMemberVariable;
      std::string ResetInvalidateMemberVariable;
      std::string ResetMemberAssertion;
      std::string ResetMemberHeader;
      std::string ResetMemberSource;
      std::string ResetParamValidation;
      std::string MoveAssignmentClaimMember;
      std::string MoveAssignmentInvalidateMember;
      std::string MoveConstructorInvalidateMember;
      std::string MoveConstructorClaimMember;
      std::string HandleClassName;
      std::string CreateVoidConstructorHeader;
      std::string CreateVoidConstructorSource;
      std::string ResetVoidMemberHeader;
      std::string ResetVoidMemberSource;
      std::string DefaultValueMod;
    };


    struct GeneratedMethodCode
    {
      std::string Header;
      std::string Source;

      GeneratedMethodCode()
      {
      }

      explicit GeneratedMethodCode(const std::string& str)
        : Header(str)
        , Source(str)
      {
      }

      explicit GeneratedMethodCode(const std::string& strHeader, const std::string& strSource)
        : Header(strHeader)
        , Source(strSource)
      {
      }
    };


    bool IsIgnoreParameter(const ParameterRecord& param, const std::vector<std::string>& forceNullParameter)
    {
      for (auto itr = forceNullParameter.begin(); itr != forceNullParameter.end(); ++itr)
      {
        if (*itr == param.Type.Name)
          return true;
      }
      return false;
    }

    struct MemberVariable
    {
      TypeRecord FullType;
      std::string Type;
      std::string Name;

      std::string ArgumentName;
      std::string NiceNameUpperCamelCase;

      MemberVariable()
      {
      }

      MemberVariable(const TypeRecord& fullType, const std::string& type, const std::string& name, const std::string& argumentName, const std::string& niceNameUpperCamelCase)
        : FullType(fullType)
        , Type(type)
        , Name(name)
        , ArgumentName(argumentName)
        , NiceNameUpperCamelCase(niceNameUpperCamelCase)
      {
      }

      bool IsValid() const
      {
        return Name.size() > 0;
      }

      bool operator==(const MemberVariable &rhs) const
      {
        return FullType == rhs.FullType &&
          Type == rhs.Type &&
          Name == rhs.Name &&
          ArgumentName == rhs.ArgumentName &&
          NiceNameUpperCamelCase == rhs.NiceNameUpperCamelCase;
      }

      bool operator!=(const MemberVariable &rhs) const
      {
        return !(*this == rhs);
      }
    };

    struct MethodArgument
    {
      std::string FullTypeString;
      std::string ArgumentName;
      std::string ParameterValue;

      MethodArgument()
      {
      }

      MethodArgument(const std::string& fullTypeString, const std::string& argumentName)
        : FullTypeString(fullTypeString)
        , ArgumentName(argumentName)
        , ParameterValue(argumentName)
      {
      }


      MethodArgument(const std::string& fullTypeString, const std::string& argumentName, const std::string& parameterValue)
        : FullTypeString(fullTypeString)
        , ArgumentName(argumentName)
        , ParameterValue(parameterValue)
      {
      }


      bool IsValid() const
      {
        return ArgumentName.size() > 0;
      }

      bool operator==(const MethodArgument &rhs) const
      {
        return FullTypeString == rhs.FullTypeString &&
          ArgumentName == rhs.ArgumentName &&
          ParameterValue == rhs.ParameterValue;
      }

      bool operator!=(const MethodArgument &rhs) const
      {
        return !(*this == rhs);
      }
    };


    struct AnalysisResult
    {
      std::string ClassName;
      std::string IntermediaryName;
      MemberVariable ResourceMemberVariable;
      std::deque<MemberVariable> AdditionalMemberVariables;
      std::deque<MemberVariable> AllMemberVariables;
      std::deque<MethodArgument> MethodArguments;
      std::deque<MethodArgument> CreateArguments;
      std::deque<MethodArgument> DestroyArguments;


      bool operator==(const AnalysisResult &rhs) const
      {
        return ClassName == rhs.ClassName &&
          IntermediaryName == rhs.IntermediaryName &&
          ResourceMemberVariable == rhs.ResourceMemberVariable &&
          AdditionalMemberVariables == rhs.AdditionalMemberVariables &&
          AllMemberVariables == rhs.AllMemberVariables &&
          MethodArguments == rhs.MethodArguments &&
          CreateArguments == rhs.CreateArguments &&
          DestroyArguments == rhs.DestroyArguments;
      }

      bool operator!=(const AnalysisResult &rhs) const
      {
        return !(*this == rhs);
      }
    };


    class FullAnalysis
    {
    public:
      MatchedFunctionPair Pair;
      AnalysisResult      Result;

      std::shared_ptr<std::deque<FullAnalysis> > AbsorbedFunctions;

      FullAnalysis()
        : Pair()
        , Result()
        , AbsorbedFunctions()
      {
      }

      FullAnalysis(const MatchedFunctionPair pair, const AnalysisResult& result)
        : Pair(pair)
        , Result(result)
        , AbsorbedFunctions()
      {
      }

      bool operator==(const FullAnalysis &rhs) const
      {
        
        return Pair == rhs.Pair && 
          Result == rhs.Result && 
          ((AbsorbedFunctions && rhs.AbsorbedFunctions && *AbsorbedFunctions == *rhs.AbsorbedFunctions) || (AbsorbedFunctions == rhs.AbsorbedFunctions));
      }

      bool operator!=(const FullAnalysis &rhs) const
      {
        return !(*this == rhs);
      }

    };


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


    MethodArgument ToMethodArgument(const ParameterRecord& value)
    {
      // C++'ify the arguments
      std::string fullTypeString;
      std::string argumentName(value.ArgumentName);
      std::string parameterName(value.ArgumentName);

      if (value.Type.IsConstQualified && value.Type.IsStruct && value.Type.IsPointer)
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

      return MethodArgument(fullTypeString, argumentName, parameterName);
    }

    MethodArgument ToMethodArgument(const MemberVariable& value)
    {
      // C++'ify the arguments
      std::string fullTypeString;
      std::string argumentName(value.ArgumentName);
      std::string parameterName(value.ArgumentName);

      if (value.FullType.IsConstQualified && value.FullType.IsStruct && value.FullType.IsPointer)
      {
        fullTypeString = "const " + value.FullType.Name + "&";
        argumentName = GetResourceArgumentName(value.FullType, value.ArgumentName);
        parameterName = "&" + argumentName;
      }
      else if (value.FullType.IsConstQualified)
        fullTypeString = value.FullType.FullTypeString;
      else if (!value.FullType.IsPointer)
        fullTypeString = "const " + value.FullType.FullTypeString;
      else
        fullTypeString = value.FullType.FullTypeString;

      return MethodArgument(fullTypeString, argumentName, parameterName);
    }


    std::string TypeToVariableName(const std::string& typeNamePrefix, const std::string& typeName)
    {
      if( typeName.find(typeNamePrefix) != 0 )
        return typeName;
      return StringHelper::EnforceLowerCamelCaseNameStyle(typeName.substr(typeNamePrefix.size()));
    }


    AnalysisResult Analyze(const SimpleGeneratorConfig& config, const MatchedFunctionPair& functions, const std::string& lowerCamelCaseClassName, const std::vector<std::string>& forceNullParameter)
    {
      bool resourceParameterFound = false;
      std::unordered_map<std::string, MethodArgument> dstMap;

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
                resourceParameterFound = true;
                // The non const pointer is normally the 'class' type of the create function
                std::cout << "  Param matched: " << itr->Type.Name << " (*)\n";
                const auto fixedArgumentName = GetResourceArgumentName(*itr);
                result.ResourceMemberVariable = ToMemberVariable(*itr, fixedArgumentName);
                result.IntermediaryName = result.ResourceMemberVariable.ArgumentName;


                { // Build the create argument
                  auto createArgument = ToMethodArgument(*itr);
                  if (itr->Type.IsPointer)
                    createArgument.ParameterValue = std::string("&") + result.IntermediaryName;
                  else
                    createArgument.ParameterValue = result.IntermediaryName;
                  result.CreateArguments.push_back(createArgument);
                }
                { // Build the destroy argument
                  auto destroyArgument = ToMethodArgument(*itr);
                  if (itrFind->Type.IsPointer)
                    destroyArgument.ParameterValue = "&" + result.ResourceMemberVariable.Name;
                  else
                    destroyArgument.ParameterValue = result.ResourceMemberVariable.Name;
                  dstMap[itrFind->ArgumentName] = destroyArgument;
                }
              }
              else
              {
                std::cout << "  Param matched: " << itr->Type.Name << "\n";
                auto member = ToMemberVariable(*itr);
                result.AdditionalMemberVariables.push_back(member);
                result.MethodArguments.push_back(ToMethodArgument(*itr));
                result.CreateArguments.push_back(ToMethodArgument(*itr));

                auto destroyArgument = ToMethodArgument(*itr);
                destroyArgument.ParameterValue = member.Name;
                dstMap[itrFind->ArgumentName] = destroyArgument;
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
              dstMap[itrFind->ArgumentName] = destroyArgument;
            }
          }
          else
          {
            std::cout << "  Param not matched: " << itr->Type.Name << "\n";
            result.MethodArguments.push_back(ToMethodArgument(*itr));
            result.CreateArguments.push_back(ToMethodArgument(*itr));
          }
        }
      }

      if (!resourceParameterFound)
      {
        // So no members matched the params to the destroy method, lets check if the return type matches
        const auto typeName = functions.Create.ReturnType.Name;
        auto itrFind = std::find_if(functions.Destroy.Parameters.begin(), functions.Destroy.Parameters.end(), [typeName](const ParameterRecord& val) { return val.Type.Name == typeName; });
        if(itrFind == functions.Destroy.Parameters.end())
          throw NotSupportedException(std::string("Could not find created resource parameter for method: ") + functions.Create.Name);

        // We found a return type that matches a destroy parameter
        result.IntermediaryName = TypeToVariableName(config.TypeNamePrefix, typeName);
        result.ResourceMemberVariable = MemberVariable(functions.Create.ReturnType, typeName, "m_" + result.IntermediaryName, result.IntermediaryName, CaseUtil::UpperCaseFirstCharacter(result.IntermediaryName));

        { // Build the destroy argument
          auto destroyArgument = ToMethodArgument(*itrFind);
          if (itrFind->Type.IsPointer)
            destroyArgument.ParameterValue = "&" + result.ResourceMemberVariable.Name;
          else
            destroyArgument.ParameterValue = result.ResourceMemberVariable.Name;
          dstMap[itrFind->ArgumentName] = destroyArgument;
        }

        std::cout << "  Return type used as resource: " << typeName << " (*)\n";
      }
 
      // Check intermediary name for collisions
      for (auto itr = functions.Create.Parameters.begin(); itr != functions.Create.Parameters.end(); ++itr)
      {
        if (itr->Name == result.IntermediaryName)
          throw NotSupportedException("Intermediary name collides with parameter name");
      }

      for (auto itr = functions.Destroy.Parameters.begin(); itr != functions.Destroy.Parameters.end(); ++itr)
      {
        result.DestroyArguments.push_back(dstMap[itr->ArgumentName]);
      }

      if (!result.ResourceMemberVariable.IsValid())
        throw NotSupportedException("The resource type could not be determined");

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


    void FindObjectFunctions(const FunctionAnalysis& functionAnalysis, const std::deque<FullAnalysis>& managed, const AnalysisResult& result)
    {
      std::cout << "Matching functions to " << result.ClassName << "\n";

      for (auto itr = functionAnalysis.Unmatched.begin(); itr != functionAnalysis.Unmatched.end(); ++itr)
      {
        // 1. The start parameter types and order must be exactly the same as this objects member variables
        // 2. Any following parameter must not be of a 'managed' type (one that we generate a object for)
        if (IsExactStartParameterMatch(result.AllMemberVariables, *itr) )
        {
          const std::size_t numMembers = result.AllMemberVariables.size();
          if (itr->Parameters.size() <= numMembers || !IsManagedType(managed, itr->Parameters[numMembers]))
          {
            std::cout << "+ " << itr->Name << "\n";
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


    std::string GenerateForAllMembers(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const std::string snippetTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues)
    {
      const bool scriptUsesDefaultValue = ContainsDefaultValue(snippetTemplate);

      std::string result;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = snippetTemplate;
        StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
        StringUtil::Replace(snippet, "##MEMBER_ARGUMENT_NAME##", itr->ArgumentName);
        if (scriptUsesDefaultValue)
        {
          ReplaceDefaultValue(snippet, itr->Type, typeDefaultValues, snippets);
        }
        result += END_OF_LINE + snippet;
      }
      return result;
    }


    std::string GenerateConstructorInitialization(const Snippets& snippets, const std::deque<MemberVariable>& allMemberVariables, const std::string snippetTemplate, const std::unordered_map<std::string, std::string>& typeDefaultValues)
    {
      const bool scriptUsesDefaultValue = ContainsDefaultValue(snippetTemplate);

      std::string result;
      std::string seperator = ":";
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        std::string snippet = snippetTemplate;
        StringUtil::Replace(snippet, "##MEMBER_NAME##", itr->Name);
        StringUtil::Replace(snippet, "##MEMBER_SEPERATOR##", seperator);
        if (scriptUsesDefaultValue)
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


    std::deque<FullAnalysis> Analyze(const SimpleGeneratorConfig& config, const FunctionAnalysis& functionAnalysis, std::unordered_set<std::string>& rTypesWithoutDefaultValues)
    {
      std::deque<FullAnalysis> managed;
      for (auto itr = functionAnalysis.Matched.begin(); itr != functionAnalysis.Matched.end(); ++itr)
      {
        if (itr->MatchTypeInfo == MatchType::Manual)
          std::cout << "Manual matched: " << itr->Create.Name << " with " << itr->Destroy.Name << "\n";
        else
          std::cout << "Matched: " << itr->Create.Name << " with " << itr->Destroy.Name << "\n";

        const auto result = Analyze(config, *itr, CaseUtil::LowerCaseFirstCharacter(itr->Name), config.ForceNullParameter);

        CheckDefaultValues(rTypesWithoutDefaultValues, result.AllMemberVariables, config.TypeDefaultValues);

        managed.push_back(FullAnalysis(*itr, result));
      }

      // 
      AbsorbFunctions(managed, config.ClassFunctionAbsorbtion);


      for (auto itr = managed.begin(); itr != managed.end(); ++itr)
      {
        FindObjectFunctions(functionAnalysis, managed, itr->Result);
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
      const std::string resetParamValidation = GenerateForAllMembers(snippets, fullAnalysis.Result.AdditionalMemberVariables, snippets.ResetParamValidation, config.TypeDefaultValues);

      std::string content(activeSnippet);
      StringUtil::Replace(content, "##CLASS_NAME##", fullAnalysis.Result.ClassName);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETERS##", createMethodParameters);
      StringUtil::Replace(content, "##CREATE_METHOD_PARAMETER_NAMES##", createMethodParameterNames);
      StringUtil::Replace(content, "##CREATE_FUNCTION_ARGUMENTS##", createFunctionArguments);
      StringUtil::Replace(content, "##CREATE_FUNCTION##", fullAnalysis.Pair.Create.Name);
      StringUtil::Replace(content, "##RESET_PARAMETER_VALIDATION##", resetParamValidation);
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


    std::string GenerateMemberAsParameters(const std::deque<MemberVariable>& allMemberVariables)
    {
      std::string content;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        const auto resourceAsArgument = ToMethodArgument(*itr);
        content += resourceAsArgument.FullTypeString + " " + resourceAsArgument.ArgumentName;
        if ((itr + 1) != allMemberVariables.end())
          content += ", ";
      }
      return content;
    }

    std::string GenerateMemberAsNames(const std::deque<MemberVariable>& allMemberVariables)
    {
      std::string content;
      for (auto itr = allMemberVariables.begin(); itr != allMemberVariables.end(); ++itr)
      {
        const auto resourceAsArgument = ToMethodArgument(*itr);
        content += resourceAsArgument.ArgumentName;
        if ((itr + 1) != allMemberVariables.end())
          content += ", ";
      }
      return content;
    }


    std::string GenerateContent(const SimpleGeneratorConfig& config, const FullAnalysis& fullAnalysis, const std::string& contentTemplate, const std::string*const pSnippetMemberVariable,
                                const std::string*const pSnippetMemberVariableGet, const Snippets& snippets)
    {
      // Generate the RAII resource class header file
      std::string content = contentTemplate;

      std::string classAdditionalMemberVariables;
      std::string classAdditionalGetMemberVariablesMethods;
      {
        if (fullAnalysis.Result.AdditionalMemberVariables.size() > 0)
        {
          if (pSnippetMemberVariable != nullptr)
            classAdditionalMemberVariables = END_OF_LINE;
          if (pSnippetMemberVariableGet != nullptr)
            classAdditionalGetMemberVariablesMethods = END_OF_LINE;
        }
        for (auto itr = fullAnalysis.Result.AdditionalMemberVariables.begin(); itr != fullAnalysis.Result.AdditionalMemberVariables.end(); ++itr)
        {
          if (pSnippetMemberVariable != nullptr)
          {
            std::string memberVariable = *pSnippetMemberVariable;
            StringUtil::Replace(memberVariable, "##MEMBER_TYPE##", itr->Type);
            StringUtil::Replace(memberVariable, "##MEMBER_NAME##", itr->Name);
            classAdditionalMemberVariables += memberVariable;
          }

          if (pSnippetMemberVariableGet != nullptr)
          {
            std::string memberVariableGet = *pSnippetMemberVariableGet;
            StringUtil::Replace(memberVariableGet, "##MEMBER_TYPE##", itr->Type);
            StringUtil::Replace(memberVariableGet, "##MEMBER_NAME##", itr->Name);
            StringUtil::Replace(memberVariableGet, "##MEMBER_NICE_NAME##", itr->NiceNameUpperCamelCase);
            classAdditionalGetMemberVariablesMethods += memberVariableGet;
          }
        }
      }

      const auto resourceAsArgument = ToMethodArgument(fullAnalysis.Result.ResourceMemberVariable);

      const std::string memberParameters = GenerateMemberAsParameters(fullAnalysis.Result.AllMemberVariables);
      const std::string memberParameterNames = GenerateMemberAsNames(fullAnalysis.Result.AllMemberVariables);
      const std::string resetSetMembers = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetSetMemberVariable, config.TypeDefaultValues);
      const std::string defaultConstructorInitialization = GenerateConstructorInitialization(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ConstructorMemberInitialization, config.TypeDefaultValues);
      const std::string moveConstructorMemberInitialization = GenerateConstructorInitialization(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveConstructorClaimMember, config.TypeDefaultValues);
      const std::string moveAssignmentClaimMembers = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveAssignmentClaimMember, config.TypeDefaultValues);
      const std::string moveAssignmentInvalidateMembers = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveAssignmentInvalidateMember, config.TypeDefaultValues);
      const std::string moveConstructorInvalidateMembers = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.MoveConstructorInvalidateMember, config.TypeDefaultValues);
      const std::string resetInvalidateMembers = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetInvalidateMemberVariable, config.TypeDefaultValues);
      const std::string resetMemberAssertions = GenerateForAllMembers(snippets, fullAnalysis.Result.AllMemberVariables, snippets.ResetMemberAssertion, config.TypeDefaultValues);

      const std::string destroyFunctionArguments = GenerateExpandedParameterNameList(fullAnalysis.Result.DestroyArguments);

      auto classExtraConstructors = GenerateExtraCreates(config, snippets, snippets.CreateConstructorHeader, snippets.CreateConstructorSource, snippets.CreateVoidConstructorHeader, snippets.CreateVoidConstructorSource, fullAnalysis);
      auto classExtraResetMethods = GenerateExtraCreates(config, snippets, snippets.ResetMemberHeader, snippets.ResetMemberSource, snippets.ResetVoidMemberHeader, snippets.ResetVoidMemberSource, fullAnalysis);

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
  }


  // FIX: 
  // - Handle 'create array' of resources, example: CommandBuffers
  // - Add helper methods 

  SimpleGenerator::SimpleGenerator(const Capture& capture, const SimpleGeneratorConfig& config, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : Generator(capture, config)
  {
    const auto pathResetModeHeader = IO::Path::Combine(templateRoot, "TemplateResetMode_header.hpp");
    const auto resetModeHeaderTemplate = IO::File::ReadAllText(pathResetModeHeader);

    const auto pathHeader = IO::Path::Combine(templateRoot, "Template_header.hpp");
    const auto pathHeaderSnippetMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_MemberVariable.txt");
    const auto pathHeaderSnippetMemberVariableGet = IO::Path::Combine(templateRoot, "TemplateSnippet_MemberVariableGet.txt");
    const auto headerTemplate = IO::File::ReadAllText(pathHeader);
    const auto headerSnippetMemberVariable = IO::File::ReadAllText(pathHeaderSnippetMemberVariable);
    const auto headerSnippetMemberVariableGet = IO::File::ReadAllText(pathHeaderSnippetMemberVariableGet);

    const auto pathSource = IO::Path::Combine(templateRoot, "Template_source.cpp");
    const auto pathSnippetConstructorMemberInitialization = IO::Path::Combine(templateRoot, "TemplateSnippet_ConstructorMemberInitialization.txt");
    const auto pathSnippetCreateConstructorHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateConstructorHeader.txt");
    const auto pathSnippetCreateConstructorSource = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateConstructorSource.txt");
    const auto pathSnippetResetSetMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetSetMemberVariable.txt");
    const auto pathSnippetResetInvalidateMemberVariable = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetInvalidateMemberVariable.txt");
    const auto pathSnippetResetMemberAssertion = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberAssertion.txt");
    const auto pathSnippetResetMemberHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberHeader.txt");
    const auto pathSnippetResetMemberSource = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetMemberSource.txt");
    const auto pathSnippetResetParamValidation = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetParamValidation.txt");
    const auto pathSnippetMoveAssignmentClaimMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveAssignmentClaimMember.txt");
    const auto pathSnippetMoveAssignmentInvalidateMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveAssignmentInvalidateMember.txt");
    const auto pathSnippetMoveConstructorInvalidateMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveConstructorInvalidateMember.txt");
    const auto pathSnippetMoveConstructorClaimMember = IO::Path::Combine(templateRoot, "TemplateSnippet_MoveConstructorClaimMember.txt");
    const auto pathSnippetHandleClassName = IO::Path::Combine(templateRoot, "TemplateSnippet_HandleClassName.txt");
    const auto pathSnippetCreateVoidConstructorHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateVoidConstructorHeader.txt");
    const auto pathSnippetCreateVoidConstructorSource = IO::Path::Combine(templateRoot, "TemplateSnippet_CreateVoidConstructorSource.txt");
    const auto pathSnippetResetVoidMemberHeader = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetVoidMemberHeader.txt");
    const auto pathSnippetResetVoidMemberSource = IO::Path::Combine(templateRoot, "TemplateSnippet_ResetVoidMemberSource.txt");
    const auto pathSnippetDefaultValueMod = IO::Path::Combine(templateRoot, "TemplateSnippet_DefaultValueMod.txt");
    

    const auto sourceTemplate = IO::File::ReadAllText(pathSource);
    Snippets snippets;
    snippets.ConstructorMemberInitialization = IO::File::ReadAllText(pathSnippetConstructorMemberInitialization);
    snippets.CreateConstructorHeader = IO::File::ReadAllText(pathSnippetCreateConstructorHeader);
    snippets.CreateConstructorSource = IO::File::ReadAllText(pathSnippetCreateConstructorSource);
    snippets.ResetSetMemberVariable = IO::File::ReadAllText(pathSnippetResetSetMemberVariable);
    snippets.ResetInvalidateMemberVariable = IO::File::ReadAllText(pathSnippetResetInvalidateMemberVariable);
    snippets.ResetMemberAssertion = IO::File::ReadAllText(pathSnippetResetMemberAssertion);
    snippets.ResetMemberHeader = IO::File::ReadAllText(pathSnippetResetMemberHeader);
    snippets.ResetMemberSource = IO::File::ReadAllText(pathSnippetResetMemberSource);
    snippets.ResetParamValidation = IO::File::ReadAllText(pathSnippetResetParamValidation);
    snippets.MoveAssignmentClaimMember = IO::File::ReadAllText(pathSnippetMoveAssignmentClaimMember);
    snippets.MoveAssignmentInvalidateMember = IO::File::ReadAllText(pathSnippetMoveAssignmentInvalidateMember);
    snippets.MoveConstructorInvalidateMember = IO::File::ReadAllText(pathSnippetMoveConstructorInvalidateMember);
    snippets.MoveConstructorClaimMember = IO::File::ReadAllText(pathSnippetMoveConstructorClaimMember);
    snippets.HandleClassName = IO::File::ReadAllText(pathSnippetHandleClassName);
    snippets.CreateVoidConstructorHeader = IO::File::ReadAllText(pathSnippetCreateVoidConstructorHeader);
    snippets.CreateVoidConstructorSource = IO::File::ReadAllText(pathSnippetCreateVoidConstructorSource);
    snippets.ResetVoidMemberHeader = IO::File::ReadAllText(pathSnippetResetVoidMemberHeader);
    snippets.ResetVoidMemberSource = IO::File::ReadAllText(pathSnippetResetVoidMemberSource);
    snippets.DefaultValueMod= IO::File::ReadAllText(pathSnippetDefaultValueMod);

    std::unordered_set<std::string> typesWithoutDefaultValues;

    auto fullAnalysis = Analyze(config, m_functionAnalysis, typesWithoutDefaultValues);


    const bool generateSourceFile = sourceTemplate.size() > 0;
    for (auto itr = fullAnalysis.begin(); itr != fullAnalysis.end(); ++itr)
    {
      {
        auto headerContent = GenerateContent(config, *itr, headerTemplate, &headerSnippetMemberVariable, &headerSnippetMemberVariableGet, snippets);
        auto fileName = IO::Path::Combine(dstPath, itr->Result.ClassName + ".hpp");
        WriteAllTextIfChanged(fileName, headerContent);
      }
      if (generateSourceFile)
      {
        auto sourceContent = GenerateContent(config, *itr, sourceTemplate, nullptr, nullptr, snippets);
        auto fileName = IO::Path::Combine(dstPath, itr->Result.ClassName + ".cpp");
        WriteAllTextIfChanged(fileName, sourceContent);
      }
    }

    const auto copyRoot = IO::Path::Combine(templateRoot, "copy");
    IO::PathDeque files;
    if (IO::Directory::TryGetFiles(files, copyRoot, IO::SearchOptions::TopDirectoryOnly))
    {
      if (files.size() > 0)
      {
        for (auto itr = files.begin(); itr != files.end(); ++itr)
        {
          auto srcContent = IO::File::ReadAllText(**itr);
          auto dstFileName = IO::Path::Combine(dstPath, IO::Path::GetFileName(**itr));
          WriteAllTextIfChanged(dstFileName, srcContent);
        }
      }
    }

    if (resetModeHeaderTemplate.size() > 0 && IsResetModeRequired(fullAnalysis))
    {
      std::string content(resetModeHeaderTemplate);
      StringUtil::Replace(content, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      auto dstFileName = IO::Path::Combine(dstPath, "ResetMode.hpp");
      WriteAllTextIfChanged(dstFileName, content);
    }

    // Write 'Readme.txt'
    {
      std::string content("Auto-generated ##API_NAME## ##API_VERSION## C++11 RAII classes by ##PROGRAM_NAME## ##PROGRAM_VERSION## (https://github.com/Unarmed1000)" + END_OF_LINE);

      StringUtil::Replace(content, "##API_NAME##", config.APIName);
      StringUtil::Replace(content, "##API_VERSION##", config.APIVersion);
      StringUtil::Replace(content, "##PROGRAM_NAME##", config.Program.Name);
      StringUtil::Replace(content, "##PROGRAM_VERSION##", config.Program.Version);
      auto dstFileName = IO::Path::Combine(dstPath, "RAIIGenVersion.txt");
      WriteAllTextIfChanged(dstFileName, content);
    }


    for (auto itr = m_functionAnalysis.MissingDestroy.begin(); itr != m_functionAnalysis.MissingDestroy.end(); ++itr)
      std::cout << "WARNING: No match found for: " << *itr << "\n";

    std::vector<std::string> sortedTypesWithoutDefaultValues(typesWithoutDefaultValues.begin(), typesWithoutDefaultValues.end());
    std::sort(sortedTypesWithoutDefaultValues.begin(), sortedTypesWithoutDefaultValues.end());
    for (auto itr = sortedTypesWithoutDefaultValues.begin(); itr != sortedTypesWithoutDefaultValues.end(); ++itr)
      std::cout << "WARNING: Missing default value for type: " << *itr << "\n";


  }
}
