#ifndef MB_GENERATOR_CONFIGUTIL_HPP
#define MB_GENERATOR_CONFIGUTIL_HPP
//***************************************************************************************************************************************************
//* BSD 3-Clause License
//*
//* Copyright (c) 2017, Rene Thrane
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

#include <FslBase/Exceptions.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <RAIIGen/Generator/BlackListEntry.hpp>

namespace MB
{
  namespace ConfigUtil
  {
    struct CurrentEntityInfo
    {
      std::string ClassName;

      CurrentEntityInfo(const std::string& className)
        : ClassName(className)
      {
      }
    };


    inline bool CheckMatchRequirement(const BlackListEntry& entry, const CurrentEntityInfo& currentEntityInfo)
    {
      switch (entry.MatchRequirement)
      {
      case BlackListMatch::Always:
        return true;
      case BlackListMatch::NotPostfixClassName:
        return ! Fsl::StringUtil::EndsWith(currentEntityInfo.ClassName, entry.Name);
      default:
        throw Fsl::NotSupportedException("BlackListMatch type not supported");
      }
    }


    inline bool HasPostfix(const std::string& str, const std::vector<BlackListEntry>& postfixes, const CurrentEntityInfo& currentEntityInfo)
    {
      for (const auto& entry : postfixes)
      {
        if (Fsl::StringUtil::EndsWith(str, entry.Name) && CheckMatchRequirement(entry, currentEntityInfo) )
          return true;
      }
      return false;
    }


    inline bool HasEntry(const std::string& str, const std::vector<BlackListEntry>& postfixes, const CurrentEntityInfo& currentEntityInfo)
    {
      for (const auto& entry : postfixes)
      {
        if (str == entry.Name && CheckMatchRequirement(entry, currentEntityInfo))
          return true;
      }
      return false;
    }
  }
}

#endif
