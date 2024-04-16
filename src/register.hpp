#pragma once

#include <types.hpp>
#include <fmt/core.h>

struct Register {
    using reg_t = u8;
    reg_t reg { 0 };
    constexpr explicit Register(reg_t r)
        : reg(r) {
    }
    constexpr operator reg_t() const { return reg; }
};

template <>
struct fmt::formatter<Register> : formatter<u8> {
    auto format(Register r, format_context& ctx) const {
        return formatter<u8>::format(r.reg, ctx);
    }
};