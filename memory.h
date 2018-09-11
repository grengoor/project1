#ifndef MEMORY_H
#define MEMORY_H


#include <iostream>
#include <fstream>

#include <cstring>

#include <sys/types.h>
#include <unistd.h>


typedef int mem_t;

class Memory {
public:
    static const int MEMORY_LEN = 2000;
private:
    mem_t memory[MEMORY_LEN];
    /* Read commands from pipe_r, write to pipe_w */
    FILE *pipe_r = nullptr, *pipe_w = nullptr;

    pid_t _pid = -1;

    bool _is_good = true;

public:
    static const int USER_BEGIN = 0;
    static const int USER_END = 999;
    static const int SYSTEM_BEGIN = 1000;
    static const int SYSTEM_END = MEMORY_LEN - 1;

    Memory(std::string file_name, int pipe_r_fd, int pipe_w_fd) {
        pipe_r = fdopen(pipe_r_fd, "r");
        pipe_w = fdopen(pipe_w_fd, "w");
        if (!(pipe_r && pipe_w)) {
            std::cerr << "Memory::Memory: Failed to open files\n";
            _is_good = false;
            return;
        }

        FILE *file = fopen(file_name.c_str(), "r");
        int i = 0;

        if (!file) {
            std::cerr << "Failed to open " << file_name << "\n";
            _is_good = false;
            return;
        }

        while (!feof(file) && i != SYSTEM_BEGIN) {
            const int SIZE = 100;
            char str[SIZE];

            fgets(str, SIZE, file);
            memory[i++] = atoi(str);
        }

        fclose(file);
    }

    mem_t read(int address) const {
        return memory[address];
    }

    void write(int address, mem_t data) {
        memory[address] = data;
    }

    bool do_command() {
        const int SIZE = 30;
        char str[SIZE];

        fgets(str, SIZE, pipe_r);
        std::cout << str;

        if (str[0] == 'r') { /* Read */
            int address;

            sscanf(str, "r %d\n", &address);
            fprintf(pipe_w, "%d\n", memory[address]);
            fflush(pipe_w);
        } else if (str[0] == 'w') { /* Write */
            int address, value;

            sscanf(str, "w %d %d\n", &address, &value);
            memory[address] = value;
        } else if (!strcmp(str, "end\n")) {
            return false;
        } else {
            std::cerr << "Memory::do_command: Invalid input\n";
            return false;
        }
        return true;
    }

    /* Start a new process and accept commands by reading from cpu_pipe and
     * writing to mem_pipe
     * If do_fork, then execute in a forked process
     * Return 0 upon success, -1 upon failure */
    int start(bool do_fork) {
        if (do_fork) {
            pid_t pid = fork();
            if (pid == 0) {
                while (do_command());
                exit(0);
            } else if (pid > 0) {
                _pid = pid;
            } else {
                std::cerr << "Memory::start: Failed to create fork\n";
                return -1;
            }
        } else {
            _pid = -1;
            while (do_command());
        }
        return 0;
    }

    pid_t pid() const {
        return _pid;
    }

    bool is_good() const {
        return _is_good;
    }

    operator bool() const {
        return is_good();
    }
};


#endif /* MEMORY_H */
