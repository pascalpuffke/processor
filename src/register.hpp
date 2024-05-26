#pragma once

#include <spec.hpp>
#include <fmt/core.h>

struct Register {
    ProcessorSpec::reg_t reg { 0 };
    constexpr explicit Register(ProcessorSpec::reg_t r)
        : reg(r) {
    }
    constexpr operator ProcessorSpec::reg_t() const { return reg; }
};

template <>
struct fmt::formatter<Register> : formatter<ProcessorSpec::reg_t> {
    auto format(Register r, format_context& ctx) const {
        return formatter<ProcessorSpec::reg_t>::format(r.reg, ctx);
    }
};