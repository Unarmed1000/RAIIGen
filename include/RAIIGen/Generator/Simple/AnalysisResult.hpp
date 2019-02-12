#ifndef MB_GENERATOR_SIMPLE_ANALYSISRESULT_HPP
#define MB_GENERATOR_SIMPLE_ANALYSISRESULT_HPP
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

#include <RAIIGen/Generator/Simple/MemberVariable.hpp>
#include <RAIIGen/Generator/Simple/UnrolledCreateMethod.hpp>
#include <deque>

namespace MB
{
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
    UnrolledCreateMethod UnrolledCreateMethod;
    std::string ResourceCountVariableName;

    bool operator==(const AnalysisResult& rhs) const
    {
      return ClassName == rhs.ClassName && IntermediaryName == rhs.IntermediaryName && ResourceMemberVariable == rhs.ResourceMemberVariable &&
             AdditionalMemberVariables == rhs.AdditionalMemberVariables && AllMemberVariables == rhs.AllMemberVariables &&
             MethodArguments == rhs.MethodArguments && CreateArguments == rhs.CreateArguments && DestroyArguments == rhs.DestroyArguments &&
             UnrolledCreateMethod == rhs.UnrolledCreateMethod && ResourceCountVariableName == rhs.ResourceCountVariableName;
    }

    bool operator!=(const AnalysisResult& rhs) const
    {
      return !(*this == rhs);
    }
  };
}
#endif
