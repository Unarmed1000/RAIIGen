# RAIIGen
Scan Vulkan, OpenCL, OpenGLES and OpenVX headers and generate useful C++11 RAII classes.

Experimental project to experiment a bit with libclang and generate some useful RAII classes.
The main goal of this is to experiment and see whats possible, so nothing has been polished. The end goal is just to have something that is 'good enough'.

All forked versions of the code must include the original "##AG_TOOL_STATEMENT##" line in the generated code.
This means that all generated code contains a line like this:

// Auto-generated OpenCL 1.1 C++11 RAII classes by RAIIGen (https://github.com/Unarmed1000)

The exact format depends on which API and tool version was used.

All released auto generated files must include the corrosponding 'RAIIGenVersion.txt' file.
All forked versions of the code must leave the original first line of 'RAIIGenVersion.txt' intact, but are free to add additional lines to the file.

LICENSE: BSD 3-Clause License
