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

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <climits>
#include <limits>
#include <FslBase/String/StringParseUtil.hpp>
#include <FslBase/Log/Log.hpp>
#include <FslBase/Exceptions.hpp>

namespace Fsl
{
  namespace
  {
    uint32_t ParseUInt32(const char* const psz, const int startIndex, const int length)
    {
      if (psz == nullptr)
        throw std::invalid_argument("psz can not be null");

      const int actualLength = strlen(psz);
      const int endIndex = startIndex + length;
      if (startIndex < 0 || length < 1 || endIndex > actualLength)
        throw std::invalid_argument("startIndex or length out of bounds");

      // Check the string contains a digit as expected
      const char* pszSrc = psz + startIndex;
      if ((pszSrc[0] >= '0' && pszSrc[0] <= '9') || (length >= 2 && pszSrc[0] == '+' && (pszSrc[1] >= '0' && pszSrc[1] <= '9')))
      {
        char* pEnd = nullptr;
        errno = 0;
        uint32_t value = strtoul(pszSrc, &pEnd, 0);
        if (value == ULONG_MAX && errno == ERANGE)
          throw OverflowException("The number is outside than the expected value range");

        if (pEnd != (pszSrc + length))
          throw FormatException("number not in the correct format");

        return value;
      }
      else
        throw FormatException("number not in the correct format");
    }


    int32_t ParseInt32(const char* const psz, const int startIndex, const int length)
    {
      if (psz == nullptr)
        throw std::invalid_argument("psz can not be null");

      const int actualLength = strlen(psz);
      const int endIndex = startIndex + length;
      if (startIndex < 0 || length < 1 || endIndex > actualLength)
        throw std::invalid_argument("startIndex or length out of bounds");

      // Check the string contains a digit as expected
      const char* pszSrc = psz + startIndex;
      if ((pszSrc[0] >= '0' && pszSrc[0] <= '9') || (length >= 2 && (pszSrc[0] == '+' || pszSrc[0] == '-') && (pszSrc[1] >= '0' && pszSrc[1] <= '9')))
      {
        char* pEnd = nullptr;
        errno = 0;

        if (length >= 32)
          throw std::invalid_argument("string is too long");
        char tmpBuffer[32];
        for (int i = 0; i < length; ++i)
          tmpBuffer[i] = psz[startIndex + i];
        tmpBuffer[length] = 0;

        const int32_t value = strtol(tmpBuffer, &pEnd, 0);
        if ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE)
          throw OverflowException("The number is outside than the expected value range");

        if (pEnd != (tmpBuffer + length))
          throw FormatException("number not in the correct format");

        return value;
      }
      else
        throw FormatException("number not in the correct format");
    }


    double ParseDouble(const char* const psz, const int startIndex, const int length)
    {
      if (psz == nullptr)
        throw std::invalid_argument("psz can not be null");

      const int actualLength = strlen(psz);
      const int endIndex = startIndex + length;
      if (startIndex < 0 || length < 1 || endIndex > actualLength)
        throw std::invalid_argument("startIndex or length out of bounds");

      // Check the string doesn't start with a white space
      const char* pszSrc = psz + startIndex;
      if (pszSrc[0] != ' ')
      {
        char* pEnd = nullptr;
        errno = 0;
        const double value = strtod(pszSrc, &pEnd);
        if (value == HUGE_VAL && errno == ERANGE)
          throw OverflowException("The number is outside than the expected value range");

        if (pEnd != (pszSrc + endIndex))
          throw FormatException("number not in the correct format");

        return value;
      }
      else
        throw FormatException("number not in the correct format");
    }


    template <typename T>
    int DoParseArray(T* pDst, const int dstLength, const char* const psz, const int startIndex, const int length)
    {
      if (psz == nullptr)
        throw std::invalid_argument("psz can not be null");

      const int actualLength = strlen(psz);
      const int endIndex = startIndex + length;
      if (startIndex < 0 || length < 1 || endIndex > actualLength)
        throw std::invalid_argument("startIndex or length out of bounds");

      const char* pszCurrent = psz + startIndex;
      const char* const pszEnd = pszCurrent + length;
      if (length < 3 || *pszCurrent != '[' || *(pszEnd - 1) != ']')
        throw FormatException("array not in the correct format");

      ++pszCurrent;
      int index = 0;
      while (pszCurrent < pszEnd && index < dstLength)
      {
        const int count = strcspn(pszCurrent, ",]");
        if (index >= dstLength)
          throw FormatException("array not in the correct format");
        const int charactersConsumed = StringParseUtil::Parse(*(pDst + index), pszCurrent, 0, count);
        assert(charactersConsumed == count);
        pszCurrent += charactersConsumed + 1;
        ++index;
      }

      if (pszCurrent != pszEnd)
        throw FormatException("array not in the correct format");
      return index;
    }
  }

  int StringParseUtil::Parse(bool& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(uint8_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(int8_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(uint16_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(int16_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(uint32_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(int32_t& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  // int StringParseUtil::Parse(uint64_t& rResult, const char*const psz)
  //{
  // if (psz == nullptr)
  //  throw std::invalid_argument("psz can not be null");
  // return Parse(rResult, psz, 0, strlen(psz));
  //}


  // int StringParseUtil::Parse(int64_t& rResult, const char*const psz)
  //{
  // if (psz == nullptr)
  //  throw std::invalid_argument("psz can not be null");
  // return Parse(rResult, psz, 0, strlen(psz));
  //}


  int StringParseUtil::Parse(float& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(double& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(Point2& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(Rectangle& rResult, const char* const psz)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return Parse(rResult, psz, 0, strlen(psz));
  }


  int StringParseUtil::Parse(bool& rResult, const char* const psz, const int startIndex, const int length)
  {
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");

    const int actualLength = strlen(psz);
    const char* pszSrc = psz + startIndex;
    int charactersConsumed = 0;

    if (startIndex < 0 || length < 1 || (startIndex + length) > actualLength)
      throw std::invalid_argument("startIndex or length out of bounds");

    if (length == 4 && strncmp(pszSrc, "true", 4) == 0)
    {
      rResult = true;
      charactersConsumed = 4;
    }
    else if (length == 1 && strncmp(pszSrc, "1", 1) == 0)
    {
      rResult = true;
      charactersConsumed = 1;
    }
    else if (length == 1 && strncmp(pszSrc, "0", 1) == 0)
    {
      rResult = false;
      charactersConsumed = 1;
    }
    else if (length == 5 && strncmp(pszSrc, "false", 5) == 0)
    {
      rResult = false;
      charactersConsumed = 5;
    }
    return charactersConsumed;
  }


  int StringParseUtil::Parse(uint8_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    const uint32_t result = ParseUInt32(psz, startIndex, length);
    if (result > std::numeric_limits<uint8_t>::max())
      throw OverflowException("overflow");
    rResult = static_cast<uint8_t>(result);
    return length;
  }


  int StringParseUtil::Parse(int8_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    const int32_t result = ParseInt32(psz, startIndex, length);
    if (result < std::numeric_limits<int8_t>::min() || result > std::numeric_limits<int8_t>::max())
      throw OverflowException("overflow");
    rResult = static_cast<int8_t>(result);
    return length;
  }


  int StringParseUtil::Parse(uint16_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    const uint32_t result = ParseUInt32(psz, startIndex, length);
    if (result > std::numeric_limits<uint16_t>::max())
      throw OverflowException("overflow");
    rResult = static_cast<uint16_t>(result);
    return length;
  }


  int StringParseUtil::Parse(int16_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    const int32_t result = ParseInt32(psz, startIndex, length);
    if (result < std::numeric_limits<int16_t>::min() || result > std::numeric_limits<int16_t>::max())
      throw OverflowException("overflow");
    rResult = static_cast<int16_t>(result);
    return length;
  }


  int StringParseUtil::Parse(uint32_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    rResult = ParseUInt32(psz, startIndex, length);
    return length;
  }


  int StringParseUtil::Parse(int32_t& rResult, const char* const psz, const int startIndex, const int length)
  {
    rResult = ParseInt32(psz, startIndex, length);
    return length;
  }


  // int StringParseUtil::Parse(uint64_t& rResult, const char*const psz, const int startIndex, const int length)
  //{
  //  throw NotImplementedException();
  //}


  // int StringParseUtil::Parse(int64_t& rResult, const char*const psz, const int startIndex, const int length)
  //{
  //  throw NotImplementedException();
  //}


  int StringParseUtil::Parse(float& rResult, const char* const psz, const int startIndex, const int length)
  {
    const double result = ParseDouble(psz, startIndex, length);
    if (result < std::numeric_limits<float>::lowest() || result > std::numeric_limits<float>::max())
      throw OverflowException("overflow");
    rResult = static_cast<float>(result);
    return length;
  }


  int StringParseUtil::Parse(double& rResult, const char* const psz, const int startIndex, const int length)
  {
    rResult = ParseDouble(psz, startIndex, length);
    return length;
  }


  int StringParseUtil::Parse(Point2& rResult, const char* const psz, const int startIndex, const int length)
  {
    int32_t values[2];
    StringParseArrayResult res = ParseArray(values, 2, psz, startIndex, length);
    if (res.ArrayEntries != 2)
      throw FormatException("Point2 not in the correct format");
    rResult = Point2(values[0], values[1]);
    return res.CharactersConsumed;
  }


  int StringParseUtil::Parse(Rectangle& rResult, const char* const psz, const int startIndex, const int length)
  {
    int32_t values[4];
    StringParseArrayResult res = ParseArray(values, 4, psz, startIndex, length);
    if (res.ArrayEntries != 4)
      throw FormatException("Rectangle not in the correct format");
    rResult = Rectangle(values[0], values[1], values[2], values[3]);
    return res.CharactersConsumed;
  }


  const StringParseArrayResult StringParseUtil::ParseArray(bool* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(uint8_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(int8_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(uint16_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(int16_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(uint32_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(int32_t* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  // const StringParseArrayResult StringParseUtil::ParseArray(uint64_t* pDst, const int dstLength, const char*const psz)
  //{
  // if (pDst == nullptr)
  //  throw std::invalid_argument("pDst can not be null");
  // if (dstLength < 0)
  //  throw std::invalid_argument("dstLength can not be < 0");
  // if (psz == nullptr)
  //  throw std::invalid_argument("psz can not be null");
  // return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  //}


  // const StringParseArrayResult StringParseUtil::ParseArray(int64_t* pDst, const int dstLength, const char*const psz)
  //{
  // if (pDst == nullptr)
  //  throw std::invalid_argument("pDst can not be null");
  // if (dstLength < 0)
  //  throw std::invalid_argument("dstLength can not be < 0");
  // if (psz == nullptr)
  //  throw std::invalid_argument("psz can not be null");
  // return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  //}


  const StringParseArrayResult StringParseUtil::ParseArray(float* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  const StringParseArrayResult StringParseUtil::ParseArray(double* pDst, const int dstLength, const char* const psz)
  {
    if (pDst == nullptr)
      throw std::invalid_argument("pDst can not be null");
    if (dstLength < 0)
      throw std::invalid_argument("dstLength can not be < 0");
    if (psz == nullptr)
      throw std::invalid_argument("psz can not be null");
    return ParseArray(pDst, dstLength, psz, 0, strlen(psz));
  }


  StringParseArrayResult StringParseUtil::ParseArray(bool* pDst, const int dstLength, const char* const psz, const int startIndex, const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(uint8_t* pDst, const int dstLength, const char* const psz, const int startIndex,
                                                     const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(int8_t* pDst, const int dstLength, const char* const psz, const int startIndex, const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(uint16_t* pDst, const int dstLength, const char* const psz, const int startIndex,
                                                     const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(int16_t* pDst, const int dstLength, const char* const psz, const int startIndex,
                                                     const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(uint32_t* pDst, const int dstLength, const char* const psz, const int startIndex,
                                                     const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(int32_t* pDst, const int dstLength, const char* const psz, const int startIndex,
                                                     const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  // StringParseArrayResult StringParseUtil::ParseArray(uint64_t* pDst, const int dstLength, const char*const psz, const int startIndex, const int
  // length)
  //{
  // const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
  // return StringParseArrayResult(length, arrayEntries);
  //}


  // StringParseArrayResult StringParseUtil::ParseArray(int64_t* pDst, const int dstLength, const char*const psz, const int startIndex, const int
  // length)
  //{
  // const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
  // return StringParseArrayResult(length, arrayEntries);
  //}


  StringParseArrayResult StringParseUtil::ParseArray(float* pDst, const int dstLength, const char* const psz, const int startIndex, const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  StringParseArrayResult StringParseUtil::ParseArray(double* pDst, const int dstLength, const char* const psz, const int startIndex, const int length)
  {
    const int arrayEntries = DoParseArray(pDst, dstLength, psz, startIndex, length);
    return StringParseArrayResult(length, arrayEntries);
  }


  bool StringParseUtil::TryParse(int32_t& rValue, const char* const psz, const int startIndex, const int length, const int32_t radix)
  {
    rValue = 0;
    if (psz == nullptr)
    {
      FSLLOG_DEBUG_WARNING("psz can not be null");
      return false;
    }

    const int actualLength = strlen(psz);
    const int endIndex = startIndex + length;
    if (startIndex < 0 || length < 1 || endIndex > actualLength)
    {
      FSLLOG_DEBUG_WARNING("startIndex or length out of bounds");
      return false;
    }

    // Check the string contains a digit as expected
    const char* pszSrc = psz + startIndex;
    if ((pszSrc[0] >= '0' && pszSrc[0] <= '9') || (length >= 2 && (pszSrc[0] == '+' || pszSrc[0] == '-') && (pszSrc[1] >= '0' && pszSrc[1] <= '9')))
    {
      char* pEnd = nullptr;
      errno = 0;

      if (length >= 32)
      {
        FSLLOG_DEBUG_WARNING("string is too long");
        return false;
      }
      char tmpBuffer[32];
      for (int i = 0; i < length; ++i)
        tmpBuffer[i] = psz[startIndex + i];
      tmpBuffer[length] = 0;

      const int32_t value = strtol(tmpBuffer, &pEnd, radix);
      if ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE)
        return false;

      if (pEnd != (tmpBuffer + length))
        return false;

      rValue = value;
      return true;
    }
    else
      return false;
  }

}
