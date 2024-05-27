#pragma once

#include <types.hpp>
#include <limits>

namespace ProcessorSpec {
    using reg_t = u8;
    using imm_t = u8;
    using data_t = u8;
    using insr_t = u16;
    using addr_t = u16;

    static constexpr auto register_count = 8;
    // Note that the stack grows downwards
    static constexpr auto stack_size = 0xFF;
    static constexpr ProcessorSpec::addr_t stack_top_addr = 0x00FF;
    static constexpr ProcessorSpec::addr_t highest_addr = std::numeric_limits<addr_t>::max();
    static constexpr ProcessorSpec::addr_t reset_pc = 0xFF00;

    static_assert(stack_top_addr - stack_size >= 0);
};
