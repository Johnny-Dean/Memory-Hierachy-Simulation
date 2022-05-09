// Lab8.cpp : This file contains the 'main' function. Program execution begins and ends there.

using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <bitset>

struct Block {
    int valid =   0;
    int tag   =   0;
    int data  =   0;
};

// 16 bits in word - 1 for valid

struct Instruction {
    int op;
    int rs;
    int rt;
    int offset;
};

// 8 Registers
int registers[8];
// 16 Words
// Each block has three fields valid bit, tag, and data
Block two_way_set_cache[8][2];
// 128 Words
int main_memory[128];


vector<string> read_object_code_file() {
    string file_name = "input-object-code.txt";
    ifstream input_object_code_file(file_name);
    if (!input_object_code_file) cerr << "No stream";

    vector<string> commands;
    string command;
    if (input_object_code_file.is_open()) {

        while (getline(input_object_code_file, command)) {
            commands.push_back(command);
        }
        input_object_code_file.close();
        return commands;
    }
    else cout << "Cannot read file";   
}

int compute_effective_address(int rs, int offset) {
    int byte_address = rs + offset;
    int word_address = byte_address / 4;
    return word_address;
}

void set_history_bit(Block block) {
    bitset<32> binary_register(block.tag);
    cout << block.tag;
}

void read_hit(int rt, int data) {
    registers[rt] = data;
}

Block find_victim_block(Block* set) {
    return set[0];
}

void write_hit(int rt, Block block) {
    block.data = registers[rt];
}

void write_miss(int rt, int memory_address) {
    main_memory[memory_address] = registers[rt];
}

int index_of_hit(Block* block_set) {
    return (block_set[0].valid) == 1 ? 0 : 1;
}

bool is_hit_miss(int index, int tag) {
    Block* found_set = two_way_set_cache[index];
    // check both ways for the potential hit
    if ((found_set[0].valid == 1 && found_set[0].tag == tag)) {
        return true;
    }
    else if ((found_set[1].valid == 1 && found_set[1].tag == tag)) {
        return true;  
    }
    else {
        return false;
    }
}

void load_word(Instruction instruction, Block* block_set, bool is_hit) {
    if (is_hit) {
        Block block = block_set[index_of_hit(block_set)];
        cout << "hit" << endl;
        read_hit(instruction.rt, block.data);
        set_history_bit(block);
    }
    // not in our cache and we need to search our memory for it, and find the "victim block"
    // where we want to put the data into
    else {
        cout << "miss" << endl;
        Block victim_block = find_victim_block(block_set);
        set_history_bit(victim_block);
    }
}

void store_word(Instruction instruction, Block* block_set, bool is_hit, int word_address) {
    cout << "sw: " << endl;
    if (is_hit) {
        Block block = block_set[index_of_hit(block_set)];
        cout << "hit" << endl;
        write_hit(instruction.rt , block);
        
    }
    else {
        cout << "miss" << endl;
        write_miss(instruction.rt, word_address);
    }
}

void execute_instruction(string command) {
    string opcode = "000000"; // 6 opcode
    string dest = "00000"; // 5 rt 
    string offset = "0000000000000000"; // 16 address

    // Set opcode, base, dest, and offset
    for (int i = 0; i < 6; i++) {
        opcode[i] = command[i];
    }
    for (int i = 12, j = 0; i < 17; i++, j++) {
        dest[j] = command[i];
    }
    for (int i = 17, j = 0; i < 32; i++, j++) {
        offset[j] = command[i];
    }

    int opcode_int = stoi(opcode, nullptr, 2);
    int dest_int = stoi(dest, nullptr, 2);
    int offset_int = stoi(offset, nullptr, 2);
    Instruction instruction = { opcode_int, 0, dest_int, offset_int };

    int word_address = compute_effective_address(instruction.rs, instruction.offset);
    int cache_set_number = word_address % 8;
    int tag = word_address / 8;
    bool is_hit = is_hit_miss(cache_set_number, tag);
    Block* block_set = two_way_set_cache[cache_set_number];


    if (instruction.op == 35) {
        load_word(instruction, block_set, is_hit);
    }
    else if (instruction.op == 43) {
        store_word(instruction, block_set, is_hit ,word_address);
    }
    else {
        cout << "Err" << endl;
    }
}

void execute_instructions(vector<string> instructions) {
    for (string instruction : instructions) {
        execute_instruction(instruction);
    }
}

void initialize_memory() {
    // mem[block_address] <- block_address + 5;  e.g., mem[0] <- 5,  ...  mem[127] <- 132
    for (int i = 0; i < 128; i++) {
        main_memory[i] = i + 5;
    }
}


void print_results() {
    cout << "Registers: " << endl;
    for (int i = 0; i < 8; i++) {
        bitset<32> binary_register(registers[i]);
        cout << "$S" << i << " " << binary_register << endl;
    }
    cout << endl;
    for (int i = 0; i < 2; i++) {
        cout << "Cache Block" << i << ": " << endl;
        cout << "Set #:     " << "Valid: " << "Tag: " << "Data: ";
        for (int j = 0; j < 8; j++) {
            Block block = two_way_set_cache[i][j];
            bitset<4> binary_tag(block.tag);
            bitset<32> binary_data(block.data);
            cout << j << ": " << block.valid << "  " << binary_tag << "  " << binary_data;
        }
    }
}

int main()
{   
    initialize_memory();
    vector<string> instructions = read_object_code_file();
    execute_instructions(instructions);
    print_results();
} 