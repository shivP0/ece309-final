// src/memory/context_builder.cpp — YOU IMPLEMENT. Spec §4.4.

#include "memory/context_builder.h"
#include "util/min_heap.h"

ContextPlan select_context(const std::vector<ContextItem>& items,
                           std::size_t budget, std::size_t keep_recent) {
    std::size_t total = 0;
    for (std::size_t i = 0; i < items.size(); ++i) {
        total += items[i].tokens;
    }

    if (total <= budget) {
        ContextPlan plan;
        for (std::size_t i = 0; i < items.size(); ++i) {
            plan.keep.push_back(i);
        }
        plan.total_tokens = total;
        plan.over_budget = false;
        return plan;
    }

    // Recent items start at recent_start; guard against unsigned underflow
    const std::size_t recent_start =
        (keep_recent >= items.size()) ? 0 : (items.size() - keep_recent);

    // Push unprotected items keyed on (drop_rank(role), index)
    MinHeap<std::pair<int, std::size_t>> heap;
    for (std::size_t i = 0; i < recent_start; ++i) {
        if (items[i].role == Role::System || items[i].pinned) {
            continue;
        }
        heap.push({drop_rank(items[i].role), i});
    }

    // Drop lowest-rank, oldest items until total fits or no droppable items remain
    std::vector<bool> keep(items.size(), true);
    while (total > budget && !heap.empty()) {
        std::size_t idx = heap.pop().second;
        keep[idx] = false;
        total -= items[idx].tokens;
    }

    ContextPlan plan;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (keep[i]) {
            plan.keep.push_back(i);
        }
    }
    plan.total_tokens = total;
    plan.over_budget = (total > budget);
    return plan;
}