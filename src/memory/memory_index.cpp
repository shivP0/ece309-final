// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"
#include <functional>
#include <utility>

MemoryIndex::~MemoryIndex() {
    clear();
}

bool MemoryIndex::insert(MemoryRecord rec) {
    if (root_ == nullptr) {
        root_ = new Node{std::move(rec), nullptr, nullptr};
        ++count_;
        return true;
    }

    Node* parent = nullptr;
    Node* curr = root_;
    while (curr != nullptr) {
        parent = curr;
        if (rec.id < curr->rec.id) {
            curr = curr->left;
        } else if (rec.id > curr->rec.id) {
            curr = curr->right;
        } else {
            return false;
        }
    }

    Node* node = new Node{std::move(rec), nullptr, nullptr};
    if (node->rec.id < parent->rec.id) {
        parent->left = node;
    } else {
        parent->right = node;
    }
    ++count_;
    return true;
}

const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    const Node* curr = root_;
    while (curr != nullptr) {
        if (id < curr->rec.id) {
            curr = curr->left;
        } else if (id > curr->rec.id) {
            curr = curr->right;
        } else {
            return &curr->rec;
        }
    }
    return nullptr;
}

bool MemoryIndex::remove(std::uint64_t id) {
    Node* parent = nullptr;
    Node* curr = root_;

    // Locate the target node and its parent
    while (curr != nullptr && curr->rec.id != id) {
        parent = curr;
        if (id < curr->rec.id) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    if (curr == nullptr) {
        return false;
    }

    // Two children: replace curr's record with its in-order successor
    // (leftmost node in the right subtree), then unlink that successor.
    if (curr->left != nullptr && curr->right != nullptr) {
        Node* succ_parent = curr;
        Node* succ = curr->right;
        while (succ->left != nullptr) {
            succ_parent = succ;
            succ = succ->left;
        }
        curr->rec = std::move(succ->rec);
        parent = succ_parent;
        curr = succ;
    }

    // Now curr has at most one child (either left or right)
    Node* child = (curr->left != nullptr) ? curr->left : curr->right;

    if (parent == nullptr) {
        root_ = child;
    } else if (parent->left == curr) {
        parent->left = child;
    } else {
        parent->right = child;
    }

    delete curr;
    --count_;
    return true;
}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const {
    std::function<std::size_t(const Node*)> helper = [&](const Node* node) -> std::size_t {
        if (node == nullptr) {
            return 0;
        }
        const std::size_t left_h = helper(node->left);
        const std::size_t right_h = helper(node->right);
        return 1 + (left_h > right_h ? left_h : right_h);
    };
    return helper(root_);
}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const {
    std::function<void(const Node*)> helper = [&](const Node* node) {
        if (node == nullptr) {
            return;
        }
        helper(node->left);
        fn(node->rec);
        helper(node->right);
    };
    helper(root_);
}

void MemoryIndex::clear() {
    std::function<void(Node*)> helper = [&](Node* node) {
        if (node == nullptr) {
            return;
        }
        helper(node->left);
        helper(node->right);
        delete node;
    };
    helper(root_);
    root_ = nullptr;
    count_ = 0;
}