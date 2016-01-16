#pragma once

#include <memory>
#include <string>

class TriangleMesh;

std::unique_ptr<TriangleMesh> LoadTriangleMesh(const std::string& fileName);
