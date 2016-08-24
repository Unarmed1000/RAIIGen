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

#include <FslGraphicsVulkan/Check.hpp>
#include <FslGraphics##NAMESPACE_NAME##/##CLASS_NAME##.hpp>
#include <FslGraphics##NAMESPACE_NAME##/Exceptions.hpp>
#include <cassert>
#include <cstring>
#include <utility>

namespace Fsl
{
  namespace ##NAMESPACE_NAME##
  {
    // move assignment operator
    ##CLASS_NAME##& ##CLASS_NAME##::operator=(##CLASS_NAME##&& other)
    {
      if (this != &other)
      {
        // Free existing resources then transfer the content of other to this one and fill other with default values
        if (IsValid())
          Reset();

        // Claim ownership here##MOVE_ASSIGNMENT_CLAIM_MEMBERS##

        // Remove the data from other##MOVE_ASSIGNMENT_INVALIDATE_MEMBERS##
      }
      return *this;
    }


    // Transfer ownership from other to this
    ##CLASS_NAME##::##CLASS_NAME##(##CLASS_NAME##&& other)##MOVE_CONSTRUCTOR_MEMBER_INITIALIZATION##
    {
      // Remove the data from other##MOVE_CONSTRUCTOR_INVALIDATE_MEMBERS##
    }


    // For now we implement the code here, if this turns out to be a performance problem we can move the code
    // to the header file to ensure its inlined.
    ##CLASS_NAME##::##CLASS_NAME##()##DEFAULT_CONSTRUCTOR_MEMBER_INITIALIZATION##
    {
    }


    ##CLASS_NAME##::##CLASS_NAME##(##MEMBER_PARAMETERS##)
      : ##CLASS_NAME##()
    {
      Reset(##MEMBER_PARAMETER_NAMES##);
    }


##CLASS_EXTRA_CONSTRUCTORS_SOURCE##


    ##CLASS_NAME##::~##CLASS_NAME##()
    {
      Reset();
    }


    ##RESOURCE_TYPE## ##CLASS_NAME##::Release()
    {
      const auto resource = ##RESOURCE_MEMBER_NAME##;##RESET_INVALIDATE_MEMBERS##
      return resource;
    }


    void ##CLASS_NAME##::Reset()
    {
      if (! IsValid())
        return;
##RESET_MEMBER_ASSERTIONS##

      ##DESTROY_FUNCTION##(##DESTROY_FUNCTION_ARGUMENTS##);##RESET_INVALIDATE_MEMBERS##
    }


    void ##CLASS_NAME##::Reset(##MEMBER_PARAMETERS##)
    {
      if (IsValid())
        Reset();

      ##RESET_SET_MEMBERS##
    }


##CLASS_EXTRA_RESET_METHODS_SOURCE##
  }
}
