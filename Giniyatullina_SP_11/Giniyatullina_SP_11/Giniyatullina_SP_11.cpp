// Giniyatullina_SP_11.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>

const int MAX_ROBOTS = 10;
const int MAX_TESTS = 100;

struct Robot {
    unsigned int id;
    unsigned int energy;//запас энергии
    unsigned int score;//очки
    bool is_finished;//состояние "завершил испытанние"
    HANDLE hThread;//идентификатор робота
};

Robot robots[MAX_ROBOTS];
int test_program[MAX_TESTS];//испытаний коды из файла
int test_count = 0;//сколько их прочитано
int num_robots = 0;//сколько роботов выбрал пользователь

DWORD WINAPI robot_work(LPVOID lpParam)
{
    int idx = (int)lpParam;//индекс робота в массиве
    Robot& r = robots[idx];
    srand((unsigned int)time(NULL) + idx * 13);

    std::cout << "\n[Робот " << r.id << "] Приступил к испытаниям.\n";

    for (int i = 0; i < test_count; i++) {
        int test_code = test_program[i];
        const char* test_name = "";
        int duration = 0;//длительность
        int energy_cost = 0;
        int points_reward = 0;

        //вероятность 20% поломки
        if (rand() % 10 < 2) {
            std::cout << "\n[Робот " << r.id << "] Сломался! Начинается ремонт.\n";
            Sleep(2000);
            r.energy -= 10;
            if (r.energy < 0) {
                r.energy = 0;
            }
            std::cout << "\n[Робот " << r.id << "] Ремонт завершён. Энергия: " << r.energy << "\n";

            Sleep((1 + rand() % 3) * 1000);
            continue;
        }

        if (test_code == 1) {
            test_name = "Препятствие";
            duration = 3;
            energy_cost = 10;
            points_reward = 15;
        }
        else if (test_code == 2) {
            test_name = "Спринт";
            duration = 4;
            energy_cost = 20;
            points_reward = 30;
        }
        else if (test_code == 4) {
            test_name = "Марафон";
            duration = 10;
            energy_cost = 60;
            points_reward = 55;
        }
        else if (test_code == 3) {
            test_name = "Подзарядка";
            duration = 1 + (rand() % 5);
        }

        if (test_code != 3 && r.energy < energy_cost) {
            std::cout << "\n[Робот " << r.id << "] Пропуск испытания \"" << test_name << "\" — недостаточно энергии.\n";
            Sleep((1 + rand() % 3) * 1000);
            continue;
        }


        std::cout << "\n[Робот " << r.id << "] Старт: " << test_name << " (длительность " << duration << " сек)\n";

        for (int j = 0; j < duration; j++) {
            Sleep(1000);
        }

        if (test_code == 3) {
            r.energy += duration * 10;
            if (r.energy > 100) {
                r.energy = 100;
            }
            std::cout << "\n[Робот " << r.id << "] Подзарядка завершена. Энергия: " << r.energy << "\n";
        }
        else {
            r.energy -= energy_cost;
            r.score += points_reward;
            std::cout << "\n[Робот " << r.id << "] Испытание пройдено. Энергия: " << r.energy << ", очки: " << r.score << "\n";
        }

        Sleep((1 + rand() % 3) * 1000);
    }

    r.is_finished = true;
    std::cout << "\n[Робот " << r.id << "] Финишировал. Итоговые очки: " << r.score << "\n";
    return 0;
}



bool load_tests() {
    std::ifstream file("arena.dat", std::ios::binary);
    if (!file.is_open()) {
        std::cout << "\nОшибка: файл arena.dat не найден!\n";
        return false;
    }

    int val;
    test_count = 0;
    while (file.read((char*)&val, sizeof(int)) && test_count < MAX_TESTS) {
        test_program[test_count] = val;
        test_count++;
    }
    std::cout << "\nЗагружено испытаний: " << test_count << "\n";
    return true;
}

int main()
{
    setlocale(NULL, "Russian");
    srand(time(0));

    std::cout << "Укажите количество роботов (от 3 до 10): ";
    std::cin >> num_robots;
    if (num_robots < 3 || num_robots > 10) {
        std::cout << "\nНедопустимое количество роботов.\n";
        return 1;
    }

    if (!load_tests()) {
        return 1;
    }

    for (int i = 0; i < num_robots; i++) {
        robots[i].id = i + 1;
        robots[i].energy = 60;
        robots[i].score = 0;
        robots[i].is_finished = false;
        robots[i].hThread = CreateThread(NULL, 0, robot_work, (LPVOID)i, 0, NULL);
        //атрибуты безопасности по умолч/размер стека по умолч/какую функцию выполняет поток/параметр, передаваемый в функцию (номер робота)/флаги/id потока
    }

    std::cout << "\n       Арена запущена       \n";
    std::cout << "\nКлавиши 1.." << num_robots << " — пауза/возобновление робота.\n";

    static bool paused[MAX_ROBOTS] = { false };

    while (true) {
        bool all_finished = true;
        for (int i = 0; i < num_robots; i++)
        {
            if (!robots[i].is_finished)
            {
                all_finished = false;
            }
        }
        if (all_finished)
        {
            break;
        }

        if (_kbhit()) {
            char key = _getch();
            if (key >= '1' && key <= '9')
            {
                int idx = key - '1';
                if (idx >= 0 && idx < num_robots && !robots[idx].is_finished)
                {
                    if (!paused[idx])
                    {
                        //если робот не на паузе, то ставим на паузу
                        SuspendThread(robots[idx].hThread);//приостановить поток с указанным handle
                        paused[idx] = true;
                        std::cout << "\n[Робот " << robots[idx].id << "] Приостановлен.\n";
                    }
                    else
                    {
                        ResumeThread(robots[idx].hThread);//возобновить поток
                        paused[idx] = false;
                        std::cout << "\n[Робот " << robots[idx].id << "] Возобновил работу.\n";
                    }
                }
            }
        }
        Sleep(50);
    }



    std::cout << "\nВсе роботы завершили испытания!\n";

    for (int i = 0; i < num_robots; i++) {
        WaitForSingleObject(robots[i].hThread, INFINITE);//не пойдёт дальше, пока поток действительно не завершится
        CloseHandle(robots[i].hThread);//освободить ресурсы потока
    }

    for (int i = 0; i < num_robots - 1; i++) {
        for (int k = 0; k < num_robots - 1 - i; k++) {
            if (robots[k].score < robots[k + 1].score) {
                Robot temp = robots[k];
                robots[k].id = robots[k + 1].id;
                robots[k].energy = robots[k + 1].energy;
                robots[k].score = robots[k + 1].score;

                robots[k + 1].id = temp.id;
                robots[k + 1].energy = temp.energy;
                robots[k + 1].score = temp.score;
            }
        }
    }

    std::cout << "\n       Итоговая таблица       \n";
    for (int i = 0; i < num_robots; i++)
    {
        std::cout << "Робот " << robots[i].id << " — " << robots[i].score << " очков\n";
    }

    if (num_robots > 1 && robots[0].score == robots[1].score)
    {
        std::cout << "\nНичья!\n";
    }
    else {
        std::cout << "\nПобедитель: Робот " << robots[0].id << "!\n";
    }

    _getch();

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
