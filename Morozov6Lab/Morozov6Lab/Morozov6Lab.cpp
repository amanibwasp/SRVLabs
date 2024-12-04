#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
using namespace std;

const int MAX_NUGGETS = 10000;
const int NUM_FATMEN = 3;
vector<int> dishes(NUM_FATMEN, 3000);
vector<int> consumed(NUM_FATMEN, 0);
mutex m;
atomic<bool> cook_turn(true);
atomic<bool> cook_resigned(false);
atomic<bool> cook_fired(false);
atomic<bool> cook_without_salary(false);
atomic<int> gluttons_exloaded(0);

class Cook {
    int efficiency_factor;
public:
    Cook(int efficiency) : efficiency_factor(efficiency) {}
    void work() {
        int days = 0;
        while (!cook_fired && !cook_resigned && !cook_without_salary) {
            if (!cook_turn) {
                std::this_thread::yield();
                continue;
            }
            {
                lock_guard<mutex> lock(m);

                for (int i = 0; i < NUM_FATMEN; ++i) {
                    dishes[i] += efficiency_factor;
                }
                cout << "Кук добавил по " << efficiency_factor << " наггетсов на каждую тарелку." << endl;

                cook_turn = false;
            }
            std::this_thread::sleep_for(chrono::seconds(1));
            days++;

            if (days >= 5) {
                cook_resigned = true;
                cout << "Кук устал и уволился сам!" << endl;
            }
        }
        if (cook_fired) {
            cout << "Кук уволен. Не справился (" << endl;
        }
        if (cook_without_salary) {
            cout << "Кук перестарался, никто ему уже не заплатит. Все лопнули" << endl;
        }
    }
};

class Glutton {
    int index;
    int gluttony;
    bool is_exploaded = false;
public:
    Glutton(int index, int gluttony) : index(index), gluttony(gluttony) {}
    void eat() {
        while (!cook_fired && !cook_resigned && !is_exploaded) {
            if (cook_turn) {
                std::this_thread::yield();
                continue;
            }
            {
                lock_guard<mutex> lock(m);

                if (dishes[index] >= gluttony) {
                    dishes[index] -= gluttony;
                    consumed[index] += gluttony;

                    cout << "Толстяк " << index << " съел " << gluttony << " наггетсов. Всего съел: "
                        << consumed[index] << endl;

                    if (consumed[index] > MAX_NUGGETS) {
                        cout << "Толстяк " << index << " лопнул!" << endl;
                        gluttons_exloaded++;
                        is_exploaded = true;
                        if (gluttons_exloaded == NUM_FATMEN) {
                            cook_without_salary = true;
                        }

                    }
                }
                else {
                    cout << "Толстяк " << index << " остался голодным!" << endl;
                    cook_fired = true;
                }

                cook_turn = true;
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    int efficiency_factor = 5000;
    vector<int> gluttony = { 1000, 1000, 1000 };

    Cook cook(efficiency_factor);
    Glutton first_glutton(0, gluttony[0]);
    Glutton second_glutton(1, gluttony[1]);
    Glutton third_glutton(2, gluttony[2]);

    thread cook_thread(&Cook::work, &cook);
    thread glutton_first_thread(&Glutton::eat, &first_glutton);
    thread glutton_second_thread(&Glutton::eat, &second_glutton);
    thread glutton_third_thread(&Glutton::eat, &third_glutton);

    cook_thread.join();
    glutton_first_thread.join();
    glutton_second_thread.join();
    glutton_third_thread.join();

}
