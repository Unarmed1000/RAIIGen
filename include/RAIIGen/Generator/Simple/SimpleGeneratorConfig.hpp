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
#include <RAIIGen/Generator/ClassFunctionAbsorb.hpp>
#include <unordered_map>

namespace MB
{
  struct SimpleGeneratorConfig : public GeneratorConfig
  {
    const std::vector<ClassFunctionAbsorb> ClassFunctionAbsorbtion;
    const std::unordered_map<std::string, std::string> TypeDefaultValues;
    const std::vector<std::string> ForceNullParameter;
    const std::string TypeNamePrefix;
    const bool UnrollCreateStructs;

    SimpleGeneratorConfig()
      : UnrollCreateStructs(false)
    {
    }


    SimpleGeneratorConfig(const GeneratorConfig& config,
      const std::vector<ClassFunctionAbsorb>& classFunctionAbsorbtion,
      const std::unordered_map<std::string, std::string>& typeDefaultValues, const std::vector<std::string>& forceNullParameter,
      const std::string& typeNamePrefix, const bool unrollCreateStructs)
      : GeneratorConfig(config)
      , ClassFunctionAbsorbtion(classFunctionAbsorbtion)
      , TypeDefaultValues(typeDefaultValues)
      , ForceNullParameter(forceNullParameter)
      , TypeNamePrefix(typeNamePrefix)
      , UnrollCreateStructs(unrollCreateStructs)
    {
    }

    SimpleGeneratorConfig(const BasicConfig& basicConfig, 
                          const std::vector<FunctionNamePair>& functionPairs, const std::vector<FunctionNamePair>& manualFunctionMatches,
                          const std::vector<ClassFunctionAbsorb>& classFunctionAbsorbtion, 
                          const std::unordered_map<std::string, std::string>& typeDefaultValues, const std::vector<std::string>& forceNullParameter,
                          const std::string& typeNamePrefix, const bool unrollCreateStructs)
      : GeneratorConfig(basicConfig, functionPairs, manualFunctionMatches)
      , ClassFunctionAbsorbtion(classFunctionAbsorbtion)
      , TypeDefaultValues(typeDefaultValues)
      , ForceNullParameter(forceNullParameter)
      , TypeNamePrefix(typeNamePrefix)
      , UnrollCreateStructs(unrollCreateStructs)
    {
    }
  };
}

#endif
