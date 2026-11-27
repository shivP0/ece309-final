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
    // TODO: drop timestamps that left the window, then decide.
    // Clues, in order:
    //  1. now = clock_.now_seconds().
    //  2. While the queue is NOT empty and its front timestamp has left the
    //     window, pop it. A timestamp s is still INSIDE when now - s < window_,
    //     so it has left when now - s >= window_ (exactly window_ seconds old
    //     counts as gone). Calling front() on an empty queue is undefined
    //     behavior (UBSan says "misaligned address"): check empty() first.
    //  3. If the queue already holds max_ timestamps: Deny, and do NOT push.
    //  4. Otherwise push(now) and Allow. Only allowed requests are recorded.
    (void)req;
    (void)max_;
    (void)window_;
    (void)clock_;
    return Decision::deny("not implemented", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }