#pragma once

#include <span>
#include <string>
#include <vector>

#include <instructions.hpp>

class Disassembler final {
public:
    [[nodiscard]] static auto disassemble(std::span<const insr_t> code) -> std::vector<std::string>;
};