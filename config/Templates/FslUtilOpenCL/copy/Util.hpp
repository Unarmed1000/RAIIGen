#ifndef FSLUTIL##NAMESPACE_NAME!##_UTIL_HPP
#define FSLUTIL##NAMESPACE_NAME!##_UTIL_HPP
/****************************************************************************************************************************************************
* Copyright (c) 2016 Freescale Semiconductor, Inc.
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
  
// Based on RapidOpenCL https://github.com/Unarmed1000/RapidOpenCL with permission.

#include <FslUtil##NAMESPACE_NAME##/Exceptions.hpp>
#include <cassert>
#include <string>
#include <CL/cl.h>

namespace Fsl
{
  namespace OpenCL
  {
    class Util
    {
    public:
      static inline void Check(const cl_int errorCode)
      {
        if (errorCode != CL_SUCCESS)
        {
          throw OpenCLErrorException("Unknown", errorCode);
        }
      }

      static inline void Check(const cl_int errorCode, const char*const pszMessage)
      {
        if (errorCode != CL_SUCCESS)
        {
          assert(pszMessage != nullptr);
          throw OpenCLErrorException(ToNiceMessage(pszMessage, errorCode), errorCode);
        }
      }

      
      static inline void Check(const cl_int errorCode, const std::string& message)
      {
        if (errorCode != CL_SUCCESS)
        {
          throw OpenCLErrorException(ToNiceMessage(message, errorCode), errorCode);
        }
      }

      static inline void Check(const cl_int errorCode, const char*const pszMessage, const char*const pszFileName, const int lineNumber)
      {
        if (errorCode != CL_SUCCESS)
        {
          assert(pszMessage != nullptr);
          assert(pszFileName != nullptr);
          throw OpenCLErrorException(ToNiceMessage(pszMessage, errorCode, pszFileName, lineNumber), errorCode, pszFileName, lineNumber);
        }
      }
      
      static inline void Check(const cl_int errorCode, const char*const pszMessage, const std::string& fileName, const int lineNumber)
      {
        if (errorCode != CL_SUCCESS)
        {
          assert(pszMessage != nullptr);
          throw OpenCLErrorException(ToNiceMessage(pszMessage, errorCode, fileName, lineNumber), errorCode, fileName, lineNumber);
        }
      }

      
      static inline void Check(const cl_int errorCode, const std::string& message, const char*const pszFileName, const int lineNumber)
      {
        if (errorCode != CL_SUCCESS)
        {
          assert(pszFileName != nullptr);
          throw OpenCLErrorException(ToNiceMessage(message, errorCode, pszFileName, lineNumber), errorCode, pszFileName, lineNumber);
        }
      }
      
      static inline void Check(const cl_int errorCode, const std::string& message, const std::string& fileName, const int lineNumber)
      {
        if (errorCode != CL_SUCCESS)
        {
          throw OpenCLErrorException(ToNiceMessage(message, errorCode, fileName, lineNumber), errorCode, fileName, lineNumber);
        }
      }
    private:
      static std::string ToNiceMessage(const std::string& message, const cl_int errorCode);
      static std::string ToNiceMessage(const std::string& message, const cl_int errorCode, const std::string& fileName, const int lineNumber);
    };
  }
}

#endif
