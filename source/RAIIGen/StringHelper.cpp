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

#include <RAIIGen/StringHelper.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <FslBase/Exceptions.hpp>
#include <vector>

namespace MB
{
  using namespace Fsl;


  std::string StringHelper::EnforceLowerCamelCaseNameStyle(const std::string& value)
  {
    std::string modded(value.size(), ' ');
    const auto length = value.size();
    std::size_t dstIndex = 0;
    bool uppercaseNextChar = false;
    for (std::size_t srcIndex = 0; srcIndex < length; ++srcIndex)
    {
      if (value[srcIndex] != ' ')
      {
        if (value[srcIndex] == '_')
        {
          uppercaseNextChar = true;
        }
        else if (uppercaseNextChar)
        {
          modded[dstIndex] = CaseUtil::UpperCase(value[srcIndex]);
          uppercaseNextChar = false;
          ++dstIndex;
        }
        else
        {
          modded[dstIndex] = value[srcIndex];
          ++dstIndex;
        }
      }
      else
        uppercaseNextChar = true;
    }
    modded.resize(dstIndex);
    return modded;
  }


  std::string StringHelper::EnforceUpperCamelCaseNameStyle(const std::string& value)
  {
    std::string modded(value.size(), ' ');
    const auto length = value.size();
    std::size_t dstIndex = 0;
    bool uppercaseNextChar = true;
    for (std::size_t srcIndex = 0; srcIndex < length; ++srcIndex)
    {
      if (value[srcIndex] != ' ')
      {
        if (value[srcIndex] == '_')
        {
          uppercaseNextChar = true;
        }
        else if (uppercaseNextChar)
        {
          modded[dstIndex] = CaseUtil::UpperCase(value[srcIndex]);
          uppercaseNextChar = false;
          ++dstIndex;
        }
        else
        {
          modded[dstIndex] = value[srcIndex];
          ++dstIndex;
        }
      }
      else
        uppercaseNextChar = true;
    }
    modded.resize(dstIndex);
    return modded;
  }


  std::string StringHelper::GenerateVulkanStructFlagName(const std::string structName)
  {
    if (structName.size() < 2 || structName[0] != 'V' || structName[1] != 'k')
      throw NotSupportedException("the struct did not start with the expected Vk");

    auto flagName = structName.substr(2);
    std::vector<char> dst(flagName.size() * 2);
    std::size_t dstIndex = 0;
    bool previousCharWasUpper = true;
    for (std::size_t i = 0; i < flagName.size(); ++i, ++dstIndex)
    {
      if (flagName[i] >= 'A' && flagName[i] <= 'Z')
      {
        if (!previousCharWasUpper)
        {
          dst[dstIndex] = '_';
          ++dstIndex;
        }
        previousCharWasUpper = true;
      }
      else
        previousCharWasUpper = false;
      dst[dstIndex] = CaseUtil::UpperCase(flagName[i]);
    }
    return "VK_STRUCTURE_TYPE_" + std::string(dst.data(), dstIndex);
  }

}
