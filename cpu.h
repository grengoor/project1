#ifndef CPU_H
#define CPU_H


#include <iostream>
#include <map>
#include <functional>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <sys/types.h>
#include <unistd.h>

#include "memory.h"


/* Registers */
class CPU {
private:
    // TODO: CHANGE
    typedef int reg_t;
    typedef int operand_t;

    reg_t pc = Memory::USER_BEGIN; /* Program Counter */
    reg_t sp = Memory::USER_STACK; /* Stack Pointer */
    reg_t ir; /* Instruction Register */
    reg_t ac; /* Accumulator */
    reg_t x;
    reg_t y;

    /* Timer interrupt occurs after every interrupt_value instructions */
    int interrupt_value;
    /* Current timer value */
    int current_timer = 0;

    enum {
        USER, KERNEL
    } mode = USER;

    /* Read memory from pipe_r, write commands to pipe_w */
    FILE *pipe_r = nullptr, *pipe_w = nullptr;

    pid_t _pid = -1;

    bool _is_good = true;


    mem_t read_mem(int address) const {
        const int SIZE = 30;
        char str[SIZE];
        mem_t val;

        if (mode == USER && Memory::SYSTEM_BEGIN <= address && address <= Memory::SYSTEM_END) {
            std::cerr << "Error: user program attempted to access system memory\n";
            end();
            exit(1);
        }

        fprintf(pipe_w, "r %d\n", address);
        fflush(pipe_w);
        fgets(str, SIZE, pipe_r);
        sscanf(str, "%d\n", &val);

        return val;
    }

    void write_mem(int address, int value) const {
        if (mode == USER && Memory::SYSTEM_BEGIN <= address && address <= Memory::SYSTEM_END) {
            std::cerr << "Error: user program attempted to access system memory\n";
            end();
            exit(1);
        }

        fprintf(pipe_w, "w %d %d\n", address, value);
    }

    void push_stack(operand_t x) {
        sp--;
        write_mem(sp, x);
    }

    mem_t pop_stack() {
        mem_t val = read_mem(sp);
        sp++;
        return val;
    }

    /* Set mode to KERNEL, push SP and PC to system stack, switch PC to
     * system stack, and jump to address (typically either Memory::INT_ADDRESS
     * or Memory::TIMER_ADDRESS) */
    void interrupt(operand_t address) {
        if (mode == KERNEL) {
            return;
        }
        mode = KERNEL;
        write_mem(Memory::SYSTEM_STACK, sp);
        sp = Memory::SYSTEM_STACK;
        push_stack(pc);
        pc = address;
    }

    /* Like int_f, but jump to TIMER_ADDRESS instead of INT_ADDRESS */
    void timer_interrupt() {
        interrupt(Memory::TIMER_ADDRESS);
    }


    /* Instructions */

    /* Load the value into the AC */
    void load_value(operand_t value) {
        #ifdef DEBUG
        printf("Load value %d\n", value);
        #endif
        ac = (reg_t) value;
    }

    /* Load the value at the address into the AC */
    void load_addr(operand_t address) {
        #ifdef DEBUG
        printf("Load addr %d\n", address);
        #endif
        ac = read_mem(address);
    }

    /* Load the value from the address found in the given address into the AC
     * For example, if LoadInd 500, and 500 contains 100, then load from 100 */
    void load_ind_addr(operand_t address) {
        #ifdef DEBUG
        printf("LoadInd addr %d\n", address);
        #endif
        int address2 = read_mem(address);
        return load_addr(address2);
    }

    /* Load the value at (address + X) into the AC
     * For example, if LoadIdxX 500, and X contains 10, then load from 510 */
    void load_idx_x_addr(operand_t address) {
        #ifdef DEBUG
        printf("LoadIdxX addr %d\n", address);
        #endif
        return load_addr(address + (operand_t) x);
    }

    /* Load the value at (address + Y) into the AC */
    void load_idx_y_addr(operand_t address) {
        #ifdef DEBUG
        printf("LoadIdxY addr %d\n", address);
        #endif
        return load_addr(address + (operand_t) y);
    }

    /* Load from (SP + X) into the AC
     * For example, if SP is 990, and X is 1, load from 991 */
    void load_sp_x() {
        #ifdef DEBUG
        printf("LoadSpX\n");
        #endif
        return load_addr((operand_t) (sp + x));
    }

    /* Store the value in the AC into the address */
    void store_addr(operand_t address) {
        #ifdef DEBUG
        printf("Store addr %d\n", address);
        #endif
        write_mem(address, ac);
    }

    /* Get a random int from 1 to 100 into the AC */
    void get() {
        #ifdef DEBUG
        printf("Get\n");
        #endif
        ac = (reg_t) (std::rand() % 100 + 1);
    }

    /* If port = 1, write AC as an int to the screen
     * If port = 2, write AC as a char to the screen */
    void put_port(operand_t port) const {
        #ifdef DEBUG
        printf("Put port\n");
        #endif
        if (port == 1) {
            std::cout << (int) ac;
            #ifdef DEBUG
            std::cout << '\n';
            #endif
        } else if (port == 2) {
            std::cout << (char) ac;
            #ifdef DEBUG
            std::cout << '\n';
            #endif
        }
    }

    /* Add the value in X to the AC */
    void add_x() {
        #ifdef DEBUG
        printf("AddX\n");
        #endif
        ac += x;
    }

    /* Add the value in Y to the AC */
    void add_y() {
        #ifdef DEBUG
        printf("AddY\n");
        #endif
        ac += y;
    }

    /* Subtract the value in X to the AC */
    void sub_x() {
        #ifdef DEBUG
        printf("SubX\n");
        #endif
        ac -= x;
    }

    /* Subtract the value in Y to the AC */
    void sub_y() {
        #ifdef DEBUG
        printf("SubY\n");
        #endif
        ac -= y;
    }

    /* Copy the value in the AC to X */
    void copy_to_x() {
        #ifdef DEBUG
        printf("CopyToX\n");
        #endif
        x = ac;
    }

    /* Copy the value in X to the AC */
    void copy_from_x() {
        #ifdef DEBUG
        printf("CopyFromX\n");
        #endif
        ac = x;
    }

    /* Copy the value in the AC to Y */
    void copy_to_y() {
        #ifdef DEBUG
        printf("CopyToY\n");
        #endif
        y = ac;
    }

    /* Copy the value in Y to the AC */
    void copy_from_y() {
        #ifdef DEBUG
        printf("CopyFromY\n");
        #endif
        ac = y;
    }

    /* Copy the value in AC to the SP */
    void copy_to_sp() {
        #ifdef DEBUG
        printf("CopyToSp\n");
        #endif
        sp = ac;
    }

    /* Copy the value in SP to the AC */
    void copy_from_sp() {
        #ifdef DEBUG
        printf("CopyFromSp\n");
        #endif
        ac = sp;
    }

    /* Jump to the address */
    void jump_addr(operand_t address) {
        #ifdef DEBUG
        printf("Jump addr %d\n", address);
        #endif
        pc = (reg_t) address;
    }

    /* Jump to the address only if the value in the AC is zero */
    void jump_if_equal_addr(operand_t address) {
        #ifdef DEBUG
        printf("JumpIfEqual addr %d\n", address);
        #endif
        if (ac == 0) {
            jump_addr(address);
        }
    }

    /* Jump to the address only if the value in the AC is not zero */
    void jump_if_not_equal_addr(operand_t address) {
        #ifdef DEBUG
        printf("JumpIfNotEqual addr %d\n", address);
        #endif
        if (ac != 0) {
            jump_addr(address);
        }
    }

    /* Push return address onto stack, jump to the address */
    void call_addr(operand_t address) {
        #ifdef DEBUG
        printf("Call addr %d\n", address);
        #endif
        push_stack(pc);
        pc = address;
    }

    /* Pop return address from the stack, jump to the address */
    void ret() {
        #ifdef DEBUG
        printf("Ret\n");
        #endif
        pc = pop_stack();
    }

    /* Increment the value in X */
    void inc_x() {
        #ifdef DEBUG
        printf("IncX\n");
        #endif
        ++x;
    }

    /* Decrement the value in X */
    void dec_x() {
        #ifdef DEBUG
        printf("DecX\n");
        #endif
        --x;
    }

    /* Push AC onto stack */
    void push() {
        #ifdef DEBUG
        printf("Push\n");
        #endif
        push_stack(ac);
    }

    /* Pop from stack into AC */
    void pop() {
        #ifdef DEBUG
        printf("Pop\n");
        #endif
        ac = pop_stack();
    }

    /* Perform system call */
    void int_f() {
        #ifdef DEBUG
        printf("Int\n");
        #endif
        interrupt(Memory::INT_ADDRESS);
    }

    /* Return from system call */
    void iret() {
        #ifdef DEBUG
        printf("IRet\n");
        #endif
        pc = pop_stack();
        sp = pop_stack();
        mode = USER;
    }

    /* End execution */
    void end() const {
        #ifdef DEBUG
        printf("End\n");
        #endif
        fputs("end\n", pipe_w);
        fflush(pipe_w);
    }

    enum opcode_t {
        LOAD_VALUE = 1,
        LOAD_ADDR,
        LOAD_IND_ADDR,
        LOAD_IDX_X_ADDR,
        LOAD_IDX_Y_ADDR,
        LOAD_SP_X,
        STORE_ADDR,
        GET,
        PUT_PORT,
        ADD_X,
        ADD_Y,
        SUB_X,
        SUB_Y,
        COPY_TO_X,
        COPY_FROM_X,
        COPY_TO_Y,
        COPY_FROM_Y,
        COPY_TO_SP,
        COPY_FROM_SP,
        JUMP_ADDR,
        JUMP_IF_EQUAL_ADDR,
        JUMP_IF_NOT_EQUAL_ADDR,
        CALL_ADDR,
        RET,
        INC_X,
        DEC_X,
        PUSH,
        POP,
        INT,
        IRET,
        END = 50
    };

    /* Instructions with 0 arguments */
    std::map<int, std::function<void()> > opcode_to_instruction0;
    /* Instructions with 1 argument */
    std::map<int, std::function<void(operand_t)> > opcode_to_instruction1;
    static std::map<int, int> opcode_num_of_args;

public:
    void initialize_opcode_to_instruction() {
        opcode_to_instruction0[LOAD_SP_X]              = [=]() { load_sp_x(); };
        opcode_to_instruction0[GET]                    = [=]() { get(); };
        opcode_to_instruction0[ADD_X]                  = [=]() { add_x(); };
        opcode_to_instruction0[ADD_Y]                  = [=]() { add_y(); };
        opcode_to_instruction0[SUB_X]                  = [=]() { sub_x(); };
        opcode_to_instruction0[SUB_Y]                  = [=]() { sub_y(); };
        opcode_to_instruction0[COPY_TO_X]              = [=]() { copy_to_x(); };
        opcode_to_instruction0[COPY_FROM_X]            = [=]() { copy_from_x(); };
        opcode_to_instruction0[COPY_TO_Y]              = [=]() { copy_to_y(); };
        opcode_to_instruction0[COPY_FROM_Y]            = [=]() { copy_from_y(); };
        opcode_to_instruction0[COPY_TO_SP]             = [=]() { copy_to_sp(); };
        opcode_to_instruction0[COPY_FROM_SP]           = [=]() { copy_from_sp(); };
        opcode_to_instruction0[RET]                    = [=]() { ret(); };
        opcode_to_instruction0[INC_X]                  = [=]() { inc_x(); };
        opcode_to_instruction0[DEC_X]                  = [=]() { dec_x(); };
        opcode_to_instruction0[PUSH]                   = [=]() { push(); };
        opcode_to_instruction0[POP]                    = [=]() { pop(); };
        opcode_to_instruction0[INT]                    = [=]() { int_f(); };
        opcode_to_instruction0[IRET]                   = [=]() { iret(); };
        opcode_to_instruction0[END]                    = [=]() { end(); };

        opcode_to_instruction1[LOAD_VALUE]             = [=](operand_t x) { load_value(x); };
        opcode_to_instruction1[LOAD_ADDR]              = [=](operand_t x) { load_addr(x); };
        opcode_to_instruction1[LOAD_IND_ADDR]          = [=](operand_t x) { load_ind_addr(x); };
        opcode_to_instruction1[LOAD_IDX_X_ADDR]        = [=](operand_t x) { load_idx_x_addr(x); };
        opcode_to_instruction1[LOAD_IDX_Y_ADDR]        = [=](operand_t x) { load_idx_y_addr(x); };
        opcode_to_instruction1[STORE_ADDR]             = [=](operand_t x) { store_addr(x); };
        opcode_to_instruction1[PUT_PORT]               = [=](operand_t x) { put_port(x); };
        opcode_to_instruction1[JUMP_ADDR]              = [=](operand_t x) { jump_addr(x); };
        opcode_to_instruction1[JUMP_IF_EQUAL_ADDR]     = [=](operand_t x) { jump_if_equal_addr(x); };
        opcode_to_instruction1[JUMP_IF_NOT_EQUAL_ADDR] = [=](operand_t x) { jump_if_not_equal_addr(x); };
        opcode_to_instruction1[CALL_ADDR]              = [=](operand_t x) { call_addr(x); };
    }

    CPU(int interrupt, int pipe_r_fd, int pipe_w_fd)
            : interrupt_value{interrupt} {
        pipe_r = fdopen(pipe_r_fd, "r");
        pipe_w = fdopen(pipe_w_fd, "w");
        if (!(pipe_r && pipe_w)) {
            std::cerr << "CPU::CPU: Failed to open files\n";
            _is_good = false;
            return;
        }

        initialize_opcode_to_instruction();
    }

    bool do_command() {
        int num_of_args;

        ir = read_mem(pc);
        pc++;

        num_of_args = opcode_num_of_args[ir];
        if (num_of_args == 0) {
            opcode_to_instruction0[ir]();

            if (ir == END) {
                return false;
            }
        } else if (num_of_args == 1) {
            int operand = read_mem(pc);
            pc++;
            opcode_to_instruction1[ir](operand);
        }

        current_timer++;
        if (current_timer % interrupt_value == 0) {
            timer_interrupt();
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
                std::cerr << "CPU::start: Failed to create fork\n";
                return -1;
            }
        } else {
            _pid = -1;
            while (do_command());
        }
        return 0;
    }

    bool is_good() const {
        return _is_good;
    }

    operator bool() const {
        return is_good();
    }
};


#endif /* CPU_H */
