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

#include <RAIIGen/Generator/Generator.hpp>
#include <RAIIGen/Capture.hpp>
#include <FslBase/IO/File.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <unordered_set>
#include <string>

using namespace Fsl;

namespace MB
{

  namespace
  {
  }


  Generator::Generator(const Capture& capture, const GeneratorConfig& config)
  {
    std::unordered_set<std::string> matchedFunctionNames;
    const auto functionRecords = capture.GetFunctions();

    for (auto itr = functionRecords.begin(); itr != functionRecords.end(); ++itr)
    {
      for (auto itrFunc = config.FunctionPairs.begin(); itrFunc != config.FunctionPairs.end(); ++itrFunc)
      {
        if (itr->Name.find(itrFunc->Create) == 0)
        {
          matchedFunctionNames.insert(itr->Name);

          std::string objectName = itr->Name.substr(itrFunc->Create.size());
          std::string destroyMethodName = itrFunc->Destroy + objectName;

          // First we try to find a matching destroy method for functions that follow the 'standard' pattern
          const auto itrFind = FindByName(functionRecords, destroyMethodName);
          if (itrFind != functionRecords.end())
          {
            m_functionAnalysis.Matched.push_back( MatchedFunctionPair(objectName, *itr, *itrFind, MatchType::Auto) );
            matchedFunctionNames.insert(itrFind->Name);
          }
          else
          {
            bool found = false;
            // Check if its a manual supported function
            const auto name = itr->Name;
            const auto itrFind2 = std::find_if(config.ManualFunctionMatches.begin(), config.ManualFunctionMatches.end(), [name](const FunctionNamePair& val) { return val.Create == name; });
            if (itrFind2 != config.ManualFunctionMatches.end())
            {
              const auto itrDestroy = FindByName(functionRecords, itrFind2->Destroy);
              if (itrDestroy != functionRecords.end())
              {
                m_functionAnalysis.Matched.push_back(MatchedFunctionPair(objectName, *itr, *itrDestroy, MatchType::Manual));
                matchedFunctionNames.insert(itrDestroy->Name);
                found = true;
              }
              else
                std::cout << "WARNING: The manual matched destroy funtion '" << itrFind2->Destroy << "' was not found.\n";
            }

            if (!found)
            {
              m_functionAnalysis.MissingDestroy.push_back(itr->Name);
            }
          }
        }
      }
    }

    // Store all the unmatched functions for later use
    for (auto itr = functionRecords.begin(); itr != functionRecords.end(); ++itr)
    {
      if (matchedFunctionNames.find(itr->Name) == matchedFunctionNames.end())
      {
        m_functionAnalysis.Unmatched.push_back(*itr);
      }
    }
  }


  std::deque<FunctionRecord>::const_iterator Generator::FindByName(const std::deque<FunctionRecord>& records, const std::string& name)
  {
    return std::find_if(records.begin(), records.end(), [name](const FunctionRecord& val) { return val.Name == name; });
  }

  

}
