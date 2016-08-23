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

#include <RAIIGen/Generator/OpenCLGenerator.hpp>
#include <RAIIGen/Generator/FunctionNamePair.hpp>
#include <RAIIGen/Generator/MatchedFunctionPair.hpp>
#include <RAIIGen/CaseUtil.hpp>
#include <RAIIGen/Capture.hpp>
#include <RAIIGen/FunctionParameterNameOverride.hpp>
#include <FslBase/Exceptions.hpp>
#include <FslBase/IO/File.hpp>
#include <FslBase/String/StringUtil.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>


using namespace Fsl;

namespace MB
{

  namespace
  {
    // OpenCL
    const auto CREATE_FUNCTION = "clCreate";
    const auto DESTROY_FUNCTION = "clRelease";

    const auto TYPE_NAME_PREFIX = "cl_";


    const std::vector<FunctionNamePair> g_functionPairs
    {
      FunctionNamePair(CREATE_FUNCTION, DESTROY_FUNCTION),
    };


    // Manual matches for methods that don't follow 'standard' patterns
    const std::vector<FunctionNamePair> g_manualFunctionMatches
    {
      FunctionNamePair("clCreateCommandQueueWithProperties", "clReleaseCommandQueue"),
      FunctionNamePair("clCreateContextFromType", "clReleaseContext"),
      FunctionNamePair("clCreateProgramWithSource", "clReleaseProgram"),
      FunctionNamePair("clCreateProgramWithBinary", "clReleaseProgram"),
      FunctionNamePair("clCreateProgramWithBuiltInKernels", "clReleaseProgram"),
      FunctionNamePair("clCreateBuffer", "clReleaseMemObject"),
      FunctionNamePair("clCreateImage", "clReleaseMemObject"),
      FunctionNamePair("clCreateImage2D", "clReleaseMemObject"),
      FunctionNamePair("clCreateImage3D", "clReleaseMemObject"),
      FunctionNamePair("clCreatePipe", "clReleaseMemObject"),
      FunctionNamePair("clCreateSamplerWithProperties", "clReleaseSampler"),
      FunctionNamePair("clCreateUserEvent", "clReleaseEvent"),
    };


    const std::vector<ClassFunctionAbsorb> g_classFunctionAbsorbtion
    {
      ClassFunctionAbsorb("CommandQueue", "clCreateCommandQueueWithProperties", false),
      ClassFunctionAbsorb("Context", "clCreateContextFromType"),
      ClassFunctionAbsorb("Program", "clCreateProgramWithSource", true),
      ClassFunctionAbsorb("Program", "clCreateProgramWithBinary", true),
      ClassFunctionAbsorb("Program", "clCreateProgramWithBuiltInKernels", true),
      ClassFunctionAbsorb("Image", "clCreateImage2D", true),
      ClassFunctionAbsorb("Image", "clCreateImage3D", true),
      ClassFunctionAbsorb("Sampler", "clCreateSamplerWithProperties"),
    };

// WARNING: No match found for: clCreateSubDevices
// WARNING: No match found for: clCreateSubBuffer
// WARNING: No match found for: clCreateKernelsInProgram

    const std::vector<std::string> g_forceNullParameter
    {
    };


    // This crap is necessary because they decided to 'skip' naming the parameters in the header file!!!
    const std::vector<FunctionParameterNameOverride> g_functionParameterNameOverride
    {
      // clCreateBuffer
      FunctionParameterNameOverride("clCreateBuffer", 3, "pVoid", "pHost"),
      // clCreateImage2D
      FunctionParameterNameOverride("clCreateImage2D", 3, "size", "imageWidth"),
      FunctionParameterNameOverride("clCreateImage2D", 4, "size", "imageHeight"),
      FunctionParameterNameOverride("clCreateImage2D", 5, "size", "imageRowPitch"),
      FunctionParameterNameOverride("clCreateImage2D", 6, "pVoid", "pHost"),
      // clCreateImage3D
      FunctionParameterNameOverride("clCreateImage3D", 3, "size", "imageWidth"),
      FunctionParameterNameOverride("clCreateImage3D", 4, "size", "imageHeight"),
      FunctionParameterNameOverride("clCreateImage3D", 5, "size", "imageDepth"),
      FunctionParameterNameOverride("clCreateImage3D", 6, "size", "imageRowPitch"),
      FunctionParameterNameOverride("clCreateImage3D", 7, "size", "imageSlicePitch"),
      FunctionParameterNameOverride("clCreateImage3D", 8, "pVoid", "pHost"),
      // clCreateContextFromType
      FunctionParameterNameOverride("clCreateContextFromType", 2, "pVoid(constChar*,ConstVoid*,SizeT,Void*)", "pfnNotify"),
      FunctionParameterNameOverride("clCreateContextFromType", 3, "pVoid", "pUserData"),
      // clCreateContext
      FunctionParameterNameOverride("clCreateContext", 1, "uint", "numDevices"),
      FunctionParameterNameOverride("clCreateContext", 3, "pVoid(constChar*,ConstVoid*,SizeT,Void*)", "pfnNotify"),
      FunctionParameterNameOverride("clCreateContext", 4, "pVoid", "pUserData"),
     
      // clCreateKernel
      FunctionParameterNameOverride("clCreateKernel", 1, "pChar", "pszKernelName"),


      // clCreatePipe
      FunctionParameterNameOverride("clCreatePipe", 2, "uint", "pipePacketSize"),
      FunctionParameterNameOverride("clCreatePipe", 3, "uint", "pipeMaxPackets"),
      // clCreateProgramWithSource
      FunctionParameterNameOverride("clCreateProgramWithSource", 1, "uint", "count"),
      FunctionParameterNameOverride("clCreateProgramWithSource", 2, "pChar", "ppStrings"),
      FunctionParameterNameOverride("clCreateProgramWithSource", 3, "pSize", "pLengths"),
      // clCreateProgramWithBinary
      FunctionParameterNameOverride("clCreateProgramWithBinary", 1, "uint", "numDevices"),
      FunctionParameterNameOverride("clCreateProgramWithBinary", 3, "pSize", "lengths"),
      FunctionParameterNameOverride("clCreateProgramWithBinary", 4, "pUnsignedChar", "ppBinaries"),
      FunctionParameterNameOverride("clCreateProgramWithBinary", 5, "pInt", "pBinaryStatus"),
      // clCreateSampler
      FunctionParameterNameOverride("clCreateSampler", 1, "bool", "normalizedCoords"),
    };


    // This crap is necessary because I have been unable to get the 'exact written code' from clang and
    // because OpenCL decided not to use a typedef for its function pointers
    const std::vector<FunctionParameterTypeOverride> g_functionParameterTypeOverride
    {
      // clCreateContextFromType
      FunctionParameterTypeOverride("clCreateContextFromType", 2, "void (*)(const char *, const void *, size_t, void *) __attribute__((stdcall))", "void (CL_CALLBACK *)(const char *, const void *, size_t, void *)"),
      // clCreateContext
      FunctionParameterTypeOverride("clCreateContext", 3, "void (*)(const char *, const void *, size_t, void *) __attribute__((stdcall))", "void (CL_CALLBACK *)(const char *, const void *, size_t, void *)"),
    };


    const std::unordered_map<std::string, std::string> g_typeDefaultValues =
    {
      { "cl_context", "##HANDLE_CLASS_NAME##::INVALID_CONTEXT" },
      { "cl_command_queue", "##HANDLE_CLASS_NAME##::INVALID_COMMAND_QUEUE" },
      { "cl_kernel", "##HANDLE_CLASS_NAME##::INVALID_KERNEL" },
      { "cl_event", "##HANDLE_CLASS_NAME##::INVALID_EVENT" },
      { "cl_mem", "##HANDLE_CLASS_NAME##::INVALID_MEM" },
      { "cl_program", "##HANDLE_CLASS_NAME##::INVALID_PROGRAM" },
      { "cl_sampler", "##HANDLE_CLASS_NAME##::INVALID_SAMPLER" },
    };


    Capture ModCapture(const Capture& capture)
    {
      Capture moddedCapture(capture);
      auto& functions = moddedCapture.DirectFunctions();
      for (auto itr = functions.begin(); itr != functions.end(); ++itr)
      {
        if (itr->Name.find(CREATE_FUNCTION) == 0 && itr->Parameters.size() > 0)
        {
          ParameterRecord& rLastParam = itr->Parameters.back();
          if (!rLastParam.Type.IsConstQualified && rLastParam.Type.IsPointer && rLastParam.Type.Name == "cl_int")
          {
            rLastParam.ParamType = ParameterType::ErrorCode;
            rLastParam.Name = "errorCode";
            rLastParam.ArgumentName = "errorCode";
            std::cout << "ErrorCode output detected " << itr->Name << "\n";
          }
        }
      }
      return moddedCapture;
    }
  }


  OpenCLGenerator::OpenCLGenerator(const Capture& capture, const BasicConfig& basicConfig, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : SimpleGenerator(ModCapture(capture), SimpleGeneratorConfig(basicConfig, g_functionPairs, g_manualFunctionMatches, g_classFunctionAbsorbtion, g_typeDefaultValues,  g_forceNullParameter, TYPE_NAME_PREFIX), templateRoot, dstPath)
  {
  }


  CaptureConfig OpenCLGenerator::GetCaptureConfig()
  {
    std::deque<std::string> filters;
    filters.push_back(CREATE_FUNCTION);
    filters.push_back(DESTROY_FUNCTION);
    return CaptureConfig(TYPE_NAME_PREFIX, filters, g_functionParameterNameOverride, g_functionParameterTypeOverride, true);
  }
}
