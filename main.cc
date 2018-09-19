#include <string>
#include <iostream>

#include <cstdlib>
#include <ctime>
#include <csignal>

#include <sys/types.h>

// #define DEBUG

#include "memory.h"
#include "cpu.h"

#define TEST

using namespace std;

int main(int argc, char *argv[]) {
    int ret_val = 0;
    string program_file;
    unsigned interrupt_value;

    int cpu_pipe[2], mem_pipe[2];

    srand(time(nullptr));

    if (argc < 3) {
        cerr << "Must have at least two arguments.\n";
        return 1;
    }

    program_file = argv[1];
    interrupt_value = atoi(argv[2]);

    if (pipe(cpu_pipe) < 0) {
        cerr << "Failed to create CPU pipe\n";
        return 1;
    }
    if (pipe(mem_pipe) < 0) {
        cerr << "Failed to create memory pipe\n";
        close(cpu_pipe[0]);
        close(cpu_pipe[1]);
        return 1;
    }

    Memory mem(program_file, cpu_pipe[0], mem_pipe[1]);
    if (!mem || mem.start(true) < 0) {
        ret_val = 1;
        // goto MAIN_END;
        close(cpu_pipe[0]);
        close(cpu_pipe[1]);
        close(mem_pipe[0]);
        close(mem_pipe[1]);
        return 1;
    }

    CPU cpu(interrupt_value, mem_pipe[0], cpu_pipe[1]);
    if (!cpu || cpu.start(false) < 0) {
        kill(mem.pid(), SIGTERM);
    }

// MAIN_END:
    close(cpu_pipe[0]);
    close(cpu_pipe[1]);
    close(mem_pipe[0]);
    close(mem_pipe[1]);
    return ret_val;
}
