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

#include <RAIIGen/Generator/OpenVXGenerator.hpp>
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
    // OpenVX
    const auto CREATE_FUNCTION = "vxCreate";
    const auto DESTROY_FUNCTION = "vxRelease";

    const auto TYPE_NAME_PREFIX = "vx_";
    const auto FUNCTION_NAME_PREFIX = "vx";

    const auto ERRORCODE_TYPE_NAME = "vx_status";

    const std::vector<FunctionNamePair> g_functionPairs
    {
      FunctionNamePair(CREATE_FUNCTION, DESTROY_FUNCTION),
    };


    // Manual matches for methods that don't follow 'standard' patterns
    const std::vector<FunctionNamePair> g_manualFunctionMatches
    {
      // FIX: should be another create method on CommandQueue
      //FunctionNamePair("clCreateCommandQueueWithProperties", "clReleaseCommandQueue"),
    };


    const std::vector<RAIIClassCustomization> g_arrayRAIIClassCustomization
    {
    };


    const std::vector<ClassFunctionAbsorb> g_classFunctionAbsorbtion
    {
    };


    const std::unordered_map<std::string, RAIIClassMethodOverrides> g_classMethodOverride =
    {
    };


    const std::vector<std::string> g_forceNullParameter
    {
    };


    const std::vector<FunctionGuard> g_functionGuards
    {
    };


    const std::vector<BlackListEntry> g_functionNameBlacklist
    {
    };


    const std::vector<BlackListEntry> g_enumNameBlacklist
    {
    };


    const std::vector<BlackListEntry> g_enumMemberBlacklist
    {
    };


    const std::vector<FunctionParameterNameOverride> g_functionParameterNameOverride
    {
    };


    const std::vector<FunctionParameterTypeOverride> g_functionParameterTypeOverride
    {
    };


    const std::unordered_map<std::string, std::string> g_typeDefaultValues =
    {
      { "vx_array", "##HANDLE_CLASS_NAME##::INVALID_ARRAY" },
      { "vx_context", "##HANDLE_CLASS_NAME##::INVALID_CONTEXT" },
      { "vx_convolution", "##HANDLE_CLASS_NAME##::INVALID_CONVOLUTION" },
      { "vx_delay", "##HANDLE_CLASS_NAME##::INVALID_DELAY" },
      { "vx_distribution", "##HANDLE_CLASS_NAME##::INVALID_DISTRIBUTION" },
      { "vx_graph", "##HANDLE_CLASS_NAME##::INVALID_GRAPH" },
      { "vx_image", "##HANDLE_CLASS_NAME##::INVALID_IMAGE" },
      { "vx_lut", "##HANDLE_CLASS_NAME##::INVALID_LUT" },
      { "vx_matrix", "##HANDLE_CLASS_NAME##::INVALID_MATRIX" },
      { "vx_object_array", "##HANDLE_CLASS_NAME##::INVALID_OBJECT_ARRAY" },
      { "vx_pyramid", "##HANDLE_CLASS_NAME##::INVALID_PYRAMID" },
      { "vx_remap", "##HANDLE_CLASS_NAME##::INVALID_REMAP" },
      { "vx_scalar", "##HANDLE_CLASS_NAME##::INVALID_SCALAR" },
      { "vx_threshold", "##HANDLE_CLASS_NAME##::INVALID_THRESHOLD" },
    };
  }


  OpenVXGenerator::OpenVXGenerator(const Capture& capture, const BasicConfig& basicConfig, const Fsl::IO::Path& templateRoot, const Fsl::IO::Path& dstPath)
    : SimpleGenerator(capture, 
                      SimpleGeneratorConfig(basicConfig, g_functionPairs, g_manualFunctionMatches, g_arrayRAIIClassCustomization, 
                                            g_classFunctionAbsorbtion, g_classMethodOverride, g_typeDefaultValues, g_forceNullParameter,
                                            g_functionGuards, g_functionNameBlacklist, 
                                            g_enumNameBlacklist, g_enumMemberBlacklist,
                                            TYPE_NAME_PREFIX, FUNCTION_NAME_PREFIX, ERRORCODE_TYPE_NAME, false, false),
                      templateRoot, dstPath)
  {
  }


  CaptureConfig OpenVXGenerator::GetCaptureConfig()
  {
    std::deque<std::string> filters;
    filters.push_back(CREATE_FUNCTION);
    filters.push_back(DESTROY_FUNCTION);
    return CaptureConfig(TYPE_NAME_PREFIX, filters, g_functionParameterNameOverride, g_functionParameterTypeOverride, false);
  }
}
