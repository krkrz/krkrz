
// ANGLE の shader_utils.h から吉里吉里Z用にコピーと改変
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tjsCommHead.h"

#include "GLShaderUtil.h"
#include "DebugIntf.h"
#include "OpenGLScreen.h"

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glCreateShader" ) );

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glShaderSource" ) );
    glCompileShader(shader);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glCompileShader" ) );

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetShaderiv" ) );

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetShaderiv" ) );

        // Info log length includes the null terminator, so 1 means that the info log is an empty
        // string.
        if (infoLogLength > 1)
        {
            std::vector<GLchar> infoLog(infoLogLength);
            glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), NULL, &infoLog[0]);
			tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetShaderInfoLog" ) );
            TVPAddLog( TJS_W("GL : shader compilation failed:") + ttstr(&infoLog[0])  );
        }
        else
        {
            TVPAddLog( TJS_W("GL : shader compilation failed. <Empty log message>") );
        }

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CheckLinkStatusAndReturnProgram(GLuint program, bool outputErrorMessages)
{
    if (glGetError() != GL_NO_ERROR)
        return 0;

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetProgramiv" ) );
    if (linkStatus == 0)
    {
        if (outputErrorMessages)
        {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetProgramiv" ) );

            // Info log length includes the null terminator, so 1 means that the info log is an
            // empty string.
            if (infoLogLength > 1)
            {
                std::vector<GLchar> infoLog(infoLogLength);
                glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr,
                                    &infoLog[0]);
				tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glGetProgramInfoLog" ) );

                TVPAddLog( TJS_W("GL : program link failed: ") + ttstr(&infoLog[0]) );
            }
            else
            {
            	TVPAddLog( TJS_W("GL : program link failed. <Empty log message>") );
            }
        }

        glDeleteProgram(program);
        return 0;
    }

    return program;
}
GLuint CompileProgramWithTransformFeedback(
    const std::string &vsSource,
    const std::string &fsSource,
    const std::vector<std::string> &transformFeedbackVaryings,
    GLenum bufferMode)
{
    GLuint program = glCreateProgram();
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W("glCreateProgram") );

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glAttachShader" ) );
    glDeleteShader(vs);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glDeleteShader" ) );

    glAttachShader(program, fs);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glAttachShader" ) );
    glDeleteShader(fs);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glDeleteShader" ) );

    if (transformFeedbackVaryings.size() > 0)
    {
        std::vector<const char *> constCharTFVaryings;

        for (const std::string &transformFeedbackVarying : transformFeedbackVaryings)
        {
            constCharTFVaryings.push_back(transformFeedbackVarying.c_str());
        }

        glTransformFeedbackVaryings(program, static_cast<GLsizei>(transformFeedbackVaryings.size()),
                                    &constCharTFVaryings[0], bufferMode);
		tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glTransformFeedbackVaryings" ) );
    }

    glLinkProgram(program);
	tTVPOpenGLScreen::CheckGLErrorAndLog( TJS_W( "glLinkProgram" ) );

    return CheckLinkStatusAndReturnProgram(program, true);
}
GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    std::vector<std::string> emptyVector;
    return CompileProgramWithTransformFeedback(vsSource, fsSource, emptyVector, GL_NONE);
}
/*
GLuint CompileProgramFromFiles(const std::string &vsPath, const std::string &fsPath)
{
    std::string vsSource = ReadFileToString(vsPath);
    std::string fsSource = ReadFileToString(fsPath);
    if (vsSource.empty() || fsSource.empty())
    {
        return 0;
    }

    return CompileProgram(vsSource, fsSource);
}
*/
/*
// Compute Shader is OpenGL ES 3.1 or later
GLuint CompileComputeProgram(const std::string &csSource, bool outputErrorMessages)
{
    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, csSource);
    if (cs == 0)
    {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, cs);

    glLinkProgram(program);

    return CheckLinkStatusAndReturnProgram(program, outputErrorMessages);
}
*/
/*
GLuint LoadBinaryProgramOES(const std::vector<uint8_t> &binary, GLenum binaryFormat)
{
    GLuint program = glCreateProgram();
    glProgramBinaryOES(program, binaryFormat, binary.data(), static_cast<GLint>(binary.size()));
    return CheckLinkStatusAndReturnProgram(program, true);
}
*/
GLuint LoadBinaryProgramES3(const std::vector<uint8_t> &binary, GLenum binaryFormat)
{
    GLuint program = glCreateProgram();
    glProgramBinary(program, binaryFormat, binary.data(), static_cast<GLint>(binary.size()));
    return CheckLinkStatusAndReturnProgram(program, true);
}
