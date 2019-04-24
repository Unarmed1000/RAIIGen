//***************************************************************************************************************************************************
//* BSD 3-Clause License
//*
//* Copyright (c) 2017, Rene Thrane
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

#include <RAIIGen/Generator/Simple/Enum/EnumToStringLookup.hpp>
#include <RAIIGen/Generator/Simple/Enum/EnumToStringSnippets.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/IOUtil.hpp>
#include <RAIIGen/Generator/ConfigUtil.hpp>
#include <RAIIGen/StringHelper.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <fmt/format.h>

namespace MB
{
  using namespace Fsl;

  namespace
  {
    struct EnumValueMemberInfo
    {
      std::deque<EnumMemberRecord> MemberRecord;
      bool Generated{false};
    };


    std::string GetRelativeIncludeGuard(const IO::Path& rootPath, const IO::Path& dstFilename)
    {
      auto filename = dstFilename.ToUTF8String();
      auto root = rootPath.ToUTF8String();

      if (!StringUtil::EndsWith(root, "/"))
        root += "/";

      if (!StringUtil::StartsWith(filename, root))
        throw UsageErrorException("The dstFile name was not based on the given root path");

      if (root.size() > 0)
        filename.erase(0, root.size());

      StringUtil::Replace(filename, "/", "_");
      StringUtil::Replace(filename, ".", "_");
      return CaseUtil::UpperCase(filename);
    }

    EnumToStringSnippets LoadSnippets(const IO::Path& templateRoot)
    {
      EnumToStringSnippets snippets;
      const auto pathHeader = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_header.hpp");
      const auto pathMethod = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_method.txt");
      const auto pathCaseEntry = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_case_entry.txt");
      const auto pathCaseEntryReturn = IO::Path::Combine(templateRoot, "enumDebugStrings/Template_case_entry_return.txt");

      snippets.Header = IO::File::ReadAllText(pathHeader);
      snippets.Method = IO::File::ReadAllText(pathMethod);
      snippets.CaseEntry = IO::File::ReadAllText(pathCaseEntry);
      snippets.CaseEntryReturn = IO::File::ReadAllText(pathCaseEntryReturn);
      return snippets;
    }

    std::string GenerateEnumMember(const VersionGuardConfig& versionGuard, const std::string& snippet, const std::string& snippetReturn,
                                   const EnumMemberRecord& enumMember, EnumValueMemberInfo& rDuplicationInfo)
    {
      if (rDuplicationInfo.Generated)
      {
        return "";
      }

      std::string content;
      if (rDuplicationInfo.MemberRecord.size() == 1)
      {
        // The simple normal case
        content = snippet + END_OF_LINE + snippetReturn;
        StringUtil::Replace(content, "##ENUM_MEMBER_NAME##", enumMember.Name);

        if (versionGuard.IsValid && enumMember.Version != VersionRecord())
        {
          content = fmt::format("#if {0}{1}{2}{1}#endif", versionGuard.ToGuardString(enumMember.Version), END_OF_LINE, content);
        }
      }
      else
      {
        content.clear();
        auto lastVersion = VersionRecord();
        std::string lastName;
        uint32_t countIf = 0;
        bool addReturn = false;
        bool addEndIf = false;
        for (auto entry : rDuplicationInfo.MemberRecord)
        {
          if (lastVersion != entry.Version)
          {
            if (addReturn)
            {
              std::string entryContent = snippetReturn;
              StringUtil::Replace(entryContent, "##ENUM_MEMBER_NAME##", lastName);
              content += entryContent + END_OF_LINE;
              addReturn = false;
            }
            if (countIf > 0)
            {
              content += fmt::format("#elif {0}{1}", versionGuard.ToGuardString(entry.Version), END_OF_LINE);
            }
            else
            {
              content += fmt::format("#if {0}{1}", versionGuard.ToGuardString(entry.Version), END_OF_LINE);
            }
            lastVersion = entry.Version;
            addEndIf = true;
            ++countIf;
          }
          // There are duplicated entries
          std::string entryContent(snippet);    // +END_OF_LINE + snippetReturn;
          StringUtil::Replace(entryContent, "##ENUM_MEMBER_NAME##", entry.Name);
          addReturn = true;
          lastName = entry.Name;

          content += entryContent + END_OF_LINE;
        }
        if (addReturn)
        {
          std::string entryContent = snippetReturn;
          StringUtil::Replace(entryContent, "##ENUM_MEMBER_NAME##", lastName);
          content += entryContent + END_OF_LINE;
          addReturn = false;
        }
        if (addEndIf)
        {
          content += "#endif" + END_OF_LINE;
          addEndIf = false;
        }
      }
      rDuplicationInfo.Generated = true;
      return content;
    }


    std::string GenerateHeaderFile(const SimpleGeneratorConfig& config, const std::string& snippetHeader, const IO::Path& dstRootPath,
                                   const IO::Path& dstFileName, const std::string& content, const VersionRecord& version = VersionRecord())
    {
      std::string strVersinGuardBegin;
      std::string strVersinGuardEnd;


      if (config.VersionGuard.IsValid && version != VersionRecord())
      {
        strVersinGuardBegin = fmt::format(END_OF_LINE + "#if {0}", config.VersionGuard.ToGuardString(version));
        strVersinGuardEnd = END_OF_LINE + "#endif";
      }

      std::string headerContent = snippetHeader;
      StringUtil::Replace(headerContent, "##METHODS##", content);
      StringUtil::Replace(headerContent, "##NAMESPACE_NAME##", config.NamespaceName);
      StringUtil::Replace(headerContent, "##NAMESPACE_NAME!##", CaseUtil::UpperCase(config.NamespaceName));
      StringUtil::Replace(headerContent, "##AG_TOOL_STATEMENT##", config.ToolStatement);
      StringUtil::Replace(headerContent, "##RELATIVE_INCLUDE_GUARD##", GetRelativeIncludeGuard(dstRootPath, dstFileName));

      StringUtil::Replace(headerContent, "##VERSION_GUARD_BEGIN##", strVersinGuardBegin);
      StringUtil::Replace(headerContent, "##VERSION_GUARD_END##", strVersinGuardEnd);
      return headerContent;
    }

    std::unordered_map<uint64_t, EnumValueMemberInfo> FindDuplicatedValues(const std::deque<EnumMemberRecord>& members,
                                                                           const std::vector<BlackListEntry>& enumMemberBlacklist,
                                                                           const ConfigUtil::CurrentEntityInfo& currentEnumEntityInfo)
    {
      std::unordered_map<uint64_t, EnumValueMemberInfo> valueToNameLookup;
      for (const auto& enumMember : members)
      {
        if (!ConfigUtil::HasMatchingEntry(enumMember.Name, enumMemberBlacklist, currentEnumEntityInfo))
        {
          auto itrFind = valueToNameLookup.find(enumMember.UnsignedValue);
          if (itrFind == valueToNameLookup.end())
          {
            // first entry
            EnumValueMemberInfo record;
            record.MemberRecord.push_back(enumMember);
            valueToNameLookup.emplace(std::make_pair(enumMember.UnsignedValue, std::move(record)));
          }
          else
          {
            // Duplicated entry found
            itrFind->second.MemberRecord.push_back(enumMember);
          }
        }
      }

      for (auto& rEntries : valueToNameLookup)
      {
        if (rEntries.second.MemberRecord.size() > 1)
        {
          std::sort(rEntries.second.MemberRecord.begin(), rEntries.second.MemberRecord.end(),
                    [](const EnumMemberRecord& lhs, const EnumMemberRecord& rhs) -> bool { return lhs.Version > rhs.Version; });
        }
      }
      return valueToNameLookup;
    }


    void ProcessOneFile(const EnumToStringSnippets& snippets, const Capture& capture, const SimpleGeneratorConfig& config,
                        const IO::Path& dstRootPath, const IO::Path& dstFileName, const bool useSeperateFiles)
    {
      std::string content;

      auto enumDict = capture.GetEnumDict();
      bool isFirst = true;

      const std::string endOfLine3 = END_OF_LINE + END_OF_LINE + END_OF_LINE;

      std::vector<std::string> sortedNames(enumDict.size());
      {
        std::size_t index = 0;
        for (const auto& entry : enumDict)
        {
          sortedNames[index] = entry.first;
          ++index;
        }
        std::sort(sortedNames.begin(), sortedNames.end());
      }


      for (const auto& entryName : sortedNames)
      {
        std::size_t caseCount = 0;
        ConfigUtil::CurrentEntityInfo currentEnumEntityInfo(entryName);
        if (!ConfigUtil::HasMatchingEntry(entryName, config.EnumNameBlacklist, currentEnumEntityInfo))
        {
          const auto itrFind = enumDict.find(entryName);
          std::string switchCaseContent;

          auto valueToNameLookup = FindDuplicatedValues(itrFind->second.Members, config.EnumMemberBlacklist, currentEnumEntityInfo);

          for (const auto& enumMember : itrFind->second.Members)
          {
            auto itrDuplication = valueToNameLookup.find(enumMember.UnsignedValue);
            if (itrDuplication != valueToNameLookup.end())
            {
              auto enumContent = GenerateEnumMember(config.VersionGuard, snippets.CaseEntry, snippets.CaseEntryReturn, enumMember, itrDuplication->second);
              if (!enumContent.empty())
              {
                switchCaseContent += END_OF_LINE + enumContent;
                ++caseCount;
              }
            }
          }
          if (caseCount > 0)
          {
            std::string methodContent = snippets.Method;
            if (config.VersionGuard.IsValid && itrFind->second.Version != VersionRecord())
            {
              methodContent =
                fmt::format("#if {0}{1}{2}{1}#endif", config.VersionGuard.ToGuardString(itrFind->second.Version), END_OF_LINE, methodContent);
            }

            StringUtil::Replace(methodContent, "##C_TYPE_NAME##", itrFind->second.Name);
            StringUtil::Replace(methodContent, "##CASE_ENTRIES##", switchCaseContent);
            content += (!isFirst ? endOfLine3 : END_OF_LINE) + methodContent;


            isFirst = false;
          }
        }
      }

      std::string headerContent = GenerateHeaderFile(config, snippets.Header, dstRootPath, dstFileName, content);

      auto dirName = IO::Path::GetDirectoryName(dstFileName);
      IO::Directory::CreateDir(dirName);

      IOUtil::WriteAllTextIfChanged(dstFileName, headerContent);
    }


    void ProcessMultipleFile(const EnumToStringSnippets& snippets, const Capture& capture, const SimpleGeneratorConfig& config,
                             const IO::Path& dstRootPath, const IO::Path& dstFilePath, const bool useSeperateFiles)
    {
      IO::Directory::CreateDir(dstFilePath);

      auto enumDict = capture.GetEnumDict();

      for (const auto& entry : enumDict)
      {
        std::size_t caseCount = 0;
        ConfigUtil::CurrentEntityInfo currentEnumEntityInfo(entry.first);
        if (!ConfigUtil::HasMatchingEntry(entry.first, config.EnumNameBlacklist, currentEnumEntityInfo))
        {
          auto valueToNameLookup = FindDuplicatedValues(entry.second.Members, config.EnumMemberBlacklist, currentEnumEntityInfo);

          std::string switchCaseContent;
          for (const auto& enumMember : entry.second.Members)
          {
            auto itrDuplication = valueToNameLookup.find(enumMember.UnsignedValue);
            assert(itrDuplication != valueToNameLookup.end());

            if (itrDuplication != valueToNameLookup.end())
            {
              auto enumContent =
                GenerateEnumMember(config.VersionGuard, snippets.CaseEntry, snippets.CaseEntryReturn, enumMember, itrDuplication->second);
              if (!enumContent.empty())
              {
                switchCaseContent += END_OF_LINE + enumContent;
                ++caseCount;
              }
            }
          }
          if (caseCount > 0)
          {
            std::string methodContent = END_OF_LINE + snippets.Method;
            StringUtil::Replace(methodContent, "##C_TYPE_NAME##", entry.second.Name);
            StringUtil::Replace(methodContent, "##CASE_ENTRIES##", switchCaseContent);

            auto dstFileName = IO::Path::Combine(dstFilePath, entry.first + ".hpp");
            std::string headerContent = GenerateHeaderFile(config, snippets.Header, dstRootPath, dstFileName, methodContent, entry.second.Version);

            IOUtil::WriteAllTextIfChanged(dstFileName, headerContent);
          }
        }
      }
    }
  }


  void EnumToStringLookup::Process(const Capture& capture, const SimpleGeneratorConfig& config, const IO::Path& templateRoot,
                                   const IO::Path& dstRootPath, const IO::Path& dstFileName, const bool useSeperateFiles)
  {
    EnumToStringSnippets snippets = LoadSnippets(templateRoot);

    if (!useSeperateFiles)
      ProcessOneFile(snippets, capture, config, dstRootPath, dstFileName, useSeperateFiles);
    else
      ProcessMultipleFile(snippets, capture, config, dstRootPath, dstFileName, useSeperateFiles);
  }
}
