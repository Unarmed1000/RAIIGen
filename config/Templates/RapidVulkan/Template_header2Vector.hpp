#ifndef RAPIDVULKAN_##CLASS_NAME!##_HPP
#define RAPIDVULKAN_##CLASS_NAME!##_HPP##VERSION_GUARD_BEGIN##
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

// ##AG_TOOL_STATEMENT##

#include <RapidVulkan/ClaimMode.hpp>
#include <RapidVulkan/CheckError.hpp>##ADDITIONAL_INCLUDES##
#include <vulkan/vulkan.h>
#include <cassert>
#include <utility>
#include <vector>

namespace RapidVulkan
{
  //! This object is movable so it can be thought of as behaving in the same was as a unique_ptr and is compatible with std containers
  class ##CLASS_NAME##
  {##CLASS_ADDITIONAL_MEMBER_VARIABLES##
    std::vector<##RESOURCE_TYPE##> ##RESOURCE_MEMBER_NAME##;
  public:
    ##CLASS_NAME##(const ##CLASS_NAME##&) = delete;
    ##CLASS_NAME##& operator=(const ##CLASS_NAME##&) = delete;

    //! @brief Move assignment operator
    ##CLASS_NAME##& operator=(##CLASS_NAME##&& other)
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

    //! @brief Move constructor
    //! Transfer ownership from other to this
    ##CLASS_NAME##(##CLASS_NAME##&& other)##MOVE_CONSTRUCTOR_MEMBER_INITIALIZATION##
    {
      // Remove the data from other##MOVE_CONSTRUCTOR_INVALIDATE_MEMBERS##
    }

    //! @brief Create a 'invalid' instance (use Reset to populate it)
    ##CLASS_NAME##()##DEFAULT_CONSTRUCTOR_MEMBER_INITIALIZATION##
    {
    }

    //! @brief Assume control of the ##CLASS_NAME## (this object becomes responsible for releasing it)
    //explicit ##CLASS_NAME##(##MEMBER_PARAMETERS##)
    //  : ##CLASS_NAME##()
    //{
    //  Reset(##MEMBER_PARAMETER_NAMES##);
    //}

##CLASS_EXTRA_CONSTRUCTORS_HEADER##

    ~##CLASS_NAME##()
    {
      Reset();
    }

    //! @brief returns the managed handle and releases the ownership.
    std::vector<##RESOURCE_TYPE##> Release() RAPIDVULKAN_FUNC_POSTFIX_WARN_UNUSED_RESULT
    {
      auto resource = std::move(##RESOURCE_MEMBER_NAME##); ##RESET_INVALIDATE_MEMBERS##
      return resource;
    }

    //! @brief Destroys any owned resources and resets the object to its default state.
    void Reset()
    {
      if (! IsValid())
        return;
##RESET_MEMBER_ASSERTIONS##

      for(std::size_t i=0; i<##RESOURCE_MEMBER_NAME##.size(); ++i)
      {
        ##DESTROY_FUNCTION##(##DESTROY_FUNCTION_ARGUMENTS##);
      }##RESET_INVALIDATE_MEMBERS##
    }

/*    
    //! @brief Destroys any owned resources and assume control of the ##CLASS_NAME## (this object becomes responsible for releasing it)
    void Reset(##MEMBER_PARAMETERS##)
    {
      if (IsValid())
        Reset();

##RESET_SET_MEMBERS_NORMAL##
    }
*/
    
##CLASS_EXTRA_RESET_METHODS_HEADER####CLASS_ADDITIONAL_GET_MEMBER_VARIABLE_METHODS##

    //! @brief Get size of the vector
    std::size_t Size() const
    {
      return ##RESOURCE_MEMBER_NAME##.size();
    }


    //! @brief Get direct access to the vector content
    const ##RESOURCE_TYPE##* Data() const
    {
      return ##RESOURCE_MEMBER_NAME##.data();
    }


    //! @brief Get the associated resource handles
    const std::vector<##RESOURCE_TYPE##>& Get() const
    {
      return ##RESOURCE_MEMBER_NAME##;
    }


    ##RESOURCE_TYPE## Get(const std::size_t arrayIndex) const
    {
      assert(arrayIndex < ##RESOURCE_MEMBER_NAME##.size());
      return ##RESOURCE_MEMBER_NAME##[arrayIndex];
    }


    //! @brief Access the resource at a given index
    ##RESOURCE_TYPE## operator[] (const std::size_t arrayIndex) const
    {
      assert(arrayIndex < ##RESOURCE_MEMBER_NAME##.size());
      return ##RESOURCE_MEMBER_NAME##[arrayIndex];
    }

    //! @brief get a pointer to the resource at the given index
    const ##RESOURCE_TYPE##* GetPointer(const std::size_t arrayIndex) const
    {
      assert(arrayIndex < ##RESOURCE_MEMBER_NAME##.size());
      return &##RESOURCE_MEMBER_NAME##[arrayIndex];
    }

    //! @brief Check if this object contains a valid resource
    inline bool IsValid() const
    {
      return ##RESOURCE_MEMBER_NAME##.size() > 0;
    }##ADDITIONAL_METHODS_HEADER##
  };
}
##VERSION_GUARD_END##
#endif
