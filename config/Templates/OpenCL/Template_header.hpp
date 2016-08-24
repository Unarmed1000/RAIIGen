#ifndef FSLUTIL##NAMESPACE_NAME!##_##CLASS_NAME!##_HPP
#define FSLUTIL##NAMESPACE_NAME!##_##CLASS_NAME!##_HPP
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

// Make sure Common.hpp is the first include file (to make the error message as helpful as possible when disabled)
#include <FslUtil##NAMESPACE_NAME##/Common.hpp>
#include <FslUtil##NAMESPACE_NAME##/CLValues.hpp>##ADDITIONAL_INCLUDES##
#include <FslBase/Attributes.hpp>
#include <CL/cl.h>

namespace Fsl
{
  namespace ##NAMESPACE_NAME##
  {
    // This object is movable so it can be thought of as behaving in the same was as a unique_ptr and is compatible with std containers
    class ##CLASS_NAME##
    {##CLASS_ADDITIONAL_MEMBER_VARIABLES##
      ##RESOURCE_TYPE## ##RESOURCE_MEMBER_NAME##;
    public:
      ##CLASS_NAME##(const ##CLASS_NAME##&) = delete;
      ##CLASS_NAME##& operator=(const ##CLASS_NAME##&) = delete;
    
      //! @brief Move assignment operator
      ##CLASS_NAME##& operator=(##CLASS_NAME##&& other);
      //! @brief Move constructor
      ##CLASS_NAME##(##CLASS_NAME##&& other);

      //! @brief Create a 'invalid' instance (use Reset to populate it)
      ##CLASS_NAME##();

      //! @brief Assume control of the ##CLASS_NAME## (this object becomes responsible for releasing it)
      explicit ##CLASS_NAME##(##MEMBER_PARAMETERS##);
      
##CLASS_EXTRA_CONSTRUCTORS_HEADER##
      
      ~##CLASS_NAME##();

      //! @brief returns the managed handle and releases the ownership.
      ##RESOURCE_TYPE## Release() FSL_FUNC_POSTFIX_WARN_UNUSED_RESULT;

      //! @brief Destroys any owned resources and resets the object to its default state.
      void Reset();

      //! @brief Destroys any owned resources and assume control of the ##CLASS_NAME## (this object becomes responsible for releasing it)
      void Reset(##MEMBER_PARAMETERS##);
      
##CLASS_EXTRA_RESET_METHODS_HEADER####CLASS_ADDITIONAL_GET_MEMBER_VARIABLE_METHODS##

      //! @brief Get the associated resource handle
      ##RESOURCE_TYPE## Get() const
      {
        return ##RESOURCE_MEMBER_NAME##;
      }

      //! @brief Check if this object contains a valid resource
      inline bool IsValid() const
      {
        return ##RESOURCE_MEMBER_NAME## != ##DEFAULT_VALUE##;
      }
    };
  }
}

#endif
