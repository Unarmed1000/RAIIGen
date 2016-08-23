/****************************************************************************************************************************************************
* Copyright (c) 2014 Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    * Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*
*    * Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*
*    * Neither the name of the Freescale Semiconductor, Inc. nor the names of
*      its contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
****************************************************************************************************************************************************/

#include <FslBase/Exceptions.hpp>
#include <FslBase/String/UTF8String.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include "../System/Platform/Platform.hpp"

namespace Fsl
{
  namespace
  {
    const char UTF8_CHAR_MIN = 0;
    const char UTF8_CHAR_MAX = 127;


    const bool IsValidUTF8(const char*const psz, const int32_t startIndex, const int32_t length)
    {
      // FIX: add UTF8 validation check
      return psz != nullptr;
    }

  }


  // move assignment operator
  UTF8String& UTF8String::operator=(UTF8String&& other)
  {
    if (this != &other)
    {
      m_content = std::move(other.m_content);
    }
    return *this;
  }


  // Transfer ownership from other to this
  UTF8String::UTF8String(UTF8String&& other)
    : m_content(std::move(other.m_content))
  {
  }


  UTF8String::UTF8String()
    : m_content()
  {

  }


  UTF8String::UTF8String(const std::string& str)
    : m_content(str)
  {
    if (! IsValidUTF8(str.c_str(), 0, str.size()))
      throw InvalidUTF8StringException("The supplied UTF8 string is not valid");
  }


  UTF8String::UTF8String(const char*const psz)
    : m_content(psz != nullptr ? psz : "")
  {
    if (!IsValidUTF8(psz, 0, std::strlen(psz)))
      throw InvalidUTF8StringException("The supplied UTF8 string is not valid");
  }


  UTF8String::UTF8String(const std::string& str, const int32_t startIndex, const int32_t length)
    : m_content()
  {
    if (startIndex < 0 || static_cast<size_t>(startIndex) >= str.size())
      throw std::invalid_argument("Start index was out of bounds");
    if (length < 0 || (static_cast<size_t>(startIndex)+static_cast<size_t>(length)) > str.size())
    {
      if ( length < 0)
        throw std::invalid_argument("length can not be negative");
      else
        throw std::invalid_argument("startIndex + length was out of bounds");
    }
    if (!IsValidUTF8(str.c_str(), startIndex, length))
      throw InvalidUTF8StringException("The supplied UTF8 string is not valid");
    m_content.assign(str, startIndex, length);
  }


  UTF8String::UTF8String(const UTF8String& str, const int32_t startIndex, const int32_t length)
  {
    if (startIndex < 0 || static_cast<size_t>(startIndex) >= str.m_content.size())
      throw std::invalid_argument("Start index was out of bounds");
    if (length < 0 || (static_cast<size_t>(startIndex)+static_cast<size_t>(length)) > str.m_content.size())
    {
      if (length < 0)
        throw std::invalid_argument("length can not be negative");
      else
        throw std::invalid_argument("startIndex + length was out of bounds");
    }
    if (!IsValidUTF8(m_content.c_str(), startIndex, length))
      throw InvalidUTF8StringException("The substring is not valid utf8");
    m_content.assign(str.m_content, startIndex, length);
  }


  UTF8String::~UTF8String()
  {

  }


  void UTF8String::Clear()
  {
    m_content.clear();
  }


  bool UTF8String::IsEmpty() const
  {
    return m_content.size() == 0;
  }


  int32_t UTF8String::GetByteSize() const
  {
    return static_cast<int32_t>(m_content.size());
  }


  bool UTF8String::Contains(const char ch) const
  {
    if (!(ch >= UTF8_CHAR_MIN && ch <= UTF8_CHAR_MAX))
      throw std::invalid_argument("ch should be in the range 0 to 127");
    return std::find(m_content.begin(), m_content.end(), ch) != m_content.end();
  }


  bool UTF8String::Contains(const UTF8String& str) const
  {
    return std::search(m_content.begin(), m_content.end(), str.m_content.begin(), str.m_content.end()) != m_content.end();
  }


  bool UTF8String::StartsWith(const char ch) const
  {
    if (!(ch >= UTF8_CHAR_MIN && ch <= UTF8_CHAR_MAX))
      throw std::invalid_argument("ch should be in the range 0 to 127");
    return (m_content.size() > 0 ? m_content.front() == ch : false);
  }


  bool UTF8String::StartsWith(const UTF8String& str) const
  {
    return StringUtil::StartsWith(m_content, str.m_content);
  }


  bool UTF8String::EndsWith(const char ch) const
  {
    if (!(ch >= UTF8_CHAR_MIN && ch <= UTF8_CHAR_MAX))
      throw std::invalid_argument("ch should be in the range 0 to 127");
    return (m_content.size() > 0 ? m_content.back() == ch : false);
  }


  bool UTF8String::EndsWith(const UTF8String& path) const
  {
    if (m_content.size() < path.m_content.size())
      return false;
    return (m_content.compare(m_content.size() - path.m_content.size(), path.m_content.size(), path.m_content) == 0);
  }

  void UTF8String::Replace(const char from, const char to)
  {
    if (!(from >= UTF8_CHAR_MIN && from <= UTF8_CHAR_MAX))
      throw std::invalid_argument("from char should be in the range 0 to 127");
    if (!(to >= UTF8_CHAR_MIN && to <= UTF8_CHAR_MAX))
      throw std::invalid_argument("from to should be in the range 0 to 127");

    std::replace(m_content.begin(), m_content.end(), from, to);
  }


  int32_t UTF8String::IndexOf(const char ch) const
  {
    if (!(ch >= UTF8_CHAR_MIN && ch <= UTF8_CHAR_MAX))
      throw std::invalid_argument("ch should be in the range 0 to 127");
    return StringUtil::IndexOf(m_content, ch);
  }


  int32_t UTF8String::LastIndexOf(const char ch) const
  {
    if (!(ch >= UTF8_CHAR_MIN && ch <= UTF8_CHAR_MAX))
      throw std::invalid_argument("ch should be in the range 0 to 127");
    const std::size_t index = m_content.rfind(ch);
    return (index != std::string::npos ? index : -1);
  }


  std::string UTF8String::ToAsciiString() const
  {
    // Slow but it works
    std::stringstream stream;
    std::string::const_iterator itr = m_content.begin();
    std::string::const_iterator itrEnd = m_content.end();
    bool bIsFirst = true;
    while (itr != itrEnd)
    {
      const uint32_t value = *itr;
      if (value < UTF8_CHAR_MAX)
      {
        stream << (char)value;
        bIsFirst = true;
      }
      else if (bIsFirst)
      {
        stream << '?';
        bIsFirst = false;
      }
      ++itr;
    }
    return stream.str();
  }


  std::wstring UTF8String::ToWString() const
  {
    return Platform::UTF8ToWString(m_content);
  }


  void UTF8String::Reset(const char*const psz, const int32_t startIndex, const int32_t length)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    if (startIndex < 0 || length < 0)
      throw std::invalid_argument("index or length can not be negative");
    if (!IsValidUTF8(psz, startIndex, length))
      throw InvalidUTF8StringException("The substring is not valid utf8");

    m_content.assign(psz + startIndex, length);
  }

}
