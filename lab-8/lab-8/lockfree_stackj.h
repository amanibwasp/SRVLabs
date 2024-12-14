#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <limits>
#include <cstdint>

#include "Node.h"

#undef max
#undef min

namespace m
{
    struct Position
    {
        double x{};
        double y{};
    };
}

using Data = m::Position;

namespace lf {
    template <typename T = m::Position>
    class LockFreeVersionedStack {
    public:
        using NodePtr = Node<T> *;
        using AtomicNodePtr = std::atomic<Node<T> *>;
        using AtomicVersion = std::atomic_uint64_t;


        LockFreeVersionedStack(std::size_t readers_num)
        : subs_num_(readers_num)
        {
            stop_flag_.store(false);
            subscribers_ = new AtomicVersion[readers_num];
            for (int i = 0; i < readers_num; ++i)
            {
                subscribers_[i].store(0);
            }
        }

        ~LockFreeVersionedStack()
        {
            while (pop());
            for (auto& node : trash_)
            {
                delete node;
            }
        }

        bool subscribe(std::uint64_t const & id, NodePtr & stack_ptr)
        {
            auto & sub = subscribers_[id];

            while ((sub.load(std::memory_order_relaxed) == stack_.version.load(std::memory_order_relaxed)))
            {
                if (stop_flag_)
                {
                    return false;
                }
            }
            auto readed_version = sub.load();
            while (!sub.compare_exchange_strong(readed_version, stack_.version))
            {

                if (stop_flag_)
                {
                    return false;
                }
            }

            stack_ptr = stack_.head.load();
            return true;
        }

        void unsubscribe(std::uint64_t const & id)
        {
            subscribers_[id].store(0);
        }

        void push(T value)
        {

            NodePtr new_node = new Node<T>;
            new_node->data = std::move(value);
            new_node->next = stack_.head.load();
            new_node->version = stack_.version.load() + 1;

            stack_.head.store(new_node);

            ++(stack_.version);

        }

        bool pop()
        {

            NodePtr old_node = stack_.head.load();
            if (old_node == nullptr)
            {
                return false;
            }

            NodePtr new_first_node = old_node->next;

            if (new_first_node != nullptr)
            {
                new_first_node->version = stack_.version.load() + 1;
            }
            stack_.head.store(new_first_node);
            ++(stack_.version);
            update_trash(old_node);
            return true;
        }

        void stop()
        {
            stop_flag_.store(true);
        }

        bool is_stopped()
        {
            return stop_flag_.load();
        }

        std::uint64_t last_version()
        {
            return stack_.version.load();
        }

    private:
        void update_trash(NodePtr old_node)
        {
            trash_.push_back(old_node);

            
            uint64_t min_version = std::numeric_limits<uint64_t>::max();
            for (std::size_t i = 0; i < subs_num_; ++i)
            {
                auto version = subscribers_[i].load();
                if (version == 0)
                {
                    continue;
                }
                min_version = std::min(min_version, version);
            }

            
            for (std::size_t i = 0; i < trash_.size(); )
            {
                if (trash_[i]->version < min_version)
                {
                    delete trash_[i];
                    trash_.erase(trash_.begin() + i);
                }
                else
                {
                    ++i;
                }
            }
        }

        struct VersionedHead
        {
            AtomicVersion version;
            AtomicNodePtr head;
        };

        VersionedHead stack_;
        AtomicVersion * subscribers_;


        std::size_t subs_num_;

        std::vector<NodePtr> trash_;
        std::atomic<bool> stop_flag_;
    };

        

}