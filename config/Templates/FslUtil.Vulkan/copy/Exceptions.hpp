#ifndef FSLUTIL_##NAMESPACE_NAME!##_EXCEPTIONS_HPP
#define FSLUTIL_##NAMESPACE_NAME!##_EXCEPTIONS_HPP
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

// ##AG_TOOL_STATEMENT##
// Auto generation template based on RapidVulkan https://github.com/Unarmed1000/RapidVulkan with permission.

#include <FslGraphics/Exceptions.hpp>
#include <string>
#include <vulkan/vulkan.h>

namespace Fsl
{
  namespace Vulkan
  {
    class VulkanException : public std::runtime_error
    {
      std::string m_fileName;
      int m_lineNumber;
    public:
      explicit VulkanException(const std::string& whatArg)
        : std::runtime_error(whatArg)
        , m_fileName()
        , m_lineNumber(0)
      {
      }

      explicit VulkanException(const std::string& whatArg, const std::string& fileName, const int lineNumber)
        : std::runtime_error(whatArg)
        , m_fileName(fileName)
        , m_lineNumber(lineNumber)
      {
      }


      std::string GetFileName() const
      {
        return m_fileName;
      }


      int GetLineNumber() const
      {
        return m_lineNumber;
      }
    };



    class VulkanErrorException : public VulkanException
    {
      VkResult m_result;
    public:
      explicit VulkanErrorException(const std::string& whatArg, const VkResult result)
        : VulkanException(whatArg)
        , m_result(result)
      {
      }

      explicit VulkanErrorException(const std::string& whatArg, const VkResult result, const std::string& fileName, const int lineNumber)
        : VulkanException(whatArg, fileName, lineNumber)
        , m_result(result)
      {
      }

      VkResult GetResult() const
      {
        return m_result;
      }
    };

    class UnsupportedVulkanPixelFormatException : public VulkanException
    {
      VkFormat m_pixelFormat;
    public:
      UnsupportedVulkanPixelFormatException(const std::string& str, const VkFormat pixelFormat)
        : VulkanException(str)
        , m_pixelFormat(pixelFormat)
      {
      }

      UnsupportedVulkanPixelFormatException(const VkFormat pixelFormat)
        : VulkanException("Unsupported pixel format")
        , m_pixelFormat(pixelFormat)
      {
      }
    };

  }
}

#endif
