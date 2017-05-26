
// ANGLE の shader_utils.h から吉里吉里Z用にコピーと改変
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#ifndef GLShaderUtilH
#define GLShaderUtilH

#include "OpenGLHeader.h"

#include <string>
#include <vector>

#define SHADER_SOURCE(...) #__VA_ARGS__

extern GLuint CompileShader(GLenum type, const std::string &source);
//extern GLuint CompileShaderFromFile(GLenum type, const std::string &sourcePath);
/*
GLuint CompileProgramWithTransformFeedback(const std::string &vsSource,
                                    const std::string &fsSource,
                                    const std::vector<std::string> &transformFeedbackVaryings,
                                    GLenum bufferMode);
*/
extern GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource);
//extern GLuint CompileProgramFromFiles(const std::string &vsPath, const std::string &fsPath);
//extern GLuint CompileComputeProgram(const std::string &csSource, bool outputErrorMessages = true);

//extern GLuint LoadBinaryProgramOES(const std::vector<uint8_t> &binary, GLenum binaryFormat);
extern GLuint LoadBinaryProgramES3(const std::vector<uint8_t> &binary, GLenum binaryFormat);




#endif