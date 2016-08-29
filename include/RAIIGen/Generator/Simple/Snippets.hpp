#ifndef MB_GENERATOR_SIMPLE_SNIPPETS_HPP
#define MB_GENERATOR_SIMPLE_SNIPPETS_HPP
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

#include <string>

namespace MB
{
    struct Snippets
    {
      std::string ConstructorMemberInitialization;
      std::string CreateConstructorHeader;
      std::string CreateConstructorSource;
      std::string ResetSetMemberVariable;
      std::string ResetInvalidateMemberVariable;
      std::string ResetMemberAssertion;
      std::string ResetMemberHeader;
      std::string ResetMemberSource;
      std::string ResetParamValidation;
      std::string MoveAssignmentClaimMember;
      std::string MoveAssignmentInvalidateMember;
      std::string MoveConstructorInvalidateMember;
      std::string MoveConstructorClaimMember;
      std::string HandleClassName;
      std::string CreateVoidConstructorHeader;
      std::string CreateVoidConstructorSource;
      std::string ResetVoidMemberHeader;
      std::string ResetVoidMemberSource;
      std::string ResetUnrollMemberHeader;
      std::string ResetUnrollMemberSource;
      std::string ResetUnrollStructVariable;
      std::string DefaultValueMod;
      std::string IncludeResetMode;
      std::string UnrolledWrap;
    };
}
#endif
