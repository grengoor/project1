#include <functional>
#include <map>

#include "cpu.h"

using namespace std;


map<int, int> CPU::opcode_num_of_args = {
    {LOAD_VALUE, 1},
    {LOAD_ADDR, 1},
    {LOAD_IND_ADDR, 1},
    {LOAD_IDX_X_ADDR, 1},
    {LOAD_IDX_Y_ADDR, 1},
    {LOAD_SP_X, 0},
    {STORE_ADDR, 1},
    {GET, 0},
    {PUT_PORT, 0},
    {ADD_X, 0},
    {ADD_Y, 0},
    {SUB_X, 0},
    {SUB_Y, 0},
    {COPY_TO_X, 0},
    {COPY_FROM_X, 0},
    {COPY_TO_Y, 0},
    {COPY_FROM_Y, 0},
    {COPY_TO_SP, 0},
    {COPY_FROM_SP, 0},
    {JUMP_ADDR, 1},
    {JUMP_IF_EQUAL_ADDR, 1},
    {JUMP_IF_NOT_EQUAL_ADDR, 1},
    {CALL_ADDR, 1},
    {RET, 0},
    {INC_X, 0},
    {DEC_X, 0},
    {PUSH, 0},
    {POP, 0},
    {INT, 1},
    {IRET, 0},
    {END, 0}
};
