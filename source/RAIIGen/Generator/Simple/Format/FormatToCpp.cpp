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

#include <RAIIGen/Generator/Simple/Format/FormatToCpp.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/IOUtil.hpp>
#include <RAIIGen/StringHelper.hpp>
#include <RAIIGen/EnumMemberRecord.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

namespace MB
{
  using namespace Fsl;

  namespace
  {
    typedef std::pair<std::string, std::string> FormatRecord;

    const auto BYTES_PER_PIXEL_UNDEFINED = "PixelFormatFlags::BytesPerPixelUndefined";


    std::vector<FormatRecord > g_numericFormat =
    {
      FormatRecord("_UNORM", "PixelFormatFlags::NF_UNorm"),
      FormatRecord("_SNORM", "PixelFormatFlags::NF_SNorm"),
      FormatRecord("_USCALED", "PixelFormatFlags::NF_UScaled"),
      FormatRecord("_SSCALED", "PixelFormatFlags::NF_SScaled"),
      FormatRecord("_UINT", "PixelFormatFlags::NF_UInt"),
      FormatRecord("_SINT", "PixelFormatFlags::NF_SInt"),
      FormatRecord("_UFLOAT", "PixelFormatFlags::NF_UFloat"),
      FormatRecord("_SFLOAT", "PixelFormatFlags::NF_SFloat"),
      FormatRecord("_SRGB", "PixelFormatFlags::NF_Srgb"),
    };


    std::vector<FormatRecord > g_compression =
    {
      FormatRecord("BC", "PixelFormatFlags::CS_BC"),
      FormatRecord("ETC", "PixelFormatFlags::CS_ETC2"),
      FormatRecord("EAC", "PixelFormatFlags::CS_EAC"),
      FormatRecord("ASTC", "PixelFormatFlags::CS_ASTC"),
    };

    std::string TryGetFormat(const std::vector<FormatRecord >& formatRecords, const std::string& name)
    {
      for (auto itr = formatRecords.begin(); itr != formatRecords.end(); ++itr)
      {
        if (name.find(itr->first) != std::string::npos)
          return itr->second;
      }
      return std::string();
    }

    std::string TryGetNumericFormat(const std::string& name)
    {
      return TryGetFormat(g_numericFormat, name);
    }


    std::string TryGetCompressionScheme(const std::string& name)
    {
      auto index = name.find('_');
      if (index == std::string::npos)
        return std::string();

      const auto firstPart = name.substr(0,index);
      return TryGetFormat(g_compression, firstPart);
    }


    int32_t IndexOfNextValue(const std::string& name, const int32_t startIndex = 0)
    {
      int32_t index = startIndex;
      while (index < static_cast<int32_t>(name.size()) && name[index] < '0' && name[index] > '9')
        ++index;

      return (index < static_cast<int32_t>(name.size())) ? index : -1;
    }


    const uint32_t ToNum(const std::vector<char> digits, const std::size_t numDigits)
    {
      if (numDigits <= 0)
        return 0;

      uint32_t result = 0;
      uint32_t mod = 1;
      for (std::size_t i = numDigits; i > 0; --i)
      {
        assert(digits[i - 1] >= '0' && digits[i - 1] <= '9');
        result += (digits[i - 1] - '0') * mod;
        mod *= 10;
      }
      return result;
    }


    std::deque<uint32_t> TryGetValues(const std::string& name)
    {
      std::deque<uint32_t> result;
      std::size_t numDigits = 0;
      std::vector<char> currentNumber(name.size());
      for (std::size_t i = 0; i < name.size(); ++i)
      {
        if (name[i] >= '0' && name[i] <= '9')
        {
          currentNumber[numDigits] = name[i];
          ++numDigits;
        }
        else if(numDigits > 0)
        {
          result.push_back(ToNum(currentNumber, numDigits));
          numDigits = 0;
        }
      }
      if (numDigits > 0)
      {
        result.push_back(ToNum(currentNumber, numDigits));
        numDigits = 0;
      }
      return result;
    }


    std::string TryGetBytesPerPixel(const std::string& name)
    {
      auto index = name.find('_');
      if (index == std::string::npos)
        return BYTES_PER_PIXEL_UNDEFINED;

      const auto firstPart = name.substr(0, index);
      auto values = TryGetValues(firstPart);
      if( values.size() < 0 )
        return BYTES_PER_PIXEL_UNDEFINED;

      uint64_t totalBits = 0;
      for (auto itr = values.begin(); itr != values.end(); ++itr)
        totalBits += *itr;

      const auto totalBytes = totalBits / 8;
      if ((totalBits % 8) != 0)
        throw NotSupportedException("We only support formats that can be contained in full bytes");

      if( totalBytes <= 0 )
        return BYTES_PER_PIXEL_UNDEFINED;

      std::stringstream result;
      result << "PixelFormatFlags::BytesPerPixel" << totalBytes;
      return result.str();
    }


    std::string TryGetPack(const std::string& name)
    {
      if( StringUtil::EndsWith(name, "PACK8")   || StringUtil::EndsWith(name, "PACK16") ||
          StringUtil::EndsWith(name, "PACK32")  || StringUtil::EndsWith(name, "PACK64") ||
          StringUtil::EndsWith(name, "PACK128") || StringUtil::EndsWith(name, "PACK256") )
      {
        return std::string("PixelFormatFlags::UsesHostEndianness");
      }
      return std::string();
    }


    std::string GetEncodedName(const EnumMemberRecord& enumMember)
    {
      std::string removePrefix("VK_FORMAT_");
      if (! StringUtil::StartsWith(enumMember.Name, removePrefix))
      {
        std::cout << "WARNING: type did not start with: " << removePrefix << "\n";
        return enumMember.Name;
      }

      const auto newName = enumMember.Name.substr(removePrefix.size());
      const auto numericFormat = TryGetNumericFormat(newName);
      const auto compressionScheme = TryGetCompressionScheme(newName);
      const auto pack = TryGetPack(newName);

      const std::string bytesPerPixel = (compressionScheme.size() > 0 ? BYTES_PER_PIXEL_UNDEFINED : TryGetBytesPerPixel(newName));

      std::stringstream result;
      result << newName;
      result << " = ";
      result << enumMember.UnsignedValue;

      if (bytesPerPixel.size() > 0)
        result << " | " + bytesPerPixel;

      if (numericFormat.size() > 0)
        result << " | " + numericFormat;

      if (compressionScheme.size() > 0)
        result << " | " + compressionScheme;

      if (pack.size() > 0)
        result << " | " + pack;
      result << ",";

      //return result.str();

      std::stringstream result2;
      // result2 << "static_assert( ((static_cast<uint32_t>(PixelFormat::" << newName << ") & static_cast<uint32_t>(PixelFormatFlags::BIT_MASK_FORMAT_ID)) - static_cast<uint32_t>(PixelFormat::ENUM_BEGIN_RANGE)) == " << enumMember.UnsignedValue << ", \"The index did not match our assumption\");";
      //result2 << newName;
      //result2 << "PixelFormat::" << newName << ",";
      result2 << "case PixelFormat::" << newName << ":";
      //result2 << "static_assert( ((static_cast<uint32_t>(PixelFormat::" << newName << ") & static_cast<uint32_t>(PixelFormatFlags::BIT_MASK_FORMAT_ID)) - static_cast<uint32_t>(PixelFormat::ENUM_BEGIN_RANGE)) == static_cast<uint32_t>(" << enumMember.UnsignedValue << " - VK_FORMAT_BEGIN_RANGE), \"The formatId did not match the Vulkan format\");";

      return result2.str();
    }
  }

  FormatToCpp::FormatToCpp(const Capture& capture, const std::string& namespaceName, const IO::Path& templateRoot, const IO::Path& dstFileName)
  {
    auto enumDict = capture.GetEnumDict();
    const auto itrFind = enumDict.find("VkFormat");
    if (itrFind == enumDict.end())
    {
      std::cout << "VkFormat not found\n";
      return;
    }

    const auto formatRecord = itrFind->second;

    for (auto itr = formatRecord.Members.begin(); itr != formatRecord.Members.end(); ++itr)
    {
      //std::cout << itr->Name << "\n";
      std::cout << GetEncodedName(*itr) << "\n";
    }
  }


}
