#pragma once

#include <spec.hpp>
#include <fmt/core.h>

struct Immediate {
    ProcessorSpec::imm_t imm { 0 };
    constexpr explicit Immediate(ProcessorSpec::imm_t i)
        : imm(i) {
    }
    constexpr operator ProcessorSpec::imm_t() const { return imm; }
};

template <>
struct fmt::formatter<Immediate> : formatter<ProcessorSpec::imm_t> {
    auto format(Immediate i, format_context& ctx) const {
        return formatter<ProcessorSpec::imm_t>::format(i.imm, ctx);
    }
};
