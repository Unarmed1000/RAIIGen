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

      // clEnqueueReadBuffer
      FunctionParameterNameOverride("clEnqueueReadBuffer", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 2, "bool", "blockingRead"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 3, "size", "offset"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 4, "size", "cb"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 5, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueReadBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueReadBufferRect
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 2, "bool", "blockingRead"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 3, "pSize", "pBufferOrigin"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 4, "pSize", "pHostOrigin"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 6, "size", "bufferRowPitch"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 7, "size", "bufferSlicePitch"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 8, "size", "hostRowPitch"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 9, "size", "hostSlicePitch"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 10, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 11, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueReadBufferRect", 12, "pEvent", "pEventWaitList"),

      // clEnqueueWriteBuffer
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 2, "bool", "blockingWrite"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 3, "size", "offset"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 4, "size", "cb"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 5, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueWriteBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueWriteBufferRect
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 2, "bool", "blockingWrite"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 3, "pSize", "pBufferOrigin"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 4, "pSize", "pHostOrigin"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 6, "size", "bufferRowPitch"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 7, "size", "bufferSlicePitch"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 8, "size", "hostRowPitch"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 9, "size", "hostSlicePitch"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 10, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 11, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueWriteBufferRect", 12, "pEvent", "pEventWaitList"),

      // clEnqueueCopyBuffer
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 1, "mem", "srcBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 2, "mem", "dstBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 3, "size", "srcOffset"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 4, "size", "dstOffset"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 5, "size", "cb"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueCopyBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueCopyBufferRect
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 1, "mem", "srcBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 2, "mem", "dstBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 3, "pSize", "pSrcOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 4, "pSize", "pDstOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 6, "size", "srcRowPitch"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 7, "size", "srcSlicePitch"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 8, "size", "dstRowPitch"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 9, "size", "dstSlicePitch"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 10, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueCopyBufferRect", 11, "pEvent", "pEventWaitList"),

      // clEnqueueReadImage
      FunctionParameterNameOverride("clEnqueueReadImage", 1, "mem", "image"),
      FunctionParameterNameOverride("clEnqueueReadImage", 2, "bool", "blockingRead"),
      FunctionParameterNameOverride("clEnqueueReadImage", 3, "pSize", "pOrigin"),
      FunctionParameterNameOverride("clEnqueueReadImage", 4, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueReadImage", 5, "size", "rowPitch"),
      FunctionParameterNameOverride("clEnqueueReadImage", 6, "size", "slicePitch"),
      FunctionParameterNameOverride("clEnqueueReadImage", 7, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueReadImage", 8, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueReadImage", 9, "pEvent", "pEventWaitList"),

      // clEnqueueWriteImage
      FunctionParameterNameOverride("clEnqueueWriteImage", 1, "mem", "image"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 2, "bool", "blockingWrite"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 3, "pSize", "pOrigin"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 4, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 5, "size", "inputRowPitch"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 6, "size", "inputSlicePitch"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 7, "pVoid", "ptr"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 8, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueWriteImage", 9, "pEvent", "pEventWaitList"),

      // clEnqueueCopyImage
      FunctionParameterNameOverride("clEnqueueCopyImage", 1, "mem", "srcImage"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 2, "mem", "dstImage"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 3, "pSize", "pSrcOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 4, "pSize", "pDstOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueCopyImage", 7, "pEvent", "pEventWaitList"),

      // clEnqueueCopyImageToBuffer
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 1, "mem", "srcImage"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 2, "mem", "dstBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 3, "pSize", "pSrcOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 4, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 5, "size", "dstOffset"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueCopyImageToBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueCopyBufferToImage
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 1, "mem", "srcBuffer"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 2, "mem", "dstImage"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 3, "size", "srcOffset"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 4, "pSize", "pDstOrigin"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueCopyBufferToImage", 7, "pEvent", "pEventWaitList"),

      // clEnqueueMapBuffer
      FunctionParameterNameOverride("clEnqueueMapBuffer", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueMapBuffer", 2, "bool", "blockingMap"),
      FunctionParameterNameOverride("clEnqueueMapBuffer", 4, "size", "offset"),
      FunctionParameterNameOverride("clEnqueueMapBuffer", 5, "size", "cb"),
      FunctionParameterNameOverride("clEnqueueMapBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueMapBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueMapImage
      FunctionParameterNameOverride("clEnqueueMapImage", 1, "mem", "image"),
      FunctionParameterNameOverride("clEnqueueMapImage", 2, "bool", "blockingMap"),
      FunctionParameterNameOverride("clEnqueueMapImage", 4, "pSize", "pOrigin"),
      FunctionParameterNameOverride("clEnqueueMapImage", 5, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueMapImage", 6, "pSize", "pImageRowPitch"),
      FunctionParameterNameOverride("clEnqueueMapImage", 7, "pSize", "pImageSlicePitch"),
      FunctionParameterNameOverride("clEnqueueMapImage", 8, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueMapImage", 9, "pEvent", "pEventWaitList"),
      FunctionParameterNameOverride("clEnqueueMapImage", 11, "pInt", "pErrorCode"),

      // clEnqueueUnmapMemObject
      FunctionParameterNameOverride("clEnqueueUnmapMemObject", 1, "mem", "memObj"),
      FunctionParameterNameOverride("clEnqueueUnmapMemObject", 2, "pVoid", "mappedPtr"),
      FunctionParameterNameOverride("clEnqueueUnmapMemObject", 3, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueUnmapMemObject", 4, "pEvent", "pEventWaitList"),

      // clEnqueueNDRangeKernel
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 2, "uint", "workDim"),
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 3, "pSize", "pGlobalWorkOffset"),
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 4, "pSize", "pGlobalWorkSize"),
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 5, "pSize", "pLocalWorkSize"),
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueNDRangeKernel", 7, "pEvent", "pEventWaitList"),

      // clEnqueueTask
      FunctionParameterNameOverride("clEnqueueTask", 2, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueTask", 3, "pEvent", "pEventWaitList"),

      // clEnqueueNativeKernel
      FunctionParameterNameOverride("clEnqueueNativeKernel", 2, "pVoid", "pArgs"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 3, "size", "cbArgs"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 4, "uint", "numMemObjects"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 5, "pMem", "pMemList"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 6, "pVoid", "ppArgsMemLoc"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 7, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueNativeKernel", 8, "pEvent", "pEventWaitList"),

      // clCompileProgram
      FunctionParameterNameOverride("clCompileProgram", 1, "uint", "numDevices"),
      FunctionParameterNameOverride("clCompileProgram", 2, "pDeviceId", "pDeviceList"),
      FunctionParameterNameOverride("clCompileProgram", 3, "pChar", "pOptions"),
      FunctionParameterNameOverride("clCompileProgram", 4, "uint", "numInputHeaders"),
      FunctionParameterNameOverride("clCompileProgram", 5, "pProgram", "pInputHeaders"),
      FunctionParameterNameOverride("clCompileProgram", 6, "pChar", "ppHeaderIncludeNames"),
      FunctionParameterNameOverride("clCompileProgram", 8, "pVoid", "pUserData"),

      // clLinkProgram
      FunctionParameterNameOverride("clLinkProgram", 1, "uint", "numDevices"),
      FunctionParameterNameOverride("clLinkProgram", 2, "pDeviceId", "pDeviceList"),
      FunctionParameterNameOverride("clLinkProgram", 3, "pChar", "pOptions"),
      FunctionParameterNameOverride("clLinkProgram", 4, "uint", "numInputPrograms"),
      FunctionParameterNameOverride("clLinkProgram", 5, "pProgram", "pInputPrograms"),
      FunctionParameterNameOverride("clLinkProgram", 7, "pVoid", "pUserData"),
      FunctionParameterNameOverride("clLinkProgram", 9, "int", "pErrorCode"),

      // clEnqueueFillBuffer 
      FunctionParameterNameOverride("clEnqueueFillBuffer", 1, "mem", "buffer"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 2, "pVoid", "pPattern"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 3, "size", "patternSize"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 4, "size", "offset"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 5, "size", "size"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 6, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueFillBuffer", 7, "pEvent", "pEventWaitList"),

      // clEnqueueFillImage 
      FunctionParameterNameOverride("clEnqueueFillImage", 1, "mem", "image"),
      FunctionParameterNameOverride("clEnqueueFillImage", 2, "pVoid", "pFillColor"),
      FunctionParameterNameOverride("clEnqueueFillImage", 3, "pSize", "pOrigin"),
      FunctionParameterNameOverride("clEnqueueFillImage", 4, "pSize", "pRegion"),
      FunctionParameterNameOverride("clEnqueueFillImage", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueFillImage", 6, "pEvent", "pEventWaitList"),

      // clEnqueueMigrateMemObjects 
      FunctionParameterNameOverride("clEnqueueMigrateMemObjects", 1, "uint", "numMemObjects"),
      FunctionParameterNameOverride("clEnqueueMigrateMemObjects", 2, "pMem", "pMemObjects"),
      FunctionParameterNameOverride("clEnqueueMigrateMemObjects", 4, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueMigrateMemObjects", 5, "pEvent", "pEventWaitList"),

      // clEnqueueMarkerWithWaitList
      FunctionParameterNameOverride("clEnqueueMarkerWithWaitList", 1, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueMarkerWithWaitList", 2, "pEvent", "pEventWaitList"),

      // clEnqueueBarrierWithWaitList
      FunctionParameterNameOverride("clEnqueueBarrierWithWaitList", 1, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueBarrierWithWaitList", 2, "pEvent", "pEventWaitList"),

      // clEnqueueSVMFree
      FunctionParameterNameOverride("clEnqueueSVMFree", 1, "uint", "numSvmPointers"),
      FunctionParameterNameOverride("clEnqueueSVMFree", 2, "**CXTypeIncompleteArray**", "pSvmPointers"),
      FunctionParameterNameOverride("clEnqueueSVMFree", 4, "pVoid", "pUserData"),
      FunctionParameterNameOverride("clEnqueueSVMFree", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMFree", 6, "pEvent", "pEventWaitList"),

      // clEnqueueSVMMemcpy 
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 1, "bool", "blockingCopy"),
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 2, "pVoid", "pDst"),
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 3, "pVoid", "pSrc"),
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 4, "size", "size"),
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMMemcpy", 6, "pEvent", "pEventWaitList"),

      // clEnqueueSVMMemFill 
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 1, "pVoid", "pSvm"),
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 2, "pVoid", "pPattern"),
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 3, "size", "patternSize"),
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 4, "size", "size"),
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMMemFill", 6, "pEvent", "pEventWaitList"),

      // clEnqueueSVMMap
      FunctionParameterNameOverride("clEnqueueSVMMap", 1, "bool", "blockingMap"),
      FunctionParameterNameOverride("clEnqueueSVMMap", 3, "pVoid", "pSvm"),
      FunctionParameterNameOverride("clEnqueueSVMMap", 4, "size", "size"),
      FunctionParameterNameOverride("clEnqueueSVMMap", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMMap", 6, "pEvent", "pEventWaitList"),

      // clEnqueueSVMUnmap
      FunctionParameterNameOverride("clEnqueueSVMUnmap", 1, "pVoid", "pSvm"),
      FunctionParameterNameOverride("clEnqueueSVMUnmap", 2, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMUnmap", 3, "pEvent", "pEventWaitList"),

      // clGetDeviceAndHostTimer
      FunctionParameterNameOverride("clGetDeviceAndHostTimer", 1, "pUlong", "pDeviceTimestamp"),
      FunctionParameterNameOverride("clGetDeviceAndHostTimer", 2, "pUlong", "pHostTimestamp"),

      // clGetKernelSubGroupInfo
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 1, "deviceId", "device"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 2, "kernelSubGroupInfo", "paramName"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 3, "size", "inputValueSize"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 4, "pVoid", "pInputValue"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 5, "size", "paramValueSize"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 6, "pVoid", "pParamValue"),
      FunctionParameterNameOverride("clGetKernelSubGroupInfo", 7, "pSize", "pParamValueSizeRet"),

      // clEnqueueSVMMigrateMem
      FunctionParameterNameOverride("clEnqueueSVMMigrateMem", 1, "uint", "numSvmPointers"),
      FunctionParameterNameOverride("clEnqueueSVMMigrateMem", 2, "pVoid", "pSvmPointers"),
      FunctionParameterNameOverride("clEnqueueSVMMigrateMem", 3, "pSize", "pSizes"),
      FunctionParameterNameOverride("clEnqueueSVMMigrateMem", 5, "uint", "numEventsInWaitList"),
      FunctionParameterNameOverride("clEnqueueSVMMigrateMem", 6, "pEvent", "pEventWaitList"),
    };


    // This crap is necessary because I have been unable to get the 'exact written code' from clang and
    // because OpenCL decided not to use a typedef for its function pointers
    const std::vector<FunctionParameterTypeOverride> g_functionParameterTypeOverride
    {
      // clCreateContextFromType
      FunctionParameterTypeOverride("clCreateContextFromType", 2, "void (*)(const char *, const void *, size_t, void *) __attribute__((stdcall))", "FNOpenCLNotify"),
      // clCreateContext
      FunctionParameterTypeOverride("clCreateContext", 3, "void (*)(const char *, const void *, size_t, void *) __attribute__((stdcall))", "FNOpenCLNotify"),
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
    : SimpleGenerator(ModCapture(capture), SimpleGeneratorConfig(basicConfig, g_functionPairs, g_manualFunctionMatches, g_classFunctionAbsorbtion, g_typeDefaultValues, g_forceNullParameter, TYPE_NAME_PREFIX, false), templateRoot, dstPath)
  {
  }


  CaptureConfig OpenCLGenerator::GetCaptureConfig()
  {
    std::deque<std::string> filters;
    //filters.push_back(CREATE_FUNCTION);
    //filters.push_back(DESTROY_FUNCTION);
    return CaptureConfig(TYPE_NAME_PREFIX, filters, g_functionParameterNameOverride, g_functionParameterTypeOverride, true);
  }
}
