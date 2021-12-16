#include <iostream>
#include <thread>
#include <chrono>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <string>

using std::string;
std::deque<int> deque;
std::mutex mu;
std::mutex mutex;
std::mutex mut;
std::mutex mute;
std::condition_variable condition;
std::condition_variable cond;
int done;
int duty;

// Класс программиста.
class Programmer {
public:
    // Индекс программиста.
    int id;
    // Кол-во проделанной программистом работы.
    int ownDuty;
    // Проверена ли его работа.
    bool checked;
    // Правильно ли сделана его работа.
    bool isOk;
    // Кол-во работы, которую нужно проделать.
    int job;

    Programmer() {
        checked = false;
        id = 0;
        job = 0;
        ownDuty = 0;
    }

    Programmer(int job, int id) {
        this->job = job;
        this->id = id;
        checked = false;
        isOk = false;
        ownDuty = 0;
    }

    // A runnable method.
    void run() {
        while (done < duty) {
            if (isOk == false && ownDuty < job) {
                work();
            }
            check();
            if (checked == false) {
                std::cout << "Programmer " << id << " is waiting for a response." << "\n\n";
                std::unique_lock<std::mutex> unique(mu);
                cond.wait(unique, [this]() { return checked; });
            }
            if ((done < duty) && ownDuty >= job) {
                std::cout << "Programmer " << id << " has done his work. He`s sleeping while waiting." << "\n\n";
                std::unique_lock<std::mutex> un(mutex);
                condition.wait(un);
            }
        }
        std::cout << "We`ve finished! Hooray!\n\n";
        condition.notify_all();
    }

    // Working method.
    void work() {
        std::lock_guard<std::mutex> lock(mut);
        std::cout << "Programmer " << id << " is working." << "\n\n";
        std::this_thread::sleep_for(std::chrono::seconds(job));
        deque.push_back(id);
        condition.notify_all();
    }

    // Checking method.
    void check();
};

Programmer *prog;

void Programmer::check() {
    {
        std::lock_guard<std::mutex> lock(mute);
        if (!deque.empty()) {
            // Checks both sides in case of situation the last added index was its own.
            if (deque.back() != id) {
                int index = deque.back();
                if (!deque.empty()){
                    deque.pop_back();
                }
                if (prog[index].ownDuty >= prog[index].job) {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::seconds(prog[index].job));
                srand(time(NULL));
                prog[index].isOk = rand() % 2;
                prog[index].checked = true;
                if (prog[index].isOk) {
                    done++;
                    prog[index].ownDuty++;
                    prog[index].isOk = false;
                    std::cout << "Programmer " << index << " have done a task." << "\n\n";
                } else {
                    std::cout << "Programmer " << index << " should rework his task." << "\n\n";
                }
                cond.notify_all();
            } else if (deque.front() != id) {
                int index = deque.front();
                deque.pop_front();
                if (prog[index].ownDuty >= prog[index].job) {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::seconds(prog[index].job));
                srand(time(NULL));
                prog[index].isOk = rand() % 2;
                prog[index].checked = true;
                if (prog[index].isOk) {
                    std::cout << "Programmer " << index << " have done a task." << "\n\n";
                    done++;
                    prog[index].ownDuty++;
                    prog[index].isOk = false;
                } else {
                    std::cout << "Programmer " << index << " should rework his task." << "\n\n";
                }
                cond.notify_all();
            }
        }
        // Wakes up another programmer if current programmer is the last awaken.
        if (deque.size() == 1 && deque.front() == id) {
            condition.notify_one();
        }
        // The job is done-time to go home.
        if (done >= duty) {
            condition.notify_all();
        }
    }
}

int main(int argc, char *argv[]) {
    done = 0;
    int prog1, prog2, prog3;
    if (argc != 4) {
        std::cout << "The number of inputs was incorrect. A default value 1 will be assigned.\n";
        prog1 = 1;
        prog2 = 1;
        prog3 = 1;
    }else {
        prog1 = atoi(argv[1]);
        prog2 = atoi(argv[2]);
        prog3 = atoi(argv[3]);
        if (prog1 < 1 || prog1 > 100) {
            std::cout << "The first input was incorrect. A default value 1 will be assigned.\n";
            prog1 = 1;
        }
        if (prog2 < 1 || prog2 > 100) {
            std::cout << "The second input was incorrect. A default value 1 will be assigned.\n";
            prog2 = 1;
        }
        if (prog3 < 1 || prog3 > 100) {
            std::cout << "The third input was incorrect. A default value 1 will be assigned.\n";
            prog3 = 1;
        }
    }
    duty = prog1 + prog2 + prog3;
    prog = new Programmer[3];
    prog[0] = Programmer(prog1, 0);
    prog[1] = Programmer(prog2, 1);
    prog[2] = Programmer(prog3, 2);
    std::thread t1([&]() { prog[0].run(); });
    std::thread t2([&]() { prog[1].run(); });
    std::thread t3([&]() { prog[2].run(); });
    t1.join();
    t2.join();
    t3.join();
}
