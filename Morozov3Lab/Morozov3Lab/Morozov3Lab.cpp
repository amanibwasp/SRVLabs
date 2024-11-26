#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <string>

int main()
{
    clock_t start, end;
    start = clock();
    std::cout << "Press anykey to start\n";
    char x = _getch();
    std::cout << "start - " << x << std::endl;
    bool exit_program = false;
    char exit_symbol = 'k';
    std::string message = std::string("Pressed: ") + x + "\n";

    while (!exit_program)  // буфер не пустой?
    {
        if (_kbhit()) {
            char new_input_char = _getch();
            if (new_input_char == exit_symbol) {
                exit_program = true;
            }
            else {
                message = std::string("Pressed: ") + new_input_char + "\n";
            }
        }
        end = clock();
        if ((double)(end - start) / CLOCKS_PER_SEC >= 1) //прошла  ли секунда?
        {
            std::cout << message;
            start = clock();
        }
    }
}
