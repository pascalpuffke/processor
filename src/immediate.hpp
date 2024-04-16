#pragma once

#include <types.hpp>
#include <fmt/core.h>

struct Immediate {
    using imm_t = u8;
    imm_t imm { 0 };
    constexpr explicit Immediate(imm_t i)
        : imm(i) {
    }
    constexpr operator imm_t() const { return imm; }
};

template <>
struct fmt::formatter<Immediate> : formatter<u8> {
    auto format(Immediate i, format_context& ctx) const {
        return formatter<u8>::format(i.imm, ctx);
    }
};