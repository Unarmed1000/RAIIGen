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

#include <FslUtil/##NAMESPACE_NAME##/Util.hpp>
#include <FslUtil/##NAMESPACE_NAME##/DebugStrings.hpp>
#include <sstream>

namespace Fsl
{
  namespace Vulkan
  {
    std::string Util::ToNiceMessage(const std::string& message, const VkResult errorCode)
    {
      std::stringstream stream;
      stream << message << " failed with error code " << Debug::ErrorCodeToString(errorCode) << " (" << errorCode <<")";
      return stream.str();
    }


    std::string Util::ToNiceMessage(const std::string& message, const VkResult errorCode, const std::string& fileName, const int lineNumber)
    {
      std::stringstream stream;
      stream << message << " failed with error code " << Debug::ErrorCodeToString(errorCode) << " (" << errorCode <<") at " << fileName << "(" << lineNumber << ")";
      return stream.str();
    }
  }
}
