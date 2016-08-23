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
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <FslBase/IO/Directory.hpp>
#include <FslBase/IO/Path.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/Generator/BasicConfig.hpp>
#include <RAIIGen/Generator/OpenCLGenerator.hpp>
#include <RAIIGen/Generator/OpenVXGenerator.hpp>
#include <RAIIGen/Generator/VulkanGenerator.hpp>

namespace MB
{
  namespace
  {
    const std::string g_programName = "RAIIGen V0.2.5";

    using namespace Fsl;

    //std::string GetCursorKindName(CXCursorKind cursorKind)
    //{
    //  CXString kindName = clang_getCursorKindSpelling(cursorKind);
    //  std::string result = clang_getCString(kindName);

    //  clang_disposeString(kindName);
    //  return result;
    //}


    //std::string GetCursorSpelling(CXCursor cursor)
    //{
    //  CXString cursorSpelling = clang_getCursorSpelling(cursor);
    //  std::string result = clang_getCString(cursorSpelling);

    //  clang_disposeString(cursorSpelling);
    //  return result;
    //}


    ////VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
    ////  const VkInstanceCreateInfo*                 pCreateInfo,
    ////  const VkAllocationCallbacks*                pAllocator,
    ////  VkInstance*                                 pInstance);
    ////
    ////FunctionDecl(vkCreateInstance)
    ////  - TypeRef(VkResult)
    ////  - ParmDecl(pCreateInfo)
    ////  --TypeRef(VkInstanceCreateInfo)
    ////  - ParmDecl(pAllocator)
    ////  --TypeRef(VkAllocationCallbacks)
    ////  - ParmDecl(pInstance)
    ////  --TypeRef(VkInstance)
    ////
    //void ExamineFunc(const CXCursor& cursor, const CXCursorKind cursorKind, const int currentLevel)
    //{
    //  const std::string cursorKindName = GetCursorKindName(cursorKind);
    //  const std::string cursorSpelling = GetCursorSpelling(cursor);

    //  if (cursorSpelling.find("vkCreate") == 0)
    //  {
    //    std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";

    //  }
    //  else if (cursorSpelling.find("vkDestroy") == 0)
    //    std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
    //  else if (cursorSpelling.find("vkAllocate") == 0)
    //    std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
    //  else if (cursorSpelling.find("vkFree") == 0)
    //    std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
    //}


    //CXChildVisitResult Visitor(CXCursor cursor, CXCursor /* parent */, CXClientData clientData)
    //{
    //  CXSourceLocation location = clang_getCursorLocation(cursor);
    //  if (clang_Location_isFromMainFile(location) == 0)
    //    return CXChildVisit_Continue;

    //  CXCursorKind cursorKind = clang_getCursorKind(cursor);

    //  unsigned int curLevel = *(reinterpret_cast<unsigned int*>(clientData));
    //  unsigned int nextLevel = curLevel + 1;

    //  //clang_getCursorType(cursor);
    //  switch (cursorKind)
    //  {
    //  case CXCursor_FunctionDecl:
    //    ExamineFunc(cursor, cursorKind, curLevel);
    //    break;
    //  default:
    //    std::cout << std::string(curLevel, '-') << " " << GetCursorKindName(cursorKind) << " (" << GetCursorSpelling(cursor) << ")\n";
    //    break;
    //  }


    //  clang_visitChildren(cursor, Visitor, &nextLevel);

    //  return CXChildVisit_Continue;
    //}


    struct Config
    {
      IO::Path HeaderRoot;
      IO::Path TemplateRoot;
      IO::Path OutputRoot;

      Config()
      {
      }


      Config(const IO::Path& headerRoot, const IO::Path& templateRoot, const IO::Path& outputRoot)
        : HeaderRoot(headerRoot)
        , TemplateRoot(templateRoot)
        , OutputRoot(outputRoot)
      {
      }
    };

    template<typename TGenerator>
    void Run(const BasicConfig& basicConfig, const IO::Path& filename, const IO::Path& templatePath, const IO::Path& dstPath, const std::vector<IO::Path>& includePaths)
    {
      using namespace MB;

      // Ensure that the dst path exist
      IO::Directory::CreateDirectory(dstPath);

      //char* clangAargs[] =
      //{
      //  "-Ic:/Program Files (x86)/AMD APP SDK/3.0/include",
      //  "-Ie:/_sdk/amdovx-core/openvx/include",
      //  //"-I.",
      //  //"-I./include",
      //  //"-x",
      //  //"c++",
      //  //"-Xclang",
      //  //"-ast-dump",
      //  //"-fsyntax-only",
      //  //"-std=c++1y"
      //};

      std::vector<std::string> clangArgsTemp(includePaths.size());
      for (std::size_t i = 0; i < clangArgsTemp.size(); ++i)
        clangArgsTemp[i] = (std::string("-I") + includePaths[i].ToUTF8String());

      std::vector<const char*> clangArgs(clangArgsTemp.size());
      for (std::size_t i = 0; i < clangArgs.size(); ++i)
        clangArgs[i] = clangArgsTemp[i].c_str();

      CXIndex index = clang_createIndex(0, 1);
      CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, filename.ToUTF8String().c_str(), static_cast<int>(clangArgs.size()), clangArgs.data(), 0, nullptr);

      if (!tu)
      {
        std::cout << "Failed\n";
        throw std::runtime_error("Failed to translate source file");
      }


      CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

      {
        //unsigned int treeLevel = 0;
        //clang_visitChildren(rootCursor, Visitor, &treeLevel);
      }

      {
        const auto captureConfig = TGenerator::GetCaptureConfig();
        Capture capture(captureConfig, rootCursor);
        //capture.Dump();
        TGenerator generator(capture, basicConfig, templatePath, dstPath);
      }

      clang_disposeTranslationUnit(tu);
      clang_disposeIndex(index);

    }


    template<typename TGenerator>
    void RunGenerator(const std::string& programName, const Config& config, const std::string& filename, const std::string& templateName, const std::string& baseApiName, const std::string& apiVersion)
    {
      const auto apiNameAndVersion = baseApiName + apiVersion;
      const auto templateNameAndVersion = templateName + apiVersion;
      const auto apiHeaderPath = IO::Path::Combine(config.HeaderRoot, IO::Path::Combine("khronos", apiNameAndVersion));
      const auto templatePath = IO::Path::Combine(config.TemplateRoot, templateName);
      const auto srcFile = IO::Path::Combine(apiHeaderPath, filename);
      const auto dstPath = IO::Path::Combine(config.OutputRoot, templateNameAndVersion);

      const std::vector<IO::Path> includePaths = { apiHeaderPath };
      std::cout << "*** Running " << apiNameAndVersion << " generator ***\n";


      const auto toolStatement = std::string("Auto-generated ") + baseApiName + " " + apiVersion + " C++11 RAII classes based on " + programName + " (https://github.com/Unarmed1000)";

      auto namespaceName = apiNameAndVersion;
      StringUtil::Replace(namespaceName, ".", "_");

      BasicConfig basicConfig(programName, toolStatement, namespaceName);

      Run<TGenerator>(basicConfig, srcFile, templatePath, dstPath, includePaths);
    }


    void GenerateClasses(const std::string& programName, const IO::Path& currentWorkingDirectory)
    {
      const auto headerRoot = IO::Path::Combine(currentWorkingDirectory, "config/Headers");
      const auto templateRoot = IO::Path::Combine(currentWorkingDirectory, "config/Templates");
      const auto outputRoot = IO::Path::Combine(currentWorkingDirectory, "output");

      Config config(headerRoot, templateRoot, outputRoot);

      RunGenerator<MB::OpenCLGenerator>(programName, config, "CL/cl.h", "OpenCL", "OpenCL", "1.1");
      RunGenerator<MB::OpenCLGenerator>(programName, config, "CL/cl.h", "OpenCL", "OpenCL", "1.2");
      RunGenerator<MB::OpenCLGenerator>(programName, config, "CL/cl.h", "OpenCL", "OpenCL", "2.0");
      RunGenerator<MB::OpenCLGenerator>(programName, config, "CL/cl.h", "OpenCL", "OpenCL", "2.1");
      RunGenerator<MB::OpenVXGenerator>(programName, config, "VX/vx.h", "OpenVX", "OpenVX", "1.0.1");
      RunGenerator<MB::OpenVXGenerator>(programName, config, "VX/vx.h", "OpenVX", "OpenVX", "1.1");
      RunGenerator<MB::VulkanGenerator>(programName, config, "vulkan/vulkan.h", "Vulkan", "Vulkan", "1.0");

      RunGenerator<MB::OpenCLGenerator>(programName, config, "CL/cl.h", "RapidOpenCL", "OpenCL", "1.1");
      RunGenerator<MB::OpenVXGenerator>(programName, config, "VX/vx.h", "RapidOpenVX", "OpenVX", "1.0.1");
      RunGenerator<MB::VulkanGenerator>(programName, config, "vulkan/vulkan.h", "RapidVulkan", "Vulkan", "1.0");
    }
  }
}



int main(int argc, char** argv)
{
  using namespace MB;

  std::cout << g_programName << "\n";

  const auto currentPath = IO::Directory::GetCurrentWorkingDirectory();

  try
  {
    GenerateClasses(g_programName, currentPath);
  }
  catch (const std::exception& ex)
  {
    std::cout << "ERROR: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}