#ifndef MB_VERSIONRECORD_HPP
#define MB_VERSIONRECORD_HPP
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

#include <string>
#include <FslBase/String/StringParseUtil.hpp>
#include <FslBase/String/StringUtil.hpp>

namespace MB
{
  struct VersionRecord
  {
    uint32_t Major;
    uint32_t Minor;
    uint32_t Build;
    uint32_t Hotfix;

    VersionRecord()
      : Major(0)
      , Minor(0)
      , Build(0)
      , Hotfix(0)
    {
    }

    explicit VersionRecord(const std::string& version)
      : Major(0)
      , Minor(0)
      , Build(0)
      , Hotfix(0)
    {
      using namespace Fsl;
      const auto indexFirstDot = StringUtil::IndexOf(version, '.');
      if (indexFirstDot < 0)
        throw std::invalid_argument("Version format is incorrect");
      const auto indexSecondDot = StringUtil::IndexOf(version, '.', indexFirstDot + 1);
      if (indexSecondDot < 0)
        throw std::invalid_argument("Version format is incorrect");
      const auto indexThirdDot = StringUtil::IndexOf(version, '.', indexSecondDot + 1);
      if (indexThirdDot < 0)
        throw std::invalid_argument("Version format is incorrect");
      if (StringUtil::IndexOf(version, '.', indexThirdDot + 1) >= 0)
        throw std::invalid_argument("Version format is incorrect");


      const auto stringLength = static_cast<int>(version.size());

      StringParseUtil::Parse(Major, version.c_str(), 0, indexFirstDot);
      StringParseUtil::Parse(Minor, version.c_str(), indexFirstDot + 1, (indexSecondDot - indexFirstDot - 1));
      StringParseUtil::Parse(Build, version.c_str(), indexSecondDot + 1, (indexThirdDot - indexSecondDot - 1));
      StringParseUtil::Parse(Hotfix, version.c_str(), indexThirdDot + 1, (stringLength - indexThirdDot - 1));
    }


    void Clear()
    {
      *this = VersionRecord();
    }


    bool operator==(const VersionRecord& rhs) const
    {
      return Major == rhs.Major && Minor == rhs.Minor && Build == rhs.Build;
    }

    bool operator!=(const VersionRecord& rhs) const
    {
      return !(*this == rhs);
    }
  };
}
#endif
