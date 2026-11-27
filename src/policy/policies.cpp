// src/policy/policies.cpp — YOU IMPLEMENT. Spec §5.2.

#include "policy/policies.h"

#include <utility>

AllowlistPolicy::AllowlistPolicy(const std::vector<std::string>& programs)
    : programs_(programs.begin(), programs.end()) {}

Decision AllowlistPolicy::evaluate(const CommandRequest& req) {
    if (programs_.find(req.program) != programs_.end()) {
        return Decision::allow("'" + req.program + "' is on the allowlist", name());
    }
    return Decision::deny("'" + req.program + "' is not on the allowlist", name());
}

ArgumentPatternPolicy::ArgumentPatternPolicy(std::vector<std::string> patterns)
    : patterns_(std::move(patterns)) {}

Decision ArgumentPatternPolicy::evaluate(const CommandRequest& req) {
    const std::string line = req.command_line();
    for (const std::string& pattern : patterns_) {
        if (pattern.empty()) {
            continue;
        }
        if (line.find(pattern) != std::string::npos) {
            return Decision::deny("command matches blocked pattern '" + pattern + "'", name());
        }
    }
    return Decision::allow("command matches no blocked patterns", name());
}

ConfirmPolicy::ConfirmPolicy(const std::vector<std::string>& programs)
    : programs_(programs.begin(), programs.end()) {}

Decision ConfirmPolicy::evaluate(const CommandRequest& req) {
    if (programs_.find(req.program) != programs_.end()) {
        return Decision::ask("'" + req.program + "' requires confirmation", name());
    }
    return Decision::allow("'" + req.program + "' does not require confirmation", name());
}

RateLimitPolicy::RateLimitPolicy(std::size_t max_commands, double window_seconds, const Clock& clock)
    : max_(max_commands), window_(window_seconds), clock_(clock) {}

Decision RateLimitPolicy::evaluate(const CommandRequest& req) {
    (void)req;

    const double now = clock_.now_seconds();
    while (!times_.empty() && now - times_.front() >= window_) {
        times_.pop();
    }

    if (times_.size() >= max_) {
        return Decision::deny("rate limit exceeded", name());
    }

    times_.push(now);
    return Decision::allow("within rate limit", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }