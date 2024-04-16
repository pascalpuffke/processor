#pragma once

#include <string>
#include <vector>

#include <instructions.hpp>

class Assembler final {
public:
    [[nodiscard]] static auto assemble(const std::string& source) -> std::vector<insr_t>;
};