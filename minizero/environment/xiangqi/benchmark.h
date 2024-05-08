#pragma once
#ifndef BENCHMARK_H_INCLUDED
#define BENCHMARK_H_INCLUDED

#include <iosfwd>
#include <string>
#include <vector>

namespace minizero::env::xiangqi {

class Position;

std::vector<std::string> setup_bench(const Position&, std::istream&);

} // namespace minizero::env::xiangqi

#endif // #ifndef BENCHMARK_H_INCLUDED
