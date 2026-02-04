// IWYU pragma: private, include <neutron/execution_resources.hpp>
#pragma once
#include <cstdint>
#include <thread>
#include <version>
#include "execution.hpp"

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>

#elif defined(__linux) || defined(__linux__)
#include <pthread.h>
#if ATOM_USES_NUMA && __has_include(<numa.h>)
#include <numa.h>
#endif

#elif false

#else
#endif

namespace neutron {

#if __cplusplus > 202302L && __has_include(<execution>)
namespace execution = ::std::execution;
#elif __has_include(<stdexec/stdexec.hpp>)
namespace execution = ::stdexec;
#endif

namespace _affinity_thread {

void _set_affinity(uint64_t core, uint64_t numa) {
#if defined(_WIN32) // ignore numa policy
  HANDLE handle = GetCurrentThread();
  DWORD_PTR mask = 1ULL << core;
  SetThreadAffinityMask(handle, mask);
#elif defined(__linux) || defined(__linux__) // complete numa support
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);

  pthread_t this_pthread = pthread_self();
  pthread_setaffinity_np(this_pthread, sizeof(cpu_set_t), &cpuset);

#if ATOM_USES_NUMA && __has_include(<numa.h>)
  numa_run_on_node(static_cast<int>(numa));
#endif

#elif false

#else
#endif
}

} // namespace _affinity_thread

void set_affinity(uint64_t core, uint32_t numa = 0) {
  _affinity_thread::_set_affinity(core, numa);
}

class affinity_thread {
  class _affinity {
  public:
    [[nodiscard]] uint64_t get_core() const noexcept { return core_; }

#if defined(__linux) || defined(__linux__)
    constexpr _affinity(uint64_t core, uint64_t numa) noexcept
        : core_(core), numa_(numa) {}

    [[nodiscard]] uint64_t get_numa() const noexcept { return numa_; }
#else
    constexpr _affinity(uint64_t core, uint64_t numa) noexcept : core_(core) {}

    ATOM_NODISCARD uint64_t get_numa() const noexcept { return 0; }
#endif

  private:
    uint64_t core_;
#if defined(__linux) || defined(__linux__)
    uint64_t numa_;
#endif
  };

public:
  explicit affinity_thread(uint64_t core, uint64_t numa = 0)
      : affinity_{core, numa}, loop_(), thread_([this] {
          set_affinity(affinity_.get_core(), affinity_.get_numa());
          loop_.run();
        }) {}

  affinity_thread(const affinity_thread &) = delete;
  affinity_thread &operator=(const affinity_thread &) = delete;
  affinity_thread(affinity_thread &&) = delete;
  affinity_thread &operator=(affinity_thread &&) = delete;

  ~affinity_thread() {
    loop_.finish();
    thread_.join();
  }

  auto get_scheduler() noexcept { return loop_.get_scheduler(); }

  [[nodiscard]] uint32_t core_id() const noexcept {
    return affinity_.get_core();
  }

  [[nodiscard]] uint32_t numa_id() const noexcept {
    return affinity_.get_numa();
  }

  [[nodiscard]]
  auto get_id() const noexcept -> std::thread::id {
    return thread_.get_id();
  }

private:
  _affinity affinity_;
  execution::run_loop loop_;
  std::thread thread_;
};

} // namespace neutron

