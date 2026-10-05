#pragma once
// Lightweight scoped performance timers for the renderer performance pass.
// Enabled by default (overhead ~50ns/scope); define PERF_DISABLE to compile out.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Perf {
public:
  struct Sample { double totalMs = 0.0; double maxMs = 0.0; std::uint64_t calls = 0; std::uint64_t events = 0; };
  static void add(const char* name, double ms, std::uint64_t events = 0) {
    std::lock_guard<std::mutex> lock(mutex());
    Sample& s = samples()[name];
    s.totalMs += ms;
    s.maxMs = std::max(s.maxMs, ms);
    ++s.calls;
    s.events += events;
  }
  static void report() {
    std::lock_guard<std::mutex> lock(mutex());
    std::vector<std::pair<std::string, Sample>> rows(samples().begin(), samples().end());
    std::sort(rows.begin(), rows.end(),
              [](const auto& a, const auto& b) { return a.second.totalMs > b.second.totalMs; });
    std::fprintf(stderr, "%-16s %10s %8s %10s %10s %12s\n", "scope", "total_ms", "calls", "avg_ms", "max_ms", "events");
    for (const auto& r : rows) {
      const Sample& s = r.second;
      std::fprintf(stderr, "%-16s %10.2f %8llu %10.3f %10.3f %12llu\n", r.first.c_str(), s.totalMs,
                   (unsigned long long)s.calls, s.calls ? s.totalMs / s.calls : 0.0, s.maxMs,
                   (unsigned long long)s.events);
    }
  }
  static void reset() {
    std::lock_guard<std::mutex> lock(mutex());
    samples().clear();
  }

private:
  static std::unordered_map<std::string, Sample>& samples() {
    static std::unordered_map<std::string, Sample> s;
    return s;
  }
  static std::mutex& mutex() {
    static std::mutex m;
    return m;
  }
};

#ifndef PERF_DISABLE
class PerfScope {
public:
  explicit PerfScope(const char* name, std::uint64_t* events = nullptr)
      : m_name(name), m_events(events), m_start(std::chrono::steady_clock::now()) {}
  ~PerfScope() {
    const auto end = std::chrono::steady_clock::now();
    Perf::add(m_name, std::chrono::duration<double, std::milli>(end - m_start).count(),
              m_events ? *m_events : 0);
  }
  PerfScope(const PerfScope&) = delete;
  PerfScope& operator=(const PerfScope&) = delete;

private:
  const char* m_name;
  std::uint64_t* m_events;
  std::chrono::steady_clock::time_point m_start;
};
#define PERF_SCOPE(name) PerfScope _perf_scope_##__LINE__(name)
#define PERF_SCOPE_E(name, eventsVar) PerfScope _perf_scope_##__LINE__(name, &(eventsVar))
#else
#define PERF_SCOPE(name) ((void)0)
#define PERF_SCOPE_E(name, eventsVar) ((void)0)
#endif
