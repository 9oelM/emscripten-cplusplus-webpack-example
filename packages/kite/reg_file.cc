#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "reg_file.h"

using namespace std;

reg_file_t::reg_file_t() {
    // Clear all values in the registers and dependency check table.
    memset(regs, 0, sizeof(regs));
    memset(dep, 0, sizeof(dep));

    // Load initial register file state.
    load_reg_state();
}

reg_file_t::~reg_file_t() {
}

// Write in the register file. 
void reg_file_t::write(inst_t *m_inst, unsigned m_index, int64_t m_value) {
    if(m_index >= num_kite_regs) {
        cerr << "Error: cannot write register x" << m_index << endl;
        exit(1);
    }
    regs[m_index] = m_value;
    // Clear the dependency checker if no subsequent instructions have claimed
    // the destination register of a retiring instruction.
    if(dep[m_index] == m_inst) { dep[m_index] = 0; }
}

// Data dependency check
bool reg_file_t::dep_check(inst_t *m_inst) {
    bool stall = false;
    inst_t *prod_inst = 0;
    if(m_inst->rs1_num > 0) {
        // Check the data hazard for rs1.
        if((prod_inst = dep[m_inst->rs1_num])) {
            // Rs1 value is forwarded from a producer instruction.
            if(prod_inst->rd_ready) { m_inst->rs1_val = prod_inst->rd_val; }
            // Rs1 is not ready yet.
            else { stall = true; }
        }
        // Rs1 is clear to go.
        else { m_inst->rs1_val = regs[m_inst->rs1_num]; }
    }
    if(m_inst->rs2_num > 0) {
        // Check the data hazard for rs2.
        if((prod_inst = dep[m_inst->rs2_num])) {
            // Rs2 value is forwarded from a producer instruction.
            if(prod_inst->rd_ready) { m_inst->rs2_val = prod_inst->rd_val; }
            // Rs2 is not ready yet.
            else { stall = true; }
        }
        // Rs2 is clear to go.
        else { m_inst->rs2_val = regs[m_inst->rs2_num]; }
    }
    if(!stall && (m_inst->rd_num > 0)) {
        // This instruction is the last producer of rd.
        dep[m_inst->rd_num] = m_inst;
    }
    return stall;
}

// Clear the dependency check state.
void reg_file_t::flush() {
    memset(dep, 0, sizeof(dep));
}

// Load initial register file state.
void reg_file_t::load_reg_state() {
    // Open a register state file.
    fstream file_stream;
    file_stream.open("reg_state", fstream::in);
    if(!file_stream.is_open()) {
        cerr << "Error: failed to open reg_state" << endl;
        exit(1);
    }

    // A bit vector to check if all registers are initialized.
    unsigned loaded = 0;

    string line;
    size_t line_num = 0;
    while(getline(file_stream, line)) {
        line_num++;
        // Crop everything after a comment symbol.
        if(line.find_first_of("#") != string::npos) { line.erase(line.find_first_of("#")); }
        // Erase all spaces.
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        // Skip blank lines.
        if(!line.size()) { continue; }
        // Get register name.
        size_t l = line.find_first_of("=");
        string reg_name = line.substr(0, l);
        // Trim the line.
        line.erase(0, l+1);

        // Set a register value.
        transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::tolower);
        size_t reg_num = get_regnum(reg_name);
        if(!is_reg_str(reg_name) || (reg_num >= num_kite_regs)) {
            cerr << "Error: invalid register name " << reg_name
                 << " at line #" << line_num << " of reg_state" << endl;
            exit(1);
        }
        if(line.length() <= 0) {
            cerr << "Error: invalid register value for " << reg_name
                 << " at line #" << line_num << " of reg_state" << endl;
            exit(1);
        }
        regs[reg_num] = get_imm(line);

        // Mark that the register state has been loaded.
        if((loaded >> reg_num) & 0b1) {
            cerr << "Error: redefinition of register state for " << reg_name
                 << " at line #" << line_num << " of reg_state" << endl;
            exit(1);
        }
        loaded |= (0b1 << reg_num);

        // x0 is hard-wired to zero.
        regs[0] = 0;
    }

    // Check if all register states are initialized.
    if(loaded != unsigned(-1)) {
        unsigned reg_num = 0;
        while(loaded & 0b1) { loaded = loaded >> 1; reg_num++; }
        cerr << "Error: register state of x" << reg_num << " is undefined" << endl;
        exit(1);
    }

    // Close register state file.
    file_stream.close();
}

void reg_file_t::print_state() const {
    cout << endl << "Register state:" << endl;
    for(unsigned i = 0; i < num_kite_regs; i++) {
        cout << "x" << i << " = " << regs[i] << endl;
    }
}

