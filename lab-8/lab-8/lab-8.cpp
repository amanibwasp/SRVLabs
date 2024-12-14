#include <thread>
#include <iostream>
#include <vector>
#include <atomic>
#include <windows.h> 

#include "Node.h" 
#include "lockfree_stackj.h" 

double eps = 0.00001;

bool compareApproximately(double a, double b)
{
    return std::abs(a - b) < eps;
}

double start = 0;
double end = 4;
double step = 0.1;

std::atomic<bool> stop_all = false;

namespace {
    size_t readers_num = 4;
    lf::LockFreeVersionedStack<Data> stack(readers_num);
    auto y = [](double x)
        {
            return 4 * x - std::pow(x, 2);
        };
    void writer() {
        try {
            double x = start;
            for (std::size_t i = 0; i < 10000; ++i)
            {
                for (std::int64_t i = 0; i < 1000; ++i)
                {
                    stack.push(Data{ x, y(x) });
                    x += step;
                }
                for (std::int64_t i = 0; i < 1000; ++i)
                {
                    stack.pop();
                    x -= step;
                }
                for (std::int64_t i = 0; i < 500; ++i)
                {
                    stack.push(Data{ x, y(x) });
                    x += step;
                }
                for (std::int64_t i = 0; i < 500; ++i)
                {
                    stack.pop();
                    x -= step;
                }
            }
            stack.stop();
            std::cout << "Число версий: " << stack.last_version() << std::endl;
        }
        catch (...) {
            std::cout << "Unknown ex! writer" << std::endl;
            stop_all.store(true);
        }
    }

    class Reader
    {
    public:
        Reader(unsigned int id, lf::LockFreeVersionedStack<Data>* stack)
            : id_(id), stack_(stack)
        {
        }

        void life()
        {
            try {
                while (!stack_->is_stopped() && !stop_all)
                {
                    auto data = read();
                    check(data);
                }
            }
            catch (std::exception e) {
                std::cout << e.what() << std::endl;
            }
            catch (...) {
                std::cout << "Unknown ex!" << std::endl;
                stop_all.store(true);
            }
        }

        std::vector<Data> read()
        {
            lf::LockFreeVersionedStack<Data>::NodePtr data_ptr;
            if (!stack_->subscribe(id_, data_ptr))
            {
                return {};
            }
            std::vector<Data> result;
            while (data_ptr != nullptr)
            {
                result.push_back(data_ptr->data);
                data_ptr = data_ptr->next;
            }
            versions_cnt++;
            return result;
        }

        void check(std::vector<Data> const& data)
        {
            double x_temp = start;
            double step = ::step;
            if (data.size() == 0)
                std::cout << data.size() << std::endl;
            for (auto const& position : data) {

                if (!compareApproximately(position.y, y(position.x)))
                {
                    std::cout << "Неконсистентность. Точка вне параболы";
                    throw std::exception();
                }

                if (!compareApproximately(std::abs(position.x - x_temp), step))
                {
                    //std::cout << "Неконсистентность. Разница между значениями функции больше шага";
                    throw std::exception();
                    //std::cout << "error pos.x = " << position.x << " x = " << x_temp << std::endl;
                }
                x_temp = position.x;
            }
        }
        std::uint64_t versions_cnt = 0;
    private:
        std::int64_t id_;
        lf::LockFreeVersionedStack<Data>* stack_;
        std::thread thread_;
    };
}

int main() {
    std::setlocale(LC_ALL, "Russian");
    std::vector<Reader> readers;
    std::vector<std::thread> threads;
    for (int i = 0; i < readers_num; ++i)
    {
        readers.emplace_back(i, &stack);
    }
    for (int i = 0; i < readers_num; ++i)
    {
        threads.emplace_back(&Reader::life, &readers[i]);
    }
    writer();

    for (size_t i = 0; i < readers_num; ++i)
    {
        threads[i].join();
    }
    for (size_t i = 0; i < readers_num; ++i)
    {
        std::cout << "Читающий поток " << i << ": " << readers[i].versions_cnt << std::endl;
    }
}


