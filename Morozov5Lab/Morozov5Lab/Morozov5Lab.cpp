#include <thread>
#include <iostream>
#include <mutex>


int coins = 101;
int Bob_coins = 0;
int Tom_coins = 0;
int Dead_man_coins = 0;

std::mutex m;

void coin_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        m.lock();
        if (coins > 0) {
            if (
                coins > 1 && thief_coins <= companion_coins ||
                coins == 1 && thief_coins < companion_coins
                ) {
                coins--;
                thief_coins++;
                std::cout << name << " взял монету. "
                    << "Монеты у " << name << ": " << thief_coins
                    << ", у напарника: " << companion_coins
                    << ". Осталось монет: " << coins << std::endl;
            }
            else if (coins == 1 && thief_coins == companion_coins) {
                coins--;
                Dead_man_coins++;
            }
        }
        else {
            m.unlock();
            break;
        }
        m.unlock();
    }
}

int main()
{
    std::setlocale(LC_ALL, "Russian");
    std::thread Bob_turn(coin_sharing, "Bob", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread Tom_turn(coin_sharing, "Tom", std::ref(Tom_coins), std::ref(Bob_coins));

    Bob_turn.join();
    Tom_turn.join();

    std::cout << "Итог: Боб - " << Bob_coins << " монет, Том - " << Tom_coins
        << " монет. " << "Покойник - " << Dead_man_coins
        << " Монет осталось: " << coins << std::endl;

    system("pause");
}


