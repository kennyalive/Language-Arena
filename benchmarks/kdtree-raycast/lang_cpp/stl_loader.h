#pragma once

#include <memory>
#include <string>

class TriangleMeshLoadingException : public std::exception
{
public:
    TriangleMeshLoadingException(const std::string& message);
    const char* what() const override;

private:
    const std::string _message;
};

class TriangleMesh;

std::unique_ptr<TriangleMesh> loadStlFile(const std::string& fileName);
