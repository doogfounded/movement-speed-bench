#include "movement_models.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

using clock_type = std::chrono::steady_clock;
volatile float observable_sink = 0.0f; // makes the computed positions observable to the optimizer

struct Options { std::size_t entities = 1'000'000, iterations = 100, samples = 9; };
Options parse(int argc, char** argv) {
  Options o;
  for (int i = 1; i < argc; ++i) {
    std::string_view a(argv[i]);
    auto value = [&]() -> std::size_t { if (++i >= argc) { std::cerr << "Missing value for " << a << "\n"; std::exit(2); } return std::stoull(argv[i]); };
    if (a == "--entities") o.entities = value(); else if (a == "--iterations") o.iterations = value();
    else if (a == "--samples") o.samples = value(); else if (a == "--help") { std::cout << "Usage: movement_bench [--entities N] [--iterations N] [--samples N]\n"; std::exit(0); }
    else { std::cerr << "Unknown option: " << a << "\n"; std::exit(2); }
  }
  return o;
}
template<class MakeWorld, class Move, class Sum>
void benchmark(std::string_view name, MakeWorld make_world, Move move, Sum sum, const Options& o) {
  std::vector<double> results; results.reserve(o.samples);
  for (std::size_t sample = 0; sample < o.samples; ++sample) {
    auto world = make_world(); // setup and allocation are outside the timed region
    const auto begin = clock_type::now();
    for (std::size_t i = 0; i < o.iterations; ++i) move(world, movement::dt);
    const auto end = clock_type::now();
    observable_sink += sum(world);
    results.push_back(std::chrono::duration<double, std::nano>(end - begin).count() / (o.entities * o.iterations));
  }
  std::sort(results.begin(), results.end());
  std::cout << std::left << std::setw(18) << name << std::right << std::fixed << std::setprecision(2)
            << " median " << results[results.size()/2] << " ns/entity-update"
            << "  range " << results.front() << "–" << results.back() << "\n";
}
int main(int argc, char** argv) {
  const Options o = parse(argc, argv);
  std::cout << "Entities: " << o.entities << ", iterations/sample: " << o.iterations << ", samples: " << o.samples << "\n";
  benchmark("virtual provider", [&] { return movement::make_virtual_world(o.entities); }, movement::move_virtual,
    [](const auto& w) { float s=0; for (const auto& e:w.entities) s += e.position.x; return s; }, o);
  benchmark("plain function", [&] { return movement::make_plain_world(o.entities); }, movement::move_plain_world,
    [](const auto& w) { float s=0; for (const auto& e:w) s += e.position.x; return s; }, o);
  benchmark("struct data (SoA)", [&] { return movement::make_data_world(o.entities); }, movement::move_data,
    [](const auto& w) { float s=0; for (float x:w.x) s += x; return s; }, o);
  benchmark("tagged switch", [&] { return movement::make_tagged_world(o.entities); }, movement::move_tagged,
    [](const auto& w) { float s=0; for (const auto& e:w) s += e.position.x; return s; }, o);
  std::cout << "Observable checksum: " << observable_sink << "\n";
}
