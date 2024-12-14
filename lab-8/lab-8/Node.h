#pragma once

namespace lf {
    template <typename T>
    struct Node {
        using NodePtr = Node<T>*;
        T data;
        NodePtr next;
        uint64_t version;
    };
}

