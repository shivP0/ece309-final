// src/skills/dep_graph.cpp — YOU IMPLEMENT. Spec §6.2.

#include "skills/dep_graph.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

bool DepGraph::add_node(const std::string& name) {
    return requires_.emplace(name, std::vector<std::string>{}).second;
}

bool DepGraph::add_edge(const std::string& from, const std::string& to) {
    auto it = requires_.find(from);
    if (it == requires_.end()) {
        return false;
    }

    auto& deps = it->second;
    if (std::find(deps.begin(), deps.end(), to) != deps.end()) {
        return false;
    }

    deps.push_back(to);
    required_by_[to].push_back(from);
    return true;
}

bool DepGraph::has_node(const std::string& name) const {
    return requires_.find(name) != requires_.end();
}

std::size_t DepGraph::node_count() const noexcept {
    return requires_.size();
}

const std::vector<std::string>& DepGraph::requirements(const std::string& name) const {
    auto it = requires_.find(name);
    if (it != requires_.end()) {
        return it->second;
    }
    static const std::vector<std::string> kNone;
    return kNone;
}

bool DepGraph::dfs(const std::string& node,
                   std::unordered_map<std::string, Color>& colors,
                   std::vector<std::string>& path,
                   std::vector<std::string>& out,
                   std::string* error) const {
    colors[node] = Color::Gray;
    path.push_back(node);

    const auto& deps = requires_.at(node);
    for (const std::string& dep : deps) {
        if (!has_node(dep)) {
            if (error != nullptr) {
                *error = "missing: " + dep + " (required by " + node + ")";
            }
            return false;
        }

        Color c = colors[dep];
        if (c == Color::Gray) {
            if (error != nullptr) {
                auto cycle_start = std::find(path.begin(), path.end(), dep);
                std::string msg = "cycle: ";
                for (auto it = cycle_start; it != path.end(); ++it) {
                    msg += *it + " -> ";
                }
                msg += dep;
                *error = std::move(msg);
            }
            return false;
        }

        if (c == Color::White) {
            if (!dfs(dep, colors, path, out, error)) {
                return false;
            }
        }
    }

    path.pop_back();
    colors[node] = Color::Black;
    out.push_back(node);
    return true;
}

bool DepGraph::order_from(const std::string& root, std::vector<std::string>* out,
                          std::string* error) const {
    if (!has_node(root)) {
        if (error != nullptr) {
            *error = "unknown: " + root;
        }
        return false;
    }

    std::unordered_map<std::string, Color> colors;
    colors.reserve(requires_.size());
    for (const auto& entry : requires_) {
        colors[entry.first] = Color::White;
    }

    std::vector<std::string> path;
    std::vector<std::string> result;
    if (!dfs(root, colors, path, result, error)) {
        return false;
    }

    if (out != nullptr) {
        *out = std::move(result);
    }
    return true;
}

std::vector<std::string> DepGraph::dependents(const std::string& name) const {
    std::vector<std::string> result;
    std::unordered_set<std::string> seen;
    seen.insert(name);

    std::vector<std::string> current_level{name};
    while (!current_level.empty()) {
        std::vector<std::string> next_level;
        for (const std::string& curr : current_level) {
            auto it = required_by_.find(curr);
            if (it == required_by_.end()) {
                continue;
            }
            for (const std::string& parent : it->second) {
                if (seen.insert(parent).second) {
                    next_level.push_back(parent);
                }
            }
        }

        std::sort(next_level.begin(), next_level.end());
        result.insert(result.end(), next_level.begin(), next_level.end());
        current_level = std::move(next_level);
    }

    return result;
}