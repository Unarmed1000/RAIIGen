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

#include <FslBase/String/StringParseUtil.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <string>
#include <fmt/format.h>

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

    explicit VersionRecord(const uint32_t major)
      : VersionRecord(major, 0, 0, 0)
    {
    }

    VersionRecord(const uint32_t major, const uint32_t minor)
      : VersionRecord(major, minor, 0, 0)
    {
    }

    VersionRecord(const uint32_t major, const uint32_t minor, const uint32_t build)
      : VersionRecord(major, minor, build, 0)
    {
    }


    explicit VersionRecord(const uint32_t major, const uint32_t minor, const uint32_t build, const uint32_t hotfix)
      : Major(major)
      , Minor(minor)
      , Build(build)
      , Hotfix(hotfix)
    {
    }

    explicit VersionRecord(const std::string& version, const bool allowMinimal = false)
      : Major(0)
      , Minor(0)
      , Build(0)
      , Hotfix(0)
    {
      using namespace Fsl;
      if (!allowMinimal)
      {
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
      else
      {
        const auto indexFirstDot = StringUtil::IndexOf(version, '.');
        const auto stringLength = static_cast<int>(version.size());
        if (indexFirstDot >= 0)
        {
          const auto indexSecondDot = StringUtil::IndexOf(version, '.', indexFirstDot + 1);
          if (indexSecondDot >= 0)
          {
            const auto indexThirdDot = StringUtil::IndexOf(version, '.', indexSecondDot + 1);
            if (indexThirdDot >= 0)
            {
              StringParseUtil::Parse(Major, version.c_str(), 0, indexFirstDot);
              StringParseUtil::Parse(Minor, version.c_str(), indexFirstDot + 1, (indexSecondDot - indexFirstDot - 1));
              StringParseUtil::Parse(Build, version.c_str(), indexSecondDot + 1, (indexThirdDot - indexSecondDot - 1));
              StringParseUtil::Parse(Hotfix, version.c_str(), indexThirdDot + 1, (stringLength - indexThirdDot - 1));
            }
            else
            {
              StringParseUtil::Parse(Major, version.c_str(), 0, indexFirstDot);
              StringParseUtil::Parse(Minor, version.c_str(), indexFirstDot + 1, (indexSecondDot - indexFirstDot - 1));
              StringParseUtil::Parse(Build, version.c_str(), indexSecondDot + 1, (stringLength - indexSecondDot - 1));
            }
          }
          else
          {
            StringParseUtil::Parse(Major, version.c_str(), 0, indexFirstDot);
            StringParseUtil::Parse(Minor, version.c_str(), indexFirstDot + 1, (stringLength - indexFirstDot - 1));
          }
        }
        else
        {
          StringParseUtil::Parse(Major, version.c_str(), 0, stringLength);
        }
      }
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

    bool operator>(const VersionRecord& rhs) const
    {
      return Major > rhs.Major || (Major == rhs.Major && Minor > rhs.Minor) || (Major == rhs.Major && Minor == rhs.Minor && Build > rhs.Build) ||
             (Major == rhs.Major && Minor == rhs.Minor && Build == rhs.Build && Hotfix > rhs.Hotfix);
    }

    bool operator<(const VersionRecord& rhs) const
    {
      return Major < rhs.Major || (Major == rhs.Major && Minor < rhs.Minor) || (Major == rhs.Major && Minor == rhs.Minor && Build < rhs.Build) ||
             (Major == rhs.Major && Minor == rhs.Minor && Build == rhs.Build && Hotfix < rhs.Hotfix);
    }

    std::string ToMinimalString() const
    {
      if (Hotfix == 0)
      {
        if (Build == 0)
        {
          if (Minor == 0)
          {
            return fmt::format("{}", Major, Minor, Build, Hotfix);
          }
          return fmt::format("{}.{}", Major, Minor);
        }
        return fmt::format("{}.{}.{}", Major, Minor, Build);
      }
      return fmt::format("{}.{}.{}.{}", Major, Minor, Build, Hotfix);
    }
  };
}
#endif
