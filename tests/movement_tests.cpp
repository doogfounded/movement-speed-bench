#include "movement_models.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

bool near(float a, float b) { return std::fabs(a - b) < 0.00001f; }
int main() {
  constexpr std::size_t n = 9; constexpr float elapsed = 0.5f;
  auto virtual_world = movement::make_virtual_world(n); movement::move_virtual(virtual_world, elapsed);
  auto plain_world = movement::make_plain_world(n); movement::move_plain_world(plain_world, elapsed);
  auto data_world = movement::make_data_world(n); movement::move_data(data_world, elapsed);
  auto tagged_world = movement::make_tagged_world(n); movement::move_tagged(tagged_world, elapsed);
  const float expected[] = {1, 3, 6, 1, 3, 6, 1, 3, 6};
  for (std::size_t i = 0; i < n; ++i) {
    if (!near(virtual_world.entities[i].position.x, expected[i]) || !near(plain_world[i].position.x, expected[i]) ||
        !near(data_world.x[i], expected[i]) || !near(tagged_world[i].position.x, expected[i])) {
      std::cerr << "Movement mismatch at " << i << "\n"; return EXIT_FAILURE;
    }
  }
  std::cout << "All movement models agree.\n";
}
