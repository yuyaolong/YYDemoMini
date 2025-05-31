//
// Created by yuyao on 2024/3/12.
//

#include "YYProgram.h"
#include "YYPlatform.h"
#include <sstream>
#include <fstream>

static std::string ExpandMacros(std::vector<std::pair<std::string, std::string>> macros, const std::string& source)
{
    std::string result = source;

    for (const auto& macro : macros)
    {
        std::string::size_type pos = 0;
        while ((pos = result.find(macro.first, pos)) != std::string::npos)
        {
            result.replace(pos, macro.first.length(), macro.second);
            // Move past the last replaced position
            pos += macro.second.length();
        }
    }

    return result;
}

static void DumpShaderSource(const std::string& source)
{
    std::stringstream ss(source);
    std::string line;
    int i = 1;
    while (std::getline(ss, line))
    {
        YYLog::D("%04d: %s\n", i, line.c_str());
        i++;
    }
    YYLog::D("\n");
}

static bool CompileShader(GLenum type, const std::string& source, GLint* shaderOut, const std::string& debugName)
{
    GLint shader = glCreateShader(type);
    int size = static_cast<int>(source.size());
    const GLchar* sourcePtr = source.c_str();
    glShaderSource(shader, 1, (const GLchar**)&sourcePtr, &size);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        YYLog::E("shader compilation error for \"%s\"!\n", debugName.c_str());
    }

    GLint bufferLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufferLen);
    if (bufferLen > 1)
    {
        if (compiled)
        {
            YYLog::E("shader compilation warning for \"%s\"!\n", debugName.c_str());
        }

        GLsizei len = 0;
        std::unique_ptr<char> buffer(new char[bufferLen]);
        glGetShaderInfoLog(shader, bufferLen, &len, buffer.get());
        YYLog::E("%s\n", buffer.get());
        DumpShaderSource(source);
    }

#ifdef WARNINGS_AS_ERRORS
    if (!compiled || bufferLen > 1)
#else
    if (!compiled)
#endif
    {
        return false;
    }

    *shaderOut = shader;
    return true;
}

YYProgram::YYProgram(void* assetMgr, bool addMacro){
    YYLog::D("program created");
#ifdef __ANDROID__
    if (addMacro)
    {
        AddMacro("HEADER", "#version 320 es\n");
    }
    mAssertMgr = static_cast<AAssetManager*>(assetMgr);
#elif defined _WIN32
    if (addMacro)
    {
        AddMacro("HEADER", "#version 460\n");
    }
#endif
}


YYProgram::~YYProgram()
{
    YYLog::D("program destroy");
    Delete();
}

void YYProgram::AddMacro(const std::string& key, const std::string& value)
{
    // In order to keep the glsl code compiling if the macro is not applied.
    // the key is enclosed in a c-style comment and double %.
    std::string token = "/*%%" + key + "%%*/";
    macros.push_back(std::pair<std::string, std::string>(token, value));
}

bool YYProgram::LoadVertFrag(const std::string& vertFilename, const std::string& fragFilename)
{
    return LoadVertGeomFrag(vertFilename, std::string(), fragFilename);
}

bool YYProgram::LoadVertGeomFrag(const std::string& vertFilename, const std::string& geomFilename, const std::string& fragFilename)
{
    // Delete old shader/program
    Delete();

    const bool useGeomShader = !geomFilename.empty();

    if (useGeomShader)
    {
        debugName = vertFilename + " + " + geomFilename + " + " + fragFilename;
    }
    else
    {
        debugName = vertFilename + " + " + fragFilename;
    }

    std::string vertSource, geomSource, fragSource;
    if (!LoadFile(vertFilename, vertSource))
    {
        YYLog::E("Failed to load vertex shader %s\n", vertFilename.c_str());
        return false;
    }
    vertSource = ExpandMacros(macros, vertSource);

    if (useGeomShader)
    {
        if (!LoadFile(geomFilename, geomSource))
        {
            YYLog::E("Failed to load geometry shader %s\n", geomFilename.c_str());
            return false;
        }
        geomSource = ExpandMacros(macros, geomSource);
    }

    if (!LoadFile(fragFilename, fragSource))
    {
        YYLog::E("Failed to load fragment shader \"%s\"\n", fragFilename.c_str());
        return false;
    }
    fragSource = ExpandMacros(macros, fragSource);

    if (!CompileShader(GL_VERTEX_SHADER, vertSource, &vertShader, vertFilename))
    {
        YYLog::E("Failed to compile vertex shader \"%s\"\n", vertFilename.c_str());
        return false;
    }

    if (useGeomShader)
    {
        geomSource = ExpandMacros(macros, geomSource);
        if (!CompileShader(GL_GEOMETRY_SHADER, geomSource, &geomShader, geomFilename))
        {
            YYLog::E("Failed to compile geometry shader \"%s\"\n", geomFilename.c_str());
            return false;
        }
    }

    if (!CompileShader(GL_FRAGMENT_SHADER, fragSource, &fragShader, fragFilename))
    {
        YYLog::E("Failed to compile fragment shader \"%s\"\n", fragFilename.c_str());
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    if (useGeomShader)
    {
        glAttachShader(program, geomShader);
    }
    glLinkProgram(program);

    if (!CheckLinkStatus())
    {
        YYLog::E("Failed to link program \"%s\"\n", debugName.c_str());

        // dump shader source for reference
        YYLog::D("\n");
        YYLog::D("%s =\n", vertFilename.c_str());
        DumpShaderSource(vertSource);
        if (useGeomShader)
        {
            YYLog::D("%s =\n", geomFilename.c_str());
            DumpShaderSource(geomSource);
        }
        YYLog::D("%s =\n", fragFilename.c_str());
        DumpShaderSource(fragSource);

        return false;
    }

    const int MAX_NAME_SIZE = 1028;
    static char name[MAX_NAME_SIZE];

    GLint numAttribs;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttribs);
    for (int i = 0; i < numAttribs; ++i)
    {
        Variable v;
        GLsizei strLen;
        glGetActiveAttrib(program, i, MAX_NAME_SIZE, &strLen, &v.size, &v.type, name);
        v.loc = glGetAttribLocation(program, name);
        attribs[name] = v;
    }

    GLint numUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (int i = 0; i < numUniforms; ++i)
    {
        Variable v;
        GLsizei strLen;
        glGetActiveUniform(program, i, MAX_NAME_SIZE, &strLen, &v.size, &v.type, name);
        int loc = glGetUniformLocation(program, name);
        v.loc = loc;
        uniforms[name] = v;
    }

    return true;
}

bool YYProgram::LoadCompute(const std::string& computeFilename)
{
    // Delete old shader/program
    Delete();

    debugName = computeFilename;

    std::string computeSource;
    if (!LoadFile(computeFilename, computeSource))
    {
        YYLog::E("Failed to load compute shader \"%s\"\n", computeFilename.c_str());
        return false;
    }

    computeSource = ExpandMacros(macros, computeSource);
    if (!CompileShader(GL_COMPUTE_SHADER, computeSource, &computeShader, computeFilename))
    {
        YYLog::E("Failed to compile compute shader \"%s\"\n", computeFilename.c_str());
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    if (!CheckLinkStatus())
    {
        YYLog::E("Failed to link program \"%s\"\n", debugName.c_str());

        // dump shader source for reference
        YYLog::D("\n");
        YYLog::D("%s =\n", computeFilename.c_str());
        DumpShaderSource(computeSource);

        return false;
    }

    const int MAX_NAME_SIZE = 1028;
    static char name[MAX_NAME_SIZE];

    GLint numUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (int i = 0; i < numUniforms; ++i)
    {
        Variable v;
        GLsizei strLen;
        glGetActiveUniform(program, i, MAX_NAME_SIZE, &strLen, &v.size, &v.type, name);
        int loc = glGetUniformLocation(program, name);
        v.loc = loc;
        uniforms[name] = v;
    }

    // TODO: build reflection info on shader storage blocks

    return true;
}

void YYProgram::Bind() const
{
    glUseProgram(program);
}

int YYProgram::GetUniformLoc(const std::string& name) const
{
    auto iter = uniforms.find(name);
    if (iter != uniforms.end())
    {
        return iter->second.loc;
    }
    else
    {
        assert(false);
        YYLog::W("Could not find uniform \"%s\" for program \"%s\"\n", name.c_str(), debugName.c_str());
        return 0;
    }
}

int YYProgram::GetAttribLoc(const std::string& name) const
{
    auto iter = attribs.find(name);
    if (iter != attribs.end())
    {
        return iter->second.loc;
    }
    else
    {
        YYLog::W("Could not find attrib \"%s\" for program \"%s\"\n", name.c_str(), debugName.c_str());
        assert(false);
        return 0;
    }
}

void YYProgram::SetUniformRaw(int loc, int32_t value) const
{
    glUniform1i(loc, value);
}

void YYProgram::SetUniformRaw(int loc, uint32_t value) const
{
    glUniform1ui(loc, value);
}

void YYProgram::SetUniformRaw(int loc, float value) const
{
    glUniform1f(loc, value);
}

void YYProgram::SetUniformRaw(int loc, const glm::vec2& value) const
{
    glUniform2fv(loc, 1, (float*)&value);
}

void YYProgram::SetUniformRaw(int loc, const glm::vec3& value) const
{
    glUniform3fv(loc, 1, (float*)&value);
}

void YYProgram::SetUniformRaw(int loc, const glm::vec4& value) const
{
    glUniform4fv(loc, 1, (float*)&value);
}

void YYProgram::SetUniformRaw(int loc, const glm::mat2& value) const
{
    glUniformMatrix2fv(loc, 1, GL_FALSE, (float*)&value);
}

void YYProgram::SetUniformRaw(int loc, const glm::mat3& value) const
{
    glUniformMatrix3fv(loc, 1, GL_FALSE, (float*)&value);
}

void YYProgram::SetUniformRaw(int loc, const glm::mat4& value) const
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&value);
}

void YYProgram::SetAttrib(int loc, float* values, size_t stride) const
{
    glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, (GLsizei)stride, values);
    glEnableVertexAttribArray(loc);
}

void YYProgram::SetAttrib(int loc, glm::vec2* values, size_t stride) const
{
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, values);
    glEnableVertexAttribArray(loc);
}

void YYProgram::SetAttrib(int loc, glm::vec3* values, size_t stride) const
{
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, values);
    glEnableVertexAttribArray(loc);
}

void YYProgram::SetAttrib(int loc, glm::vec4* values, size_t stride) const
{
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride, values);
    glEnableVertexAttribArray(loc);
}

void YYProgram::Delete()
{
    debugName = "";

    if (vertShader > 0)
    {
        glDeleteShader(vertShader);
        vertShader = 0;
    }

    if (geomShader > 0)
    {
        glDeleteShader(geomShader);
        geomShader = 0;
    }

    if (fragShader > 0)
    {
        glDeleteShader(fragShader);
        fragShader = 0;
    }

    if (computeShader > 0)
    {
        glDeleteShader(computeShader);
        computeShader = 0;
    }

    if (program > 0)
    {
        glDeleteProgram(program);
        program = 0;
    }

    uniforms.clear();
    attribs.clear();
}

bool YYProgram::CheckLinkStatus()
{
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        YYLog::E("Failed to link shaders \"%s\"\n", debugName.c_str());
    }

    const GLint MAX_BUFFER_LEN = 4096;
    GLsizei bufferLen = 0;
    std::unique_ptr<char> buffer(new char[MAX_BUFFER_LEN]);
    glGetProgramInfoLog(program, MAX_BUFFER_LEN, &bufferLen, buffer.get());
    if (bufferLen > 0)
    {
        if (linked)
        {
            YYLog::W("Warning during linking shaders \"%s\"\n", debugName.c_str());
        }
        YYLog::W("%s\n", buffer.get());
    }

#ifdef WARNINGS_AS_ERRORS
    if (!linked || bufferLen > 1)
#else
    if (!linked)
#endif
    {
        return false;
    }

    return true;
}

bool YYProgram::LoadFile(const std::string& filename, std::string& data) {
    // open shader file
    #ifdef __ANDROID__
        assert(mAssertMgr != nullptr);
        AAsset* shaderAssetFile = AAssetManager_open(static_cast<AAssetManager*>(mAssertMgr), filename.c_str(), AASSET_MODE_BUFFER);
        if (!shaderAssetFile) {return false;}
        size_t shaderBufferLen = AAsset_getLength(shaderAssetFile);
        //  additional terminating null-character ('\0') at the end automatically when c_str() return
        data = std::string( static_cast<const char*>(AAsset_getBuffer(shaderAssetFile)), shaderBufferLen);
        AAsset_close(shaderAssetFile);
    #elif defined _WIN32
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            YYLog::E("Failed to open file: %s\n", filename.c_str());
            return false;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            YYLog::E("Failed to read file: %s\n", filename.c_str());
            return false;
        }

        data.assign(buffer.begin(), buffer.end());
    #endif
    return true;
}
