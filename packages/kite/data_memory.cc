#include <cstring>
#include <fstream>
#include <iostream>
#include "defs.h"
#include "data_cache.h"
#include "data_memory.h"

using namespace std;

data_memory_t::data_memory_t(uint64_t *m_ticks, uint64_t m_memory_size, uint64_t m_latency) :
    cache(0),
    ticks(m_ticks),
    memory(0),
    accessed(0),
    memory_size(m_memory_size),
    num_dwords(m_memory_size>>3),
    latency(m_latency),
    resp_ticks(0),
    req_block(0) {
    // Check if the memory size is a multiple of doubleword.
    if(memory_size & 0b111) {
        cerr << "Error: memory size must be a multiple of doubleword" << endl;
        exit(1);
    }
    if(memory_size < min_memory_size) {
        cerr << "Error: memory size has to be at least 2KB" << endl;
        exit(1);
    }

    // Allocate a memory space in unit of doublewords.
    memory = new int64_t[num_dwords];
    accessed = new bool[num_dwords];

    // Zero the memory space.
    memset(memory,   0, num_dwords * sizeof(int64_t));
    memset(accessed, 0, num_dwords * sizeof(bool));

    // Load initial memory state.
    load_memory_state();
}

data_memory_t::~data_memory_t() {
    // Deallocate the memory space.
    delete [] memory;
    delete [] accessed;
}

// Connect to the upper-level cache.
void data_memory_t::connect(data_cache_t *m_cache) { cache = m_cache; }

// Run the data memory.
void data_memory_t::run() {
    if(req_block && (*ticks >= resp_ticks)) {
        // Invoke the upper-level cache to handle a returned response.
        cache->handle_response(req_block);
        // Clear the requested block.
        req_block = 0;
    }
}

// Load a memory block.
void data_memory_t::load_block(uint64_t m_addr, uint64_t m_block_size) {
    // Check the doubleword alignment of memory address.
    if(m_addr & 0b111) {
        cerr << "Error: invalid alignment of memory address " << m_addr << endl;
        exit(1);
    }
    // Check if the requested block size is within memory space.
    if((m_addr+m_block_size) > memory_size) {
        cerr << "Error: memory address " << m_addr << " is out of bounds" << endl;
        exit(1);
    }
    else if(m_addr < code_segment_size) {
        cerr << "Error: memory address " << m_addr << " is in the code segment" << endl;
        exit(1);
    }

    // Mark all doublewords in the requested block are accessed.
    for(uint64_t i = 0; i < m_block_size>>3; i++) { accessed[(m_addr>>3)+i] = true; }
    // Set pointer to a requested block.
    req_block = &memory[m_addr>>3];
    // Set time ticks to respond to the cache later.
    resp_ticks = *ticks + latency;
}

// Load initial memory state.
void data_memory_t::load_memory_state() {
    // Open a memory state file.
    fstream file_stream;
    file_stream.open("memory_state", fstream::in);
    if(!file_stream.is_open()) {
        cerr << "Error: failed to open memory_state" << endl;
        exit(1);
    }

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
        // Store memory state.
        size_t l = line.find_first_of("=");
        string addr_str = line.substr(0, l);
        // Trim the line.
        line.erase(0, l+1);
        string data_str = line;

        // Check if the memory address and data are valid.
        if(!is_num_str(addr_str) || !is_num_str(data_str) ||
           !addr_str.length()    || !data_str.length()) {
            cerr << "Error: invalid memory address and/or data " << addr_str
                 << " = " << data_str << " at line #" << line_num
                 << " of memory_state" << endl;
            exit(1);
        }

        // Convert the memory address and data string to numbers.
        uint64_t memory_addr = get_imm(addr_str); 
        int64_t memory_data = get_imm(data_str);
        // Check the alignment of memory address.
        if(memory_addr & 0b111) {
            cerr << "Error: invalid alignment of memory address " << memory_addr
                 << " at line #" << line_num << " of memory_state" << endl;
            exit(1);
        }
        // The memory address goes out of bounds.
        if((memory_addr+8) > memory_size) {
            cerr << "Error: memory address " << memory_addr << " is out of bounds"
                 << " at line #" << line_num << " of memory_state" << endl;
            exit(1);
        }
        else if(memory_addr < code_segment_size) {
            cerr << "Error: memory address below " << code_segment_size
                 << " is reserved for the code segment" << endl;
            exit(1);
        }
        // Check if multiple different values are defined at the same memory address.
        int64_t &dword = memory[memory_addr>>3];
        if(dword && (dword != memory_data)) {
            cerr << "Error: memory address " << memory_addr
                 << " has multiple values defined at line # " << line_num
                 << " of memory_state" << endl;
            exit(1);
        }
        // Store the memory data.
        dword = memory_data;
    }

    // Close the memory state file.
    file_stream.close();
}

// Print memory state.
void data_memory_t::print_state() const {
    cout << endl << "Memory state (only accessed addresses):" << endl;
    for(uint64_t i = 0; i < num_dwords; i++) {
        if(accessed[i]) { cout << "(" << (i<<3) << ") = " << memory[i] << endl; }
    }
}

