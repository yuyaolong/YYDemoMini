#include "yyPlyParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "cppUtils.h"
#include "YYLog.h"

static bool CheckLine(std::ifstream& plyFile, const std::string& validLine)
{
    std::string line;
    return std::getline(plyFile, line) && line == validLine;
}

static bool GetNextPlyLine(std::istringstream& plyFile, std::string& lineOut)
{
    while (std::getline(plyFile, lineOut))
    {
        // skip comment lines
        if (lineOut.find("comment", 0) != 0)
        {
            return true;
        }
    }
    return false;
}

bool YYPlyParser::Parse(std::istringstream& plyFile)
{
    // validate start of header
    std::string token1, token2, token3;

    // check header starts with "ply".
    if (!GetNextPlyLine(plyFile, token1))
    {
        YYLog::E("Unexpected error reading next line\n");
        return false;
    }
    if (token1 != "ply")
    {
        YYLog::E("Invalid ply file\n");
        return false;
    }

    // check format
    if (!GetNextPlyLine(plyFile, token1))
    {
        YYLog::E("Unexpected error reading next line\n");
        return false;
    }
    if (token1 != "format binary_little_endian 1.0" && token1 != "format binary_big_endian 1.0")
    {
        YYLog::E("Invalid ply file, expected format\n");
        return false;
    }
    if (token1 != "format binary_little_endian 1.0")
    {
        YYLog::E("Unsupported ply file, only binary_little_endian supported\n");
        return false;
    }

    // parse "element vertex {number}"
    std::string line;
    if (!GetNextPlyLine(plyFile, line))
    {
        YYLog::E("Unexpected error reading next line\n");
        return false;
    }
    std::istringstream iss(line);
    if (!((iss >> token1 >> token2 >> vertexCount) && (token1 == "element") && (token2 == "vertex")))
    {
        YYLog::E("Invalid ply file, expected \"element vertex {number}\"\n");
        return false;
    }

    // TODO: support other "element" types faces, edges etc?
    // at the moment I only care about ply files with vertex elements.

    size_t offset = 0;
    while (true)
    {
        if (!GetNextPlyLine(plyFile, line))
        {
            YYLog::E("unexpected error reading line\n");
            return false;
        }

        if (line == "end_header")
        {
            break;
        }

        iss.str(line);
        iss.clear();
        iss >> token1 >> token2 >> token3;
        if (token1 != "property")
        {
            YYLog::E("Invalid header, expected property\n");
            return false;
        }
        if (token2 == "char" || token2 == "int8")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 1, YYPlyParser::Type::Char}));
            offset += 1;
        }
        else if (token2 == "uchar" || token2 == "uint8")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 1, YYPlyParser::Type::UChar}));
            offset += 1;
        }
        else if (token2 == "short" || token2 == "int16")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 2, YYPlyParser::Type::Short}));
            offset += 2;
        }
        else if (token2 == "ushort" || token2 == "uint16")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 2, YYPlyParser::Type::UShort}));
            offset += 2;
        }
        else if (token2 == "int" || token2 == "int32")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 4, YYPlyParser::Type::Int}));
            offset += 4;
        }
        else if (token2 == "uint" || token2 == "uint32")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 4, YYPlyParser::Type::UInt}));
            offset += 4;
        }
        else if (token2 == "float" || token2 == "float32")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 4, YYPlyParser::Type::Float}));
            offset += 4;
        }
        else if (token2 == "double" || token2 == "float64")
        {
            propertyMap.emplace(std::pair<std::string, Property>(token3, {offset, 8, YYPlyParser::Type::Double}));
            offset += 8;
        }
        else
        {
            YYLog::E("Unsupported type \"%s\" for property \"%s\"\n", token2.c_str(), token3.c_str());
            return false;
        }
    }

    vertexSize = offset;

    // read rest of file into dataVec
    dataVec.resize(vertexSize * vertexCount);
    plyFile.read((char*)dataVec.data(), vertexSize * vertexCount);

    return true;
}

bool YYPlyParser::GetProperty(const std::string& key, YYPlyParser::Property& propertyOut) const
{
    auto iter = propertyMap.find(key);
    if (iter != propertyMap.end())
    {
        propertyOut = iter->second;
        return true;
    }
    return false;
}

void YYPlyParser::ForEachVertex(const VertexCallback& cb)
{
    const uint8_t* vertexPtr = dataVec.data();
    for (size_t i = 0; i < vertexCount; i++)
    {
        cb(vertexPtr, vertexSize);
        vertexPtr += vertexSize;
    }
}