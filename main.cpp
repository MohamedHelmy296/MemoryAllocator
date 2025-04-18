#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

class MemoryBlock {
public:
    int start;
    int end;
    std::string process; // Empty string means the block is free

    MemoryBlock(int s, int e, const std::string& p = "") : start(s), end(e), process(p) {}

    int size() const {
        return end - start + 1;
    }
};

class MemoryAllocator {
private:
    int maxMemory;
    std::vector<MemoryBlock> blocks;

public:
    MemoryAllocator(int size) : maxMemory(size) {
        // Initialize with one free block spanning the entire memory
        blocks.push_back(MemoryBlock(0, size - 1, ""));
    }

    // First fit allocation strategy
    bool allocateFirstFit(const std::string& process, int size) {
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].process.empty() && blocks[i].size() >= size) {
                // Found a suitable block
                int start = blocks[i].start;
                int end = start + size - 1;

                // Update the free block
                if (blocks[i].size() > size) {
                    blocks[i].start = end + 1;
                } else {
                    // Exact fit, remove the block
                    blocks.erase(blocks.begin() + i);
                }

                // Insert the allocated block
                blocks.push_back(MemoryBlock(start, end, process));

                // Sort blocks by start address
                std::sort(blocks.begin(), blocks.end(),
                    [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

                return true;
            }
        }
        return false; // No suitable block found
    }

    // Best fit allocation strategy
    bool allocateBestFit(const std::string& process, int size) {
        int bestFitIndex = -1;
        int bestFitSize = maxMemory + 1; // Initialize to a value larger than possible

        // Find the smallest suitable free block
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].process.empty() && blocks[i].size() >= size) {
                if (blocks[i].size() < bestFitSize) {
                    bestFitSize = blocks[i].size();
                    bestFitIndex = i;
                }
            }
        }

        if (bestFitIndex != -1) {
            // Found a suitable block
            int start = blocks[bestFitIndex].start;
            int end = start + size - 1;

            // Update the free block
            if (blocks[bestFitIndex].size() > size) {
                blocks[bestFitIndex].start = end + 1;
            } else {
                // Exact fit, remove the block
                blocks.erase(blocks.begin() + bestFitIndex);
            }

            // Insert the allocated block
            blocks.push_back(MemoryBlock(start, end, process));

            // Sort blocks by start address
            std::sort(blocks.begin(), blocks.end(),
                [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

            return true;
        }
        return false; // No suitable block found
    }

    // Worst fit allocation strategy
    bool allocateWorstFit(const std::string& process, int size) {
        int worstFitIndex = -1;
        int worstFitSize = -1; // Initialize to a value smaller than possible

        // Find the largest suitable free block
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].process.empty() && blocks[i].size() >= size) {
                if (blocks[i].size() > worstFitSize) {
                    worstFitSize = blocks[i].size();
                    worstFitIndex = i;
                }
            }
        }

        if (worstFitIndex != -1) {
            // Found a suitable block
            int start = blocks[worstFitIndex].start;
            int end = start + size - 1;

            // Update the free block
            if (blocks[worstFitIndex].size() > size) {
                blocks[worstFitIndex].start = end + 1;
            } else {
                // Exact fit, remove the block
                blocks.erase(blocks.begin() + worstFitIndex);
            }

            // Insert the allocated block
            blocks.push_back(MemoryBlock(start, end, process));

            // Sort blocks by start address
            std::sort(blocks.begin(), blocks.end(),
                [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

            return true;
        }
        return false; // No suitable block found
    }

    // Release memory allocated to a process
    bool release(const std::string& process) {
        bool found = false;

        // Find all blocks allocated to the process
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].process == process) {
                // Mark this block as free
                blocks[i].process = "";
                found = true;

                // Merge with any adjacent free blocks
                mergeAdjacentFreeBlocks();
            }
        }

        return found;
    }

    // Merge adjacent free blocks
    void mergeAdjacentFreeBlocks() {
        // Sort blocks by start address
        std::sort(blocks.begin(), blocks.end(),
            [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

        for (size_t i = 0; i < blocks.size() - 1; i++) {
            if (blocks[i].process.empty() && blocks[i + 1].process.empty()) {
                // Merge the blocks
                blocks[i].end = blocks[i + 1].end;
                blocks.erase(blocks.begin() + i + 1);

                // Check again from the same position
                i--;
            }
        }
    }

    // Compact memory (move all allocated blocks to lower addresses)
    void compact() {
        // Sort blocks by start address
        std::sort(blocks.begin(), blocks.end(),
            [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

        int nextFreeAddress = 0;
        std::vector<MemoryBlock> newBlocks;

        // Relocate all allocated blocks
        for (const auto& block : blocks) {
            if (!block.process.empty()) {
                // This is an allocated block, move it
                int size = block.size();
                newBlocks.push_back(MemoryBlock(nextFreeAddress, nextFreeAddress + size - 1, block.process));
                nextFreeAddress += size;
            }
        }

        // Add one free block at the end if there's space left
        if (nextFreeAddress < maxMemory) {
            newBlocks.push_back(MemoryBlock(nextFreeAddress, maxMemory - 1, ""));
        }

        blocks = newBlocks;
    }

    // Print the current status of memory allocation
    void printStatus() const {
        for (const auto& block : blocks) {
            std::string status = block.process.empty() ? "Unused" : "Process " + block.process;
            std::cout << "Addresses [" << block.start << ":" << block.end << "] " << status << std::endl;
        }
    }

    // Process a request command
    bool processRequest(const std::string& process, int size, char strategy) {
        switch (strategy) {
            case 'F': return allocateFirstFit(process, size);
            case 'B': return allocateBestFit(process, size);
            case 'W': return allocateWorstFit(process, size);
            default:
                std::cout << "Invalid allocation strategy" << std::endl;
                return false;
        }
    }
};
// Main function to handle user input and commands
int main() {
    int memorySize;
    std::cout << "Enter total memory size: ";
    std::cin >> memorySize;
    std::cin.ignore();

    MemoryAllocator allocator(memorySize);
    std::string command;

    while (true) {
        std::cout << "allocator> ";
        std::getline(std::cin, command);

    MemoryAllocator allocator(memorySize);
    std::string command;

    while (true) {
        std::cout << "allocator> ";
        std::getline(std::cin, command);

        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "X") {
            break;
        } else if (cmd == "RQ") {
            std::string process;
            int size;
            char strategy;

            iss >> process >> size >> strategy;

            if (allocator.processRequest(process, size, strategy)) {
                std::cout << "Successfully allocated " << size << " bytes to " << process << std::endl;
            } else {
                std::cout << "Error: Cannot allocate " << size << " bytes to " << process << std::endl;
            }
        } else if (cmd == "RL") {
            std::string process;
            iss >> process;

            if (allocator.release(process)) {
                std::cout << "Successfully released memory for " << process << std::endl;
            } else {
                std::cout << "Error: Process " << process << " not found" << std::endl;
            }
        } else if (cmd == "C") {
            allocator.compact();
            std::cout << "Memory compacted" << std::endl;
        } else if (cmd == "STAT") {
            allocator.printStatus();
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }

    return 0;
}}
