#pragma once

#include <cstddef>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long;

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);

using f32 = float;
using f64 = double;

static_assert(sizeof(std::size_t) == 8);
using usize = std::size_t;
static_assert(sizeof(std::ptrdiff_t) == 8);
using isize = std::ptrdiff_t;