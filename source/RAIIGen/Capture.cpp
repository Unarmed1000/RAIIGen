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

#include <clang-c/Index.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <FslBase/Exceptions.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/ClangUtil.hpp>
#include <RAIIGen/StringHelper.hpp>

using namespace Fsl;

namespace MB
{
  using namespace ClangUtil;

  namespace
  {


    struct TypeInfo
    {
      std::string Name;
      CXType Type;
      CXType OriginalType;

      TypeInfo()
        : Name()
        , Type{}
        , OriginalType{}
      {
        Type.kind = CXType_Invalid;
        OriginalType.kind = CXType_Invalid;
      }


      TypeInfo(const std::string& name, const CXType& baseType, const CXType& originalType)
        : Name(name)
        , Type(baseType)
        , OriginalType(originalType)
      {
      }
    };


    TypeInfo GetTypeInfo(const CXType type, CXType originalType)
    {
      switch (type.kind)
      {
      case CXType_Invalid:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Invalid");
      case CXType_Unexposed:
        std::cout << "WARNING: CXType_Unexposed not properly supported\n";
        return TypeInfo(GetTypeSpelling(type), type, originalType);
      case CXType_Void:
        return TypeInfo("void", type, originalType);
      case CXType_Bool:
        return TypeInfo("bool", type, originalType);
      case CXType_Char_U:
        return TypeInfo("char", type, originalType);
      case CXType_UChar:
        return TypeInfo("unsigned char", type, originalType);
      case CXType_Char16:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Char16");
      case CXType_Char32:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Char32");
      case CXType_UShort:
        return TypeInfo("unsigned short", type, originalType);
      case CXType_UInt:
        return TypeInfo("unsigned int", type, originalType);
      case CXType_ULong:
        return TypeInfo("unsigned long", type, originalType);
      case CXType_ULongLong:
        return TypeInfo("unsigned long long", type, originalType);
      case CXType_UInt128:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_UInt128");
      case CXType_Char_S:
        return TypeInfo("char", type, originalType);
      case CXType_SChar:
        return TypeInfo("signed char", type, originalType);
      case CXType_WChar:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_WChar");
      case CXType_Short:
        return TypeInfo("short", type, originalType);
      case CXType_Int:
        return TypeInfo("int", type, originalType);
      case CXType_Long:
        return TypeInfo("long", type, originalType);
      case CXType_LongLong:
        return TypeInfo("long long", type, originalType);
      case CXType_Int128:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Int128");
      case CXType_Float:
        return TypeInfo("float", type, originalType);
      case CXType_Double:
        return TypeInfo("double", type, originalType);
      case CXType_LongDouble:
        return TypeInfo("long double", type, originalType);
      case CXType_NullPtr:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_NullPtr");
      case CXType_Overload:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Overload");
      case CXType_Dependent:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Dependent");
      case CXType_Complex:
        throw std::runtime_error("GetTypeName() failed to get type name CXType_Complex");
      case CXType_Pointer:
        return GetTypeInfo(clang_getPointeeType(type), originalType);
      case CXType_BlockPointer:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_BlockPointer");
      case CXType_LValueReference:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_LValueReference");
      case CXType_RValueReference:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_RValueReference");
      case CXType_Record:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Record");
      case CXType_Enum:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Enum");
      case CXType_Typedef:
      {
        // lookup the type
        const auto actualTypeCursor = clang_getTypeDeclaration(type);
        if (actualTypeCursor.kind == CXCursor_NoDeclFound)
          throw std::runtime_error("GetTypeName() failed to get type name");
        return TypeInfo(GetCursorSpelling(actualTypeCursor), type, originalType);
      }
      case CXType_ObjCInterface:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_ObjCInterface");
      case CXType_ObjCObjectPointer:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_ObjCObjectPointer");
      case CXType_FunctionNoProto:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_FunctionNoProto");
      case CXType_FunctionProto:
        std::cout << "WARNING: CXType_FunctionProto not properly supported\n";
        return TypeInfo("**CXType_FunctionProto**", type, originalType);
      case CXType_ConstantArray:
        std::cout << "WARNING: CXType_ConstantArray not properly supported\n";
        return TypeInfo("**CXType_ConstantArray**", type, originalType);
      case CXType_Vector:
        std::cout << "WARNING: CXType_Vector not properly supported\n";
        return TypeInfo("**CXType_Vector**", type, originalType);
      case CXType_IncompleteArray:
        std::cout << "WARNING: CXType_IncompleteArray not properly supported\n";
        return TypeInfo("**CXType_IncompleteArray**", type, originalType);
      case CXType_VariableArray:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_VariableArray");
      case CXType_DependentSizedArray:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_DependentSizedArray");
      case CXType_MemberPointer:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_MemberPointer");
      case CXType_Auto:
        throw std::runtime_error("GetTypeName() failed to get type name for CXType_Auto");
      default:
        throw std::runtime_error("GetTypeName() failed to get type name");
      }
    }


    TypeInfo GetTypeInfo(const CXType type)
    {
      return GetTypeInfo(type, type);
    }


    TypeRecord GetType(const CXType type)
    {

      TypeRecord typeRecord;
      typeRecord.FullTypeString = GetTypeSpelling(type);

      const auto typeInfo = GetTypeInfo(type);
      typeRecord.Name = typeInfo.Name;
      typeRecord.IsConstQualified = clang_isConstQualifiedType(typeInfo.Type) != 0;

      const auto canonicalType = clang_getCanonicalType(typeInfo.Type);
      if (canonicalType.kind == CXType_Record)
      {
        typeRecord.IsStruct = true;

        //auto cur = clang_getTypeDeclaration(canonicalType);
      }

      if (typeInfo.Type.kind == CXType_Unexposed && canonicalType.kind == CXType_FunctionProto )
      {
        typeRecord.IsFunctionPointer = true;
      }


      if (type.kind == CXType_Pointer)
      {
        typeRecord.IsPointer = true;
        auto pointerType = clang_getPointeeType(type);
        //if (pointerType.kind == CXType_Pointer)
        //  typeRecord.IsPointerPointer = true;
      }

      return typeRecord;
    }


    TypeRecord GetType(const CXCursor cursor)
    {
      return GetType(clang_getCursorType(cursor));
    }



    std::string EnforcePrefix(const TypeRecord& type, const std::string& name)
    {
      if (name.size() <= 1 || !type.IsPointer)
        return name;
      
      if (name[0] == 'p' && CaseUtil::IsUpperCase(name[1]))
        return name;

      return "p" + CaseUtil::UpperCaseFirstCharacter(name);
    }


    std::string TypeToBasicArgumentName(const TypeRecord& type, const std::string& typeNamePrefix)
    {
      if (type.Name.find(typeNamePrefix) != 0)
      {
        if (type.Name == "size_t")
          return "size";
        return type.Name;
      }
      ;
      return CaseUtil::LowerCaseFirstCharacter(type.Name.substr(typeNamePrefix.size()));
    }


    std::string TypeToArgumentName(const TypeRecord& type, const std::string& typeNamePrefix)
    {
      return StringHelper::EnforceLowerCamelCaseNameStyle(EnforcePrefix(type, TypeToBasicArgumentName(type, typeNamePrefix)));
    }


    ParameterRecord GetParameter(const CXCursor cursor, const std::string& typeNamePrefix, const std::vector<FunctionParameterTypeOverride>& functionParameterTypeOverrides)
    {
      ParameterRecord param;
      param.Name = StringHelper::EnforceLowerCamelCaseNameStyle(GetCursorSpelling(cursor));
      param.Type = GetType(cursor);
      param.ArgumentName = StringHelper::EnforceLowerCamelCaseNameStyle(GetCursorDisplayName(cursor));
      
      if (param.Name.size() <= 0)
      {
        param.Name = TypeToArgumentName(param.Type, typeNamePrefix);
        param.IsAutoGeneratedName = true;
      }
      if (param.ArgumentName.size() <= 0)
      {
        param.ArgumentName = TypeToArgumentName(param.Type, typeNamePrefix);
        param.IsAutoGeneratedArgumentName = true;
      }
      
      return param;
    }


    void HandleParamNameOverride(ParameterRecord& rParam, const std::vector<FunctionParameterNameOverride>& functionParameterNameOverrides, const std::string& currentfunctionName, const unsigned int parameterIndex)
    {
      for (auto itr = functionParameterNameOverrides.begin(); itr != functionParameterNameOverrides.end(); ++itr)
      {
        if (itr->FunctionName == currentfunctionName && itr->ParameterIndex == parameterIndex)
        {
          if (itr->ParameterOldName != rParam.ArgumentName)
            throw UsageErrorException(std::string("The argument name '"+ rParam.ArgumentName + "' does not match the expected '" + itr->ParameterOldName + "' name"));

          rParam.Name = itr->ParameterNewName;
          rParam.ArgumentName = itr->ParameterNewName;
        }
      }
    }

    void HandleParamTypeOverride(ParameterRecord& rParam, const std::vector<FunctionParameterTypeOverride>& functionParameterTypeOverrides, const std::string& currentfunctionName, const unsigned int parameterIndex)
    {
      for (auto itr = functionParameterTypeOverrides.begin(); itr != functionParameterTypeOverrides.end(); ++itr)
      {
        if (itr->FunctionName == currentfunctionName && itr->ParameterIndex == parameterIndex)
        {
          if (itr->ParameterOldType != rParam.Type.FullTypeString)
            throw UsageErrorException(std::string("The argument type '" + rParam.Type.FullTypeString + "' does not match the expected '" + itr->ParameterOldType + "' name"));

          rParam.Type.FullTypeString = itr->ParameterNewType;
        }
      }
    }


    FunctionRecord GetFunction(const CaptureConfig& config, const CXCursor& cursor, const CXCursorKind cursorKind, const std::size_t currentLevel, FunctionErrors& rFuncErrors)
    {
      FunctionRecord currentFunction;

      currentFunction.Name = GetCursorSpelling(cursor);

      // Extract return type
      {
        const CXType returnType = clang_getCursorResultType(cursor);
        currentFunction.ReturnType = GetType(returnType);
      }

      // Extract parameters
      {
        std::unordered_set<std::string> uniqueArgumentNames;
        const unsigned int numArgs = clang_Cursor_getNumArguments(cursor);
        for (unsigned int i = 0; i<numArgs; ++i)
        {
          const CXCursor argCursor = clang_Cursor_getArgument(cursor, i);

          auto param = GetParameter(argCursor, config.TypeNamePrefix, config.FunctionParameterTypeOverrides);
          HandleParamNameOverride(param, config.FunctionParameterNameOverrides, currentFunction.Name, i);
          HandleParamTypeOverride(param, config.FunctionParameterTypeOverrides, currentFunction.Name, i);

          if (uniqueArgumentNames.find(param.Name) == uniqueArgumentNames.end())
          {
            uniqueArgumentNames.insert(param.Name);
          }
          else
          {
            rFuncErrors.DuplicatedParameterNames.push_back(DuplicatedParameterName(param.Name, i));
          }
          currentFunction.Parameters.push_back(param);
        }
      }
      if( rFuncErrors.DuplicatedParameterNames.size() > 0 )
        rFuncErrors.Name = currentFunction.Name;
      return currentFunction;
    }


    bool MatchesFilter(const std::deque<std::string>& filters, const std::string& functionName)
    {
      return std::find_if(filters.begin(), filters.end(), [functionName](const std::string& val) { return functionName.find(val) == 0; }) != filters.end();
    }

    
    MemberRecord GetMember(CXCursor cursor)
    {
      auto fieldType = GetType(cursor);
      const auto cursorSpelling = GetCursorSpelling(cursor);
      return MemberRecord(fieldType, cursorSpelling);
    }


    EnumMemberRecord GetEnumMember(CXCursor cursor)
    {
      const auto cursorSpelling = GetCursorSpelling(cursor);
      return EnumMemberRecord(cursorSpelling);
    }

    template<typename T>
    std::string BuildFullName(const std::deque<T>& parents, const std::string& name)
    {
      if (parents.size() == 0)
        return name;

      std::string fullName;
      for (auto itr = parents.begin(); itr != parents.end(); ++itr)
      {
        fullName = itr->Name + ".";
      }
      return fullName + name;
    }

  }


  Capture::Capture(const CaptureConfig& config, CXCursor rootCursor)
    : m_config(config)
    , m_level(0)
    , m_records()
  {


    // WARNING: This should be called once the object is fully constructed
    clang_visitChildren(rootCursor, Capture::VistorForwarder, this);

    if (m_functionErrors.size() <= 0)
      return;

    for (auto itr = m_functionErrors.begin(); itr != m_functionErrors.end(); ++itr)
    {
      for (auto itrDuplicated = itr->DuplicatedParameterNames.begin(); itrDuplicated != itr->DuplicatedParameterNames.end(); ++itrDuplicated)
      {
        std::cout << "ERROR: Function '" << itr->Name << "' has duplicated parameter named '" << itrDuplicated->Name << "' at parameter index " << itrDuplicated->Index << "\n";
      }
    }
    throw std::runtime_error("Duplicated parameter names found");
  }

 
  CXChildVisitResult Capture::OnVisit(CXCursor cursor, CXCursor parent)
  {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    if (m_config.OnlyScanMainHeaderFile && clang_Location_isFromMainFile(location) == 0)
      return CXChildVisit_Continue;

    CXCursorKind cursorKind = clang_getCursorKind(cursor);

    if (m_captureInfo.size() > 0)
    {
      const auto& captureInfo = m_captureInfo.back();
      if (captureInfo.Mode == CaptureMode::Struct)
      {
        assert(m_captureStructs.size() > 0);
        if (cursorKind == CXCursor_FieldDecl)
        {
          //std::cout << std::string(m_level, '-') << " " << GetCursorKindName(cursorKind) << " ('" << typeSpelling << "' '" << cursorSpelling << "')\n";

          m_captureStructs.back().Members.push_back(GetMember(cursor));
        }
      }
      else if (captureInfo.Mode == CaptureMode::Enum)
      {
        assert(m_captureEnums.size() > 0);
        if (cursorKind == CXCursor_EnumConstantDecl)
        {
          m_captureEnums.back().Members.push_back(GetEnumMember(cursor));
        }
      }
    }


    if (cursorKind == CXCursor_FunctionDecl)
    {
      const auto functionName = GetCursorSpelling(cursor);

      if (m_config.Filters.size() == 0 || MatchesFilter(m_config.Filters, functionName))
      {
        // Extract basic information
        FunctionErrors funcErrors;
        m_records.push_back(GetFunction(m_config, cursor, cursorKind, m_level, funcErrors));
        if (funcErrors.DuplicatedParameterNames.size() > 0)
          m_functionErrors.push_back(funcErrors);
      }
    }
    else if (cursorKind == CXCursor_StructDecl)
    {
      auto name = GetCursorSpelling(cursor);
      auto fullName = BuildFullName(m_captureStructs, name);
      m_captureInfo.push_back(CaptureInfo(CaptureMode::Struct, m_level));
      m_captureStructs.push_back(StructRecord(fullName));
    }
    else if (cursorKind == CXCursor_EnumDecl)
    {
      auto name = GetCursorSpelling(cursor);
      auto fullName = BuildFullName(m_captureEnums, name);
      m_captureInfo.push_back(CaptureInfo(CaptureMode::Enum, m_level));
      m_captureEnums.push_back(EnumRecord(fullName));
    }


    {
      ++m_level;
      clang_visitChildren(cursor, VistorForwarder, this);
      --m_level;

      if (m_captureInfo.size() > 0 && m_level <= m_captureInfo.back().Level)
      {
        switch (m_captureInfo.back().Mode)
        {
        case CaptureMode::Struct:
        {
          assert(m_captureStructs.size() > 0);
          auto itrFindName = m_structs.find(m_captureStructs.back().Name);
          m_structs[m_captureStructs.back().Name] = m_captureStructs.back();
          if(itrFindName == m_structs.end())
            m_structsInCapturedOrder.push_back(m_captureStructs.back());
          m_captureStructs.pop_back();
          break;
        }
        case CaptureMode::Enum:
          assert(m_captureEnums.size() > 0);
          m_enums[m_captureEnums.back().Name] = m_captureEnums.back();
          m_captureEnums.pop_back();
          break;
        default:
          std::cout << "WARNING: unhandled capture mode\n";
          break;
        }

        m_captureInfo.pop_back();
      }
    }
    return CXChildVisit_Continue;
  }


  void Capture::Dump()
  {
    for (auto itr = m_records.begin(); itr != m_records.end(); ++itr)
    {
      // Use the extracted information
      std::cout << std::string(m_level, '-') << itr->ReturnType.FullTypeString << " " << itr->Name << "(";
      for (std::size_t i = 0; i < itr->Parameters.size(); ++i)
      {
        if (i > 0)
          std::cout << ", ";
        std::cout << itr->Parameters[i].Type.FullTypeString;
        std::cout << " ";
        std::cout << itr->Parameters[i].Name;
      }
      std::cout << ")";
      std::cout << "\n";
    }
  }


  CXChildVisitResult Capture::VistorForwarder(CXCursor cursor, CXCursor parent, CXClientData clientData)
  {
    auto pCapture = static_cast<Capture*>(clientData);
    return pCapture->OnVisit(cursor, parent);
  }
}
