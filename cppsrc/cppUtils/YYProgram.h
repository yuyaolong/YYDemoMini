//
// Created by yuyao on 2024/3/12.
//

#pragma once

#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include "YYLog.h"

class YYProgram {
public:
    YYProgram(void* assetMgr, bool addMacro = true);
    ~YYProgram();

    unsigned int GetProgram() { return program;  }

    // used to inject #defines or other code into shaders
    // AddMacro("FOO", "BAR");
    // will replace the string /*%%FOO%%*/ in the source shader with BAR
    void AddMacro(const std::string& key, const std::string& value);

    bool LoadVertFrag(const std::string& vertFilename, const std::string& fragFilename);
    bool LoadVertGeomFrag(const std::string& vertFilename, const std::string& geomFilename, const std::string& fragFilename);
    bool LoadCompute(const std::string& computeFilename);
    void Bind() const;

    int GetUniformLoc(const std::string& name) const;
    int GetAttribLoc(const std::string& name) const;

    template <typename T>
    void SetUniform(const std::string& name, T value) const
    {
        auto iter = uniforms.find(name);
        if (iter != uniforms.end())
        {
            SetUniformRaw(iter->second.loc, value);
        }
        else
        {
            YYLog::W("Could not find uniform \"%s\" for program \"%s\"\n", name.c_str(), debugName.c_str());
        }
    }

    template <typename T>
    void SetAttrib(const std::string& name, T* values, size_t stride = 0) const
    {
        auto iter = attribs.find(name);
        if (iter != attribs.end())
        {
            SetAttrib(iter->second.loc, values, stride);
        }
        else
        {
            YYLog::W("Could not find attrib \"%s\" for program \"%s\"\n", name.c_str(), debugName.c_str());
        }
    }

    void SetAttrib(int loc, float* values, size_t stride = 0) const;
    void SetAttrib(int loc, glm::vec2* values, size_t stride = 0) const;
    void SetAttrib(int loc, glm::vec3* values, size_t stride = 0) const;
    void SetAttrib(int loc, glm::vec4* values, size_t stride = 0) const;

protected:

    void SetUniformRaw(int loc, int32_t value) const;
    void SetUniformRaw(int loc, uint32_t value) const;
    void SetUniformRaw(int loc, float value) const;
    void SetUniformRaw(int loc, const glm::vec2& value) const;
    void SetUniformRaw(int loc, const glm::vec3& value) const;
    void SetUniformRaw(int loc, const glm::vec4& value) const;
    void SetUniformRaw(int loc, const glm::mat2& value) const;
    void SetUniformRaw(int loc, const glm::mat3& value) const;
    void SetUniformRaw(int loc, const glm::mat4& value) const;

    void Delete();
    bool CheckLinkStatus();

    int program = 0;
    int vertShader = 0;
    int geomShader = 0;
    int fragShader = 0;
    int computeShader = 0;
    void* mAssertMgr = nullptr;

    struct Variable
    {
        int size;
        uint32_t type;
        int loc;
    };

    std::unordered_map<std::string, Variable> uniforms;
    std::unordered_map<std::string, Variable> attribs;
    std::vector<std::pair<std::string, std::string>> macros;
    std::string debugName;
private:
    bool LoadFile(const std::string& filename, std::string& data);
};

