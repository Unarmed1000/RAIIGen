#ifndef MB_GENERATOR_SIMPLE_FULLANALYSIS_HPP
#define MB_GENERATOR_SIMPLE_FULLANALYSIS_HPP
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

#include <RAIIGen/Generator/Simple/AnalyzeMode.hpp>
#include <RAIIGen/Generator/Simple/AnalysisResult.hpp>
#include <RAIIGen/Generator/MatchedFunctionPair.hpp>
#include <deque>
#include <memory>

namespace MB
{
  class FullAnalysis
  {
  public:
    MatchedFunctionPair Pair;
    AnalysisResult      Result;
    AnalyzeMode         Mode;

    std::shared_ptr<std::deque<FullAnalysis> > AbsorbedFunctions;

    FullAnalysis()
      : Pair()
      , Result()
      , AbsorbedFunctions()
      , Mode(AnalyzeMode::Normal)
    {
    }

    FullAnalysis(const MatchedFunctionPair pair, const AnalysisResult& result, const AnalyzeMode mode)
      : Pair(pair)
      , Result(result)
      , AbsorbedFunctions()
      , Mode(mode)
    {
    }

    bool operator==(const FullAnalysis &rhs) const
    {

      return Pair == rhs.Pair &&
        Result == rhs.Result &&
        Mode == rhs.Mode &&
        ((AbsorbedFunctions && rhs.AbsorbedFunctions && *AbsorbedFunctions == *rhs.AbsorbedFunctions) || (AbsorbedFunctions == rhs.AbsorbedFunctions));
    }

    bool operator!=(const FullAnalysis &rhs) const
    {
      return !(*this == rhs);
    }
  };
}
#endif
