#pragma once

#include <span>
#include <string>
#include <vector>

#include <instructions.hpp>

class Disassembler final {
public:
    [[nodiscard]] static auto disassemble(std::span<const ProcessorSpec::insr_t> code) -> std::vector<std::string>;
};