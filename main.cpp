#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <functional>

class MemoryBlock {
public:
    int start, end;
    std::string process;

    MemoryBlock(int s, int e, const std::string& p = "") : start(s), end(e), process(p) {}

    int size() const {
        return end - start + 1;
    }

    bool isFree() const {
        return process.empty();
    }
};

class MemoryAllocator {
private:
    int maxMemory;
    std::vector<MemoryBlock> blocks;

    void sortBlocks() {
        std::sort(blocks.begin(), blocks.end(), [](const MemoryBlock& a, const MemoryBlock& b) {
            return a.start < b.start;
        });
    }

    void insertBlock(int start, int size, const std::string& process) {
        blocks.emplace_back(start, start + size - 1, process);
        sortBlocks();
    }

    bool allocateByCriteria(const std::string& process, int size,
                            std::function<bool(int, int)> comp) {
        int index = -1;
        int chosenSize = comp(0, 1) ? maxMemory + 1 : -1;

        for (size_t i = 0; i < blocks.size(); ++i) {
            if (blocks[i].isFree() && blocks[i].size() >= size) {
                if (index == -1 || comp(blocks[i].size(), chosenSize)) {
                    index = static_cast<int>(i);
                    chosenSize = blocks[i].size();
                }
            }
        }

        if (index == -1) return false;

        int start = blocks[index].start;
        int end = start + size - 1;

        if (blocks[index].size() > size) {
            blocks[index].start = end + 1;
        } else {
            blocks.erase(blocks.begin() + index);
        }

        insertBlock(start, size, process);
        return true;
    }

public:
    explicit MemoryAllocator(int size) : maxMemory(size) {
        blocks.emplace_back(0, size - 1);
    }

    bool allocateFirstFit(const std::string& process, int size) {
        for (size_t i = 0; i < blocks.size(); ++i) {
            if (blocks[i].isFree() && blocks[i].size() >= size) {
                int start = blocks[i].start;
                int end = start + size - 1;

                if (blocks[i].size() > size) {
                    blocks[i].start = end + 1;
                } else {
                    blocks.erase(blocks.begin() + i);
                }

                insertBlock(start, size, process);
                return true;
            }
        }
        return false;
    }

    bool allocateBestFit(const std::string& process, int size) {
        return allocateByCriteria(process, size, std::less<int>());
    }

    bool allocateWorstFit(const std::string& process, int size) {
        return allocateByCriteria(process, size, std::greater<int>());
    }

    bool release(const std::string& process) {
        bool found = false;
        for (auto& block : blocks) {
            if (block.process == process) {
                block.process = "";
                found = true;
            }
        }
        if (found) mergeAdjacentFreeBlocks();
        return found;
    }

    void mergeAdjacentFreeBlocks() {
        sortBlocks();
        for (size_t i = 0; i < blocks.size() - 1;) {
            if (blocks[i].isFree() && blocks[i + 1].isFree()) {
                blocks[i].end = blocks[i + 1].end;
                blocks.erase(blocks.begin() + i + 1);
            } else {
                ++i;
            }
        }
    }

    void compact() {
        sortBlocks();
        std::vector<MemoryBlock> newBlocks;
        int nextFreeAddress = 0;

        for (const auto& block : blocks) {
            if (!block.isFree()) {
                int sz = block.size();
                newBlocks.emplace_back(nextFreeAddress, nextFreeAddress + sz - 1, block.process);
                nextFreeAddress += sz;
            }
        }

        if (nextFreeAddress < maxMemory)
            newBlocks.emplace_back(nextFreeAddress, maxMemory - 1, "");

        blocks = std::move(newBlocks);
    }

    void printStatus() const {
        std::cout << "\nMemory Status:\n";
        for (const auto& block : blocks) {
            std::string status = block.isFree() ? "Unused" : "Process " + block.process;
            std::cout << "Addresses [" << block.start << ":" << block.end << "] "
                      << status << " | Size: " << block.size() << "\n";
        }
        std::cout << std::endl;
    }

    bool processRequest(const std::string& process, int size, char strategy) {
        strategy = std::toupper(strategy);
        switch (strategy) {
            case 'F': return allocateFirstFit(process, size);
            case 'B': return allocateBestFit(process, size);
            case 'W': return allocateWorstFit(process, size);
            default:
                std::cerr << "Error: Unknown strategy '" << strategy << "'\n";
                return false;
        }
    }
};

// Main driver function
int main() {

     int memorySize;
    std::cout << "Enter total memory size: ";
    std::cin >> memorySize;
    std::cin.ignore();

    MemoryAllocator allocator(memorySize);
    std::string line;

    while (true) {
        std::cout << "allocator> ";
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "X") break;
        else if (cmd == "RQ") {
            std::string process;
            int size;
            char strategy;
            iss >> process >> size >> strategy;
            if (allocator.processRequest(process, size, strategy)) {
                std::cout << "Allocated " << size << " bytes to " << process << "\n";
            } else {
                std::cout << "Failed to allocate " << size << " bytes to " << process << "\n";
            }
        } else if (cmd == "RL") {
            std::string process;
            iss >> process;
            if (allocator.release(process)) {
                std::cout << "Released memory for " << process << "\n";
            } else {
                std::cout << "Process '" << process << "' not found.\n";
            }
        } else if (cmd == "C") {
            allocator.compact();
            std::cout << "Memory compacted.\n";
        } else if (cmd == "STAT") {
            allocator.printStatus();
        } else {
            std::cout << "Unknown command. Available: RQ, RL, C, STAT, X\n";
        }
    }

}
