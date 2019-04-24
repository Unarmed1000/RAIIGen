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

#include <RAIIGen/Generator/Simple/Struct/CStructToCpp.hpp>
#include <RAIIGen/Generator/Simple/Struct/StructSnippets.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/IOUtil.hpp>
#include <RAIIGen/StringHelper.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <iostream>

namespace MB
{
  using namespace Fsl;

  namespace
  {
    enum MemberClassification
    {
      Normal,
      VulkanStructTypeFlag,
      VulkanStructPNext,
    };

    std::string ToCppName(const std::string& name)
    {
      if (name.size() < 3 || name[0] != 'V' || name[1] != 'k')
        throw NotSupportedException(std::string("C type name not of the expected format: '") + name + "'");
      return StringHelper::EnforceUpperCamelCaseNameStyle(name.substr(2));
    }

    struct AnalyzedMemberRecord
    {
      MemberRecord Source;
      MemberClassification Classification;

      AnalyzedMemberRecord()
      {
      }

      explicit AnalyzedMemberRecord(const MemberRecord& source)
        : Source(source)
      {
        if (source.Name == "sType")
          Classification = MemberClassification::VulkanStructTypeFlag;
        else if (source.Name == "pNext")
          Classification = MemberClassification::VulkanStructPNext;
        else
          Classification = MemberClassification::Normal;
      }
    };

    struct AnalyzedStructRecord
    {
      StructRecord Source;
      std::string CTypeName;
      std::string CppTypeName;
      std::deque<AnalyzedMemberRecord> Members;
      bool UnrollMembersRecursively;

      AnalyzedStructRecord()
        : UnrollMembersRecursively(false)
      {
      }

      AnalyzedStructRecord(const StructRecord& source)
        : Source(source)
        , CTypeName(source.Name)
        , CppTypeName(ToCppName(source.Name))
        , UnrollMembersRecursively(false)
      {
        for (auto itr = source.Members.begin(); itr != source.Members.end(); ++itr)
        {
          Members.push_back(AnalyzedMemberRecord(*itr));
        }
      }
    };


    StructSnippets LoadSnippets(const IO::Path& templateRoot)
    {
      StructSnippets snippets;
      const auto pathHeader = IO::Path::Combine(templateRoot, "struct/Template_header.hpp");
      const auto pathStruct = IO::Path::Combine(templateRoot, "struct/Template_struct.txt");
      const auto pathStructConstructor = IO::Path::Combine(templateRoot, "struct/Template_structConstructor.txt");
      const auto pathStructParentListInitialization = IO::Path::Combine(templateRoot, "struct/Template_structParentListInitialization.txt");
      const auto pathStructParentMemberInitialization = IO::Path::Combine(templateRoot, "struct/Template_structParentMemberInitialization.txt");

      snippets.Header = IO::File::ReadAllText(pathHeader);
      snippets.Struct = IO::File::ReadAllText(pathStruct);
      snippets.StructConstructor = IO::File::ReadAllText(pathStructConstructor);
      snippets.StructParentListInitialization = IO::File::ReadAllText(pathStructParentListInitialization);
      snippets.StructParentMemberInitialization = IO::File::ReadAllText(pathStructParentMemberInitialization);
      return snippets;
    }


    std::string GenerateMemberParameterList(const std::deque<AnalyzedMemberRecord>& members,
                                            const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict,
                                            const bool unrollRecursively)
    {
      bool isFirst = true;
      std::string content;
      for (auto itr = members.begin(); itr != members.end(); ++itr)
      {
        if (itr->Classification == MemberClassification::Normal)
        {
          if (!isFirst)
            content += ", ";
          else
            isFirst = false;
          if (unrollRecursively && itr->Source.Type.IsStruct && !itr->Source.Type.IsPointer)
          {
            const auto itrFind = analyzedStructLookupDict.find(itr->Source.Type.Name);
            if (itrFind == analyzedStructLookupDict.end())
              throw NotFoundException("Could not locate the needed type");
            content += GenerateMemberParameterList(itrFind->second.Members, analyzedStructLookupDict, unrollRecursively);
          }
          else
            content += itr->Source.Type.FullTypeString + " " + itr->Source.ArgumentName;
        }
      }
      return content;
    }


    std::string GenerateParentMemberInitialization(const AnalyzedStructRecord& type, const std::string& strTemplate, const std::string& prefix,
                                                   const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict,
                                                   const bool unrollRecursively)
    {
      std::string result;
      for (auto itr = type.Members.begin(); itr != type.Members.end(); ++itr)
      {
        std::string setMember;
        switch (itr->Classification)
        {
        case MemberClassification::Normal:
          if (unrollRecursively && itr->Source.Type.IsStruct && !itr->Source.Type.IsPointer)
          {
            const auto itrFind = analyzedStructLookupDict.find(itr->Source.Type.Name);
            if (itrFind == analyzedStructLookupDict.end())
              throw NotFoundException("Could not locate the needed type");

            const std::string newPrefix(prefix + itr->Source.ArgumentName + ".");
            result += GenerateParentMemberInitialization(itrFind->second, strTemplate, newPrefix, analyzedStructLookupDict, true);
            continue;
          }
          else
            setMember = prefix + itr->Source.ArgumentName + " = " + itr->Source.ArgumentName + ";";
          break;
        case MemberClassification::VulkanStructTypeFlag:
          setMember = prefix + itr->Source.ArgumentName + " = " + StringHelper::GenerateVulkanStructFlagName(type.CTypeName) + ";";
          break;
        case MemberClassification::VulkanStructPNext:
          setMember = prefix + itr->Source.ArgumentName + " = nullptr;";
          break;
        default:
          throw NotSupportedException("Classification not handled");
        }
        std::string content(strTemplate);
        StringUtil::Replace(content, "##SET_MEMBER##", setMember);
        result += END_OF_LINE + content;
      }

      return result;
    }


    std::string GenerateParentListInitializationParameterList(const AnalyzedStructRecord& type,
                                                              const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict,
                                                              const bool unrollRecursively)
    {
      bool isFirst = true;
      std::string result = "{";
      for (auto itr = type.Members.begin(); itr != type.Members.end(); ++itr)
      {
        if (!isFirst)
          result += ", ";
        else
          isFirst = false;

        if (unrollRecursively && itr->Source.Type.IsStruct && !itr->Source.Type.IsPointer)
        {
          const auto itrFind = analyzedStructLookupDict.find(itr->Source.Type.Name);
          if (itrFind == analyzedStructLookupDict.end())
            throw NotFoundException("Could not locate the needed type");
          result += GenerateParentListInitializationParameterList(itrFind->second, analyzedStructLookupDict, unrollRecursively);
        }
        else
        {
          switch (itr->Classification)
          {
          case MemberClassification::Normal:
            result += itr->Source.ArgumentName;
            break;
          case MemberClassification::VulkanStructTypeFlag:
            result += StringHelper::GenerateVulkanStructFlagName(type.CTypeName);
            break;
          case MemberClassification::VulkanStructPNext:
            result += "nullptr";
            break;
          default:
            throw NotSupportedException("Classification not handled");
          }
        }
      }
      result += "}";
      return result;
    }


    std::string GenerateParentListInitialization(const AnalyzedStructRecord& type, const std::string& strTemplate,
                                                 const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict,
                                                 const bool unrollRecursively)
    {
      std::string parameters = GenerateParentListInitializationParameterList(type, analyzedStructLookupDict, unrollRecursively);

      std::string content(strTemplate);
      StringUtil::Replace(content, "##PARENT_MEMBERS_LIST_INITIALIZATION##", parameters);
      return content;
    }

    std::string GenerateUnrolledConstructor(const AnalyzedStructRecord& type, const StructSnippets& snippets,
                                            const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict,
                                            const bool unrollRecursively)
    {
      std::string createMethodParameters = GenerateMemberParameterList(type.Members, analyzedStructLookupDict, unrollRecursively);
      std::string modernParentListInitialization =
        GenerateParentListInitialization(type, snippets.StructParentListInitialization, analyzedStructLookupDict, unrollRecursively);
      std::string parentMemberInitialization =
        GenerateParentMemberInitialization(type, snippets.StructParentMemberInitialization, "this->", analyzedStructLookupDict, unrollRecursively);

      std::string result = snippets.StructConstructor;
      StringUtil::Replace(result, "##CREATE_METHOD_PARAMETERS##", createMethodParameters);
      StringUtil::Replace(result, "##MODERN_PARENT_LIST_INITIALIZATION##", modernParentListInitialization);
      StringUtil::Replace(result, "##PARENT_MEMBER_INITIALIZATION##", parentMemberInitialization);
      return result;
    }


    std::string GenerateConstructors(const AnalyzedStructRecord& type, const StructSnippets& snippets,
                                     const std::unordered_map<std::string, AnalyzedStructRecord>& analyzedStructLookupDict)
    {
      if (type.Members.size() <= 0)
        return std::string();

      std::string content = GenerateUnrolledConstructor(type, snippets, analyzedStructLookupDict, false);

      if (type.UnrollMembersRecursively)
        content += END_OF_LINE + END_OF_LINE + GenerateUnrolledConstructor(type, snippets, analyzedStructLookupDict, true);


      return content;
    }


    std::string GetTypeFlagInitialization(const AnalyzedStructRecord& type, const std::string& strTemplate)
    {
      for (auto itr = type.Members.begin(); itr != type.Members.end(); ++itr)
      {
        if (itr->Classification == MemberClassification::VulkanStructTypeFlag)
        {
          const auto setMember = "this->" + itr->Source.ArgumentName + " = " + StringHelper::GenerateVulkanStructFlagName(type.CTypeName) + ";";
          std::string content(strTemplate);
          StringUtil::Replace(content, "##SET_MEMBER##", setMember);
          return END_OF_LINE + content;
        }
      }
      return std::string();
    }

  }


  CStructToCpp::CStructToCpp(const Capture& capture, const std::string& toolStatement, const std::string& namespaceName, const IO::Path& templateRoot,
                             const IO::Path& dstFileName)
  {
    StructSnippets snippets = LoadSnippets(templateRoot);

    auto structs = capture.GetStructs();

    std::vector<std::string> skipTypes = {
      // FIX: these types use "fixed array size" type members  like "char test[2]"
      "VkPhysicalDeviceLimits", "VkPhysicalDeviceProperties", "VkPhysicalDeviceMemoryProperties",    "VkImageBlit",
      "VkExtensionProperties",  "VkLayerProperties",          "VkPipelineColorBlendStateCreateInfo", "VkDebugMarkerMarkerInfoEXT",
    };

    std::unordered_map<std::string, AnalyzedStructRecord> analyzedStructLookupDict;
    {
      std::vector<std::string> unrollMembersRecursively{"VkRect2D"};

      for (auto itr = structs.begin(); itr != structs.end(); ++itr)
      {
        AnalyzedStructRecord analyzedStruct(*itr);
        if (std::find(unrollMembersRecursively.begin(), unrollMembersRecursively.end(), itr->Name) != unrollMembersRecursively.end())
          analyzedStruct.UnrollMembersRecursively = true;

        analyzedStructLookupDict[itr->Name] = analyzedStruct;
      }
    }


    std::string typeCode = "";
    for (auto itr = structs.begin(); itr != structs.end(); ++itr)
    {
      // if (itr->Name != "VkRect2D")
      //  continue;
      if (std::find(skipTypes.begin(), skipTypes.end(), itr->Name) != skipTypes.end())
        continue;

      auto itrFind = analyzedStructLookupDict.find(itr->Name);
      if (itrFind == analyzedStructLookupDict.end())
        throw NotFoundException("Could not locate the expected type");

      const auto parentTypeFlagInitialization = GetTypeFlagInitialization(itrFind->second, snippets.StructParentMemberInitialization);

      std::string extraConstructors = GenerateConstructors(itrFind->second, snippets, analyzedStructLookupDict);
      std::string content(snippets.Struct);
      StringUtil::Replace(content, "##CLASS_EXTRA_CONSTRUCTORS_HEADER##", extraConstructors);
      StringUtil::Replace(content, "##CPP_TYPE_NAME##", itrFind->second.CppTypeName);
      StringUtil::Replace(content, "##C_TYPE_NAME##", itrFind->second.CTypeName);
      StringUtil::Replace(content, "##PARENT_TYPE_FLAG_INITIALIZATION##", parentTypeFlagInitialization);
      typeCode += END_OF_LINE + END_OF_LINE + content;
    }

    std::string headerContent = snippets.Header;
    StringUtil::Replace(headerContent, "##ALL_TYPES##", typeCode);
    StringUtil::Replace(headerContent, "##NAMESPACE_NAME##", namespaceName);
    StringUtil::Replace(headerContent, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(namespaceName));
    StringUtil::Replace(headerContent, "##AG_TOOL_STATEMENT##", toolStatement);


    // std::cout << headerContent << "\n";

    auto dirName = IO::Path::GetDirectoryName(dstFileName);
    IO::Directory::CreateDir(dirName);

    IOUtil::WriteAllTextIfChanged(dstFileName, headerContent);
  }

}
