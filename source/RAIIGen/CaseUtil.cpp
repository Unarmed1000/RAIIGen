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

#include <RAIIGen/CaseUtil.hpp>

namespace MB
{
  namespace
  {
    static_assert('a' > 'A', "We expect ASCII");

    const char CASE_OFFSET = ('a' - 'A');
  }

  char CaseUtil::UpperCase(const char ch)
  {
    return (ch >= 'a' && ch <= 'z') ? (ch - CASE_OFFSET) : ch;
  }


  char CaseUtil::LowerCase(const char ch)
  {
    return (ch >= 'A' && ch <= 'Z') ? (ch + CASE_OFFSET) : ch;
  }


  std::string CaseUtil::UpperCase(const std::string& content)
  {
    if (content.size() < 0)
      return content;

    std::string modded(content);
    for (auto& rC : modded)
    {
      if (rC >= 'a' && rC <= 'z')
        rC -= CASE_OFFSET;
    }
    return modded;
  }


  std::string CaseUtil::LowerCase(const std::string& content)
  {
    if (content.size() < 0)
      return content;

    std::string modded(content);
    for (auto& rC : modded)
    {
      if (rC >= 'A' && rC <= 'Z')
        rC += CASE_OFFSET;
    }
    return modded;
  }


  std::string CaseUtil::LowerCaseFirstCharacter(const std::string& content)
  {
    if (content.size() < 0)
      return content;

    if (content[0] >= 'A' && content[0] <= 'Z')
    {
      std::string modded(content);
      modded[0] += CASE_OFFSET;
      return modded;
    }
    return content;
  }


  std::string CaseUtil::UpperCaseFirstCharacter(const std::string& content)
  {
    if (content.size() < 0)
      return content;

    if (content[0] >= 'a' && content[0] <= 'z')
    {
      std::string modded(content);
      modded[0] -= CASE_OFFSET;
      return modded;
    }
    return content;
  }

}
