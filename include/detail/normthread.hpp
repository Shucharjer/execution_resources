// IWYU pragma: private, include <execution_resources.hpp>
#pragma once
#include "execution.hpp" // IWYU pragma: keep
#include <thread>

namespace neutron {

class normthread {
public:
  normthread() : loop_(), thread_([this] { loop_.run(); }) {}

  normthread(const normthread &) = delete;
  normthread &operator=(const normthread &) = delete;
  normthread(normthread &&) = delete;
  normthread &operator=(normthread &&) = delete;

  ~normthread() {
    loop_.finish();
    thread_.join
  }

  auto get_scheduler() noexcept { return loop_.get_scheduler(); }

  [[nodiscard]]
  auto get_id() const noexcept -> std::thread::id {
    return thread_.get_id();
  }

private:
  execution::run_loop loop_;
  std::thread thread_;
};

} // namespace neutron
