#ifndef MB_GENERATOR_SIMPLE_SIMPLEGENERATORCONFIG_HPP
#define MB_GENERATOR_SIMPLE_SIMPLEGENERATORCONFIG_HPP
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

#include <RAIIGen/Generator/GeneratorConfig.hpp>
#include <RAIIGen/Generator/BlackListEntry.hpp>
#include <RAIIGen/Generator/ClassFunctionAbsorb.hpp>
#include <RAIIGen/Generator/FunctionGuard.hpp>
#include <RAIIGen/Generator/RAIIClassCustomization.hpp>
#include <RAIIGen/Generator/RAIIClassMethodOverrides.hpp>
#include <unordered_map>

namespace MB
{
  struct SimpleGeneratorConfig : public GeneratorConfig
  {
    const std::vector<RAIIClassCustomization> RAIIClassCustomizations;
    const std::vector<ClassFunctionAbsorb> ClassFunctionAbsorbtion;
    const std::unordered_map<std::string, RAIIClassMethodOverrides> ClassMethodOverrides;
    const std::unordered_map<std::string, std::string> TypeDefaultValues;
    const std::vector<std::string> ForceNullParameter;
    const std::vector<FunctionGuard> FunctionGuards;
    const std::vector<BlackListEntry> FunctionNameBlacklist;
    const std::vector<BlackListEntry> EnumNameBlacklist;
    const std::vector<BlackListEntry> EnumMemberBlacklist;
    const std::string TypeNamePrefix;
    const std::string FunctionNamePrefix;
    const std::string ErrorCodeTypeName;
    const bool UnrollCreateStructs;
    const bool OwnershipTransferUseClaimMode;
    const bool IsVulkan;

    SimpleGeneratorConfig()
      : UnrollCreateStructs(false)
      , OwnershipTransferUseClaimMode(false)
      , IsVulkan(false)
    {
    }


    SimpleGeneratorConfig(const GeneratorConfig& config,
      const std::vector<RAIIClassCustomization>& raiiClassCustomizations,
      const std::vector<ClassFunctionAbsorb>& classFunctionAbsorbtion,
      const std::unordered_map<std::string, RAIIClassMethodOverrides> classMethodOverrides,
      const std::unordered_map<std::string, std::string>& typeDefaultValues, const std::vector<std::string>& forceNullParameter,
      const std::vector<FunctionGuard>& functionGuards,
      const std::vector<BlackListEntry>& functionNameBlacklist,
      const std::vector<BlackListEntry>& enumNameBlacklist,
      const std::vector<BlackListEntry>& enumMemberBlacklist,
      const std::string& typeNamePrefix, const std::string& functionNamePrefix, const std::string& errorCodeTypeName, const bool unrollCreateStructs,
      const bool ownershipTransferUseClaimMode, const bool isVulkan=false)
      : GeneratorConfig(config)
      , RAIIClassCustomizations(raiiClassCustomizations)
      , ClassFunctionAbsorbtion(classFunctionAbsorbtion)
      , ClassMethodOverrides(classMethodOverrides)
      , TypeDefaultValues(typeDefaultValues)
      , ForceNullParameter(forceNullParameter)
      , FunctionGuards(functionGuards)
      , FunctionNameBlacklist(functionNameBlacklist)
      , EnumNameBlacklist(enumNameBlacklist)
      , EnumMemberBlacklist(enumMemberBlacklist)
      , TypeNamePrefix(typeNamePrefix)
      , FunctionNamePrefix(functionNamePrefix)
      , ErrorCodeTypeName(errorCodeTypeName)
      , UnrollCreateStructs(unrollCreateStructs)
      , OwnershipTransferUseClaimMode(ownershipTransferUseClaimMode)
      , IsVulkan(isVulkan)
    {
    }

    SimpleGeneratorConfig(const BasicConfig& basicConfig, 
                          const std::vector<FunctionNamePair>& functionPairs, const std::vector<FunctionNamePair>& manualFunctionMatches,
                          const std::vector<RAIIClassCustomization>& raiiClassCustomizations,
                          const std::vector<ClassFunctionAbsorb>& classFunctionAbsorbtion,
                          const std::unordered_map<std::string, RAIIClassMethodOverrides> classMethodOverrides,
                          const std::unordered_map<std::string, std::string>& typeDefaultValues, const std::vector<std::string>& forceNullParameter,
                          const std::vector<FunctionGuard>& functionGuards,
                          const std::vector<BlackListEntry>& functionNameBlacklist,
                          const std::vector<BlackListEntry>& enumNameBlacklist,
                          const std::vector<BlackListEntry>& enumMemberBlacklist,
                          const std::string& typeNamePrefix, const std::string& functionNamePrefix, const std::string& errorCodeTypeName, 
                          const bool unrollCreateStructs, const bool ownershipTransferUseClaimMode, const bool isVulkan = false)
      : GeneratorConfig(basicConfig, functionPairs, manualFunctionMatches)
      , RAIIClassCustomizations(raiiClassCustomizations)
      , ClassFunctionAbsorbtion(classFunctionAbsorbtion)
      , ClassMethodOverrides(classMethodOverrides)
      , TypeDefaultValues(typeDefaultValues)
      , ForceNullParameter(forceNullParameter)
      , FunctionGuards(functionGuards)
      , FunctionNameBlacklist(functionNameBlacklist)
      , EnumNameBlacklist(enumNameBlacklist)
      , EnumMemberBlacklist(enumMemberBlacklist)
      , TypeNamePrefix(typeNamePrefix)
      , FunctionNamePrefix(functionNamePrefix)
      , ErrorCodeTypeName(errorCodeTypeName)
      , UnrollCreateStructs(unrollCreateStructs)
      , OwnershipTransferUseClaimMode(ownershipTransferUseClaimMode)
      , IsVulkan(isVulkan)
    {
    }
  };
}

#endif
