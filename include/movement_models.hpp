#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace movement {

struct Position { float x{}, y{}; };
constexpr float dt = 1.0f / 60.0f;

// 1. Behavior is supplied through an interface.  The virtual call is deliberately
// inside the hot loop, as it would be in a polymorphic entity system.
struct SpeedProvider {
  virtual ~SpeedProvider() = default;
  virtual float speed() const noexcept = 0;
};
struct Walking final : SpeedProvider { float speed() const noexcept override { return 2.0f; } };
struct Running final : SpeedProvider { float speed() const noexcept override { return 6.0f; } };
struct Flying final : SpeedProvider { float speed() const noexcept override { return 12.0f; } };
struct VirtualEntity { Position position; const SpeedProvider* provider{}; };

struct VirtualWorld {
  std::vector<std::unique_ptr<SpeedProvider>> providers;
  std::vector<VirtualEntity> entities;
};

inline VirtualWorld make_virtual_world(std::size_t count) {
  VirtualWorld world;
  world.providers.reserve(count);
  world.entities.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    switch (i % 3) {
      case 0: world.providers.push_back(std::make_unique<Walking>()); break;
      case 1: world.providers.push_back(std::make_unique<Running>()); break;
      default: world.providers.push_back(std::make_unique<Flying>()); break;
    }
    world.entities.push_back({{}, world.providers.back().get()});
  }
  return world;
}
inline void move_virtual(VirtualWorld& world, float elapsed) noexcept {
  for (auto& entity : world.entities) entity.position.x += entity.provider->speed() * elapsed;
}

// 2. A simple record plus a plain free function: often the least surprising OO-lite form.
struct PlainEntity { Position position; float speed{}; };
inline void move_plain(PlainEntity& entity, float elapsed) noexcept {
  entity.position.x += entity.speed * elapsed;
}
inline std::vector<PlainEntity> make_plain_world(std::size_t count) {
  std::vector<PlainEntity> world(count);
  constexpr float speeds[] = {2.0f, 6.0f, 12.0f};
  for (std::size_t i = 0; i < count; ++i) world[i].speed = speeds[i % 3];
  return world;
}
inline void move_plain_world(std::vector<PlainEntity>& world, float elapsed) noexcept {
  for (auto& entity : world) move_plain(entity, elapsed);
}

// 3. Data-oriented form (structure of arrays).  Position and speed streams are separate.
struct DataWorld { std::vector<float> x, y, speed; };
inline DataWorld make_data_world(std::size_t count) {
  DataWorld world;
  world.x.assign(count, 0.0f); world.y.assign(count, 0.0f); world.speed.resize(count);
  constexpr float speeds[] = {2.0f, 6.0f, 12.0f};
  for (std::size_t i = 0; i < count; ++i) world.speed[i] = speeds[i % 3];
  return world;
}
inline void move_data(DataWorld& world, float elapsed) noexcept {
  for (std::size_t i = 0; i < world.x.size(); ++i) world.x[i] += world.speed[i] * elapsed;
}

// Useful middle ground: closed set of kinds, no per-entity vtable/pointer allocation.
enum class Kind : unsigned char { walking, running, flying };
struct TaggedEntity { Position position; Kind kind{}; };
inline float speed_for(Kind kind) noexcept {
  switch (kind) { case Kind::walking: return 2.0f; case Kind::running: return 6.0f; case Kind::flying: return 12.0f; }
  return 0.0f;
}
inline std::vector<TaggedEntity> make_tagged_world(std::size_t count) {
  std::vector<TaggedEntity> world(count);
  for (std::size_t i = 0; i < count; ++i) world[i].kind = static_cast<Kind>(i % 3);
  return world;
}
inline void move_tagged(std::vector<TaggedEntity>& world, float elapsed) noexcept {
  for (auto& entity : world) entity.position.x += speed_for(entity.kind) * elapsed;
}
} // namespace movement
