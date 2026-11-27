// src/policy/policy_chain.cpp — YOU IMPLEMENT. Spec §5.3.

#include "policy/policy_chain.h"

#include <utility>

bool PolicyChain::add(std::unique_ptr<Policy> child) {
    if (!child) {
        return false;
    }
    children_.push_back(std::move(child));
    return true;
}

std::size_t PolicyChain::size() const noexcept { return children_.size(); }

Decision PolicyChain::evaluate(const CommandRequest& req) {
    if (children_.empty()) {
        return Decision::deny("policy chain is empty", "chain");
    }

    bool has_ask = false;
    Decision first_ask = Decision::allow("", "chain");

    for (const auto& child : children_) {
        Decision d = child->evaluate(req);
        if (d.kind == Decision::Kind::Deny) {
            return d;
        }
        if (d.kind == Decision::Kind::Ask && !has_ask) {
            first_ask = std::move(d);
            has_ask = true;
        }
    }

    if (has_ask) {
        return first_ask;
    }

    return Decision::allow("allowed by policy chain", "chain");
}