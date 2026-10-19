/*
 * Multiprogramming Operating System (MOS) - Phase 2
 * ------------------------------------------------
 * C++14-compatible learning implementation based on the supplied AOSPROJECT2.pdf.
 *
 * Implemented concepts:
 *   - 300 words of real memory, 4 bytes per word
 *   - 30 frames, 10 words per frame
 *   - Page table stored in a frame in real memory
 *   - Virtual-to-real address mapping
 *   - Program loading and dynamic data-page allocation
 *   - GD, PD, LR, SR, CR, BT, H instructions
 *   - SI, PI, TI interrupt handling
 *   - TTL, TLL, out-of-data and program-error termination messages
 *
 * Build:
 *   g++ -std=c++14 -Wall -Wextra -pedantic mos_phase2.cpp -o mos
 *
 * Run:
 *   ./mos
 *
 * Input format:
 *   $AMJ + 4 digit job id + 4 digit TTL + 4 digit TLL
 *   program cards (instructions; maximum 40 characters per card)
 *   $DTA
 *   data cards
 *   $END + job id
 *
 * This is a readable educational simulation, not a production OS.
 */

#include <array>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace {
constexpr int kMemoryWords = 300;
constexpr int kWordSize = 4;
constexpr int kFrameSize = 10;
constexpr int kFrameCount = 30;
constexpr int kVirtualAddressCount = 100;
constexpr int kMaxProgramInstructions = 100;

using Word = std::array<char, kWordSize>;
using Memory = std::array<Word, kMemoryWords>;

enum class InterruptType {
    None,
    Supervisor,
    Program,
    Timer
};

struct JobInput {
    std::string jobId = "0000";
    int ttl = 0;
    int tll = 0;
    std::vector<std::string> programInstructions;
    std::vector<std::string> dataCards;
};

struct JobResult {
    bool terminated = false;
    int errorCode = 0;
    std::string message;
};

std::string trimLineEnding(std::string line) {
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.pop_back();
    }
    return line;
}

bool startsWith(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

} // namespace

class MOS {
public:
    MOS() : randomEngine(std::random_device{}()) {}

    //this runs the program 
    int run(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream input(inputFile);
        if (!input) {
            std::cerr << "Error: cannot open " << inputFile << ".\n";
            return 1;
        }

        output.open(outputFile, std::ios::out | std::ios::trunc);
        if (!output) {
            std::cerr << "Error: cannot create " << outputFile << ".\n";
            return 1;
        }

        int jobsProcessed = 0;
        JobInput job;

        while (readNextJob(input, job)) {
            jobsProcessed++;
            initializeJob(job);
            loadProgram(job);
            executeUserProgram(job);
        }

        output.close();
        std::cout << "Finished. Processed " << jobsProcessed
                  << " job(s). Results are in " << outputFile << ".\n";
        return 0;
    }

private:
    Memory memory{};
    std::array<bool, kFrameCount> frameUsed{};
    std::array<int, kVirtualAddressCount> virtualToFrame{};
    std::array<bool, kVirtualAddressCount> programPage{};
    std::array<char, kWordSize> IR{};
    std::array<char, kWordSize> R{};
    int IC = 0;
    bool C = false;
    int PTR = -1;
    int SI = 0;
    int PI = 0;
    int TI = 0;
    int TTC = 0;
    int LLC = 0;
    int currentRA = -1;
    int operandVA = -1;
    int dataCardIndex = 0;
    int instructionCount = 0;
    JobInput currentJob;
    std::ofstream output;
    std::mt19937 randomEngine;

    // -------------------- Input parsing --------------------
// reads the next job and also storting the jobs in the structure 
    bool readNextJob(std::ifstream& input, JobInput& job) {
        std::string line;
        bool foundHeader = false;

        while (std::getline(input, line)) {
            line = trimLineEnding(line);
            if (startsWith(line, "$AMJ")) {
                foundHeader = true;
                break;
            }
        }
        if (!foundHeader) return false;

        job = JobInput{};
        if (line.size() < 16) {
            job.jobId = "BADH";
            std::cerr << "Warning: short $AMJ header: " << line << '\n';
        } else {
            job.jobId = line.substr(4, 4);
            job.ttl = parseFourDigitField(line, 8);
            job.tll = parseFourDigitField(line, 12);
        }

        bool readingProgram = true;
        bool readingData = false;
        std::string card;

        while (std::getline(input, line)) {
            line = trimLineEnding(line);

            if (startsWith(line, "$DTA")) {
                readingProgram = false;
                readingData = true;
                continue;
            }
            if (startsWith(line, "$END")) {
                if (readingProgram && !card.empty()) {
                    addProgramCard(job, card);
                }
                return true;
            }

            if (readingProgram) {
                // Ignore blank lines between control cards.
                if (line.empty()) continue;
                // program instructions before DTA
                addProgramCard(job, line);
            } else if (readingData) {
                // input data line
                job.dataCards.push_back(line);
            }
        }

        // Treat an unterminated final job as a job so errors can be inspected.
        return true;
    }

    int parseFourDigitField(const std::string& line, std::size_t start) {
        try {
            return std::stoi(line.substr(start, 4));
        } catch (...) {
            return 0;
        }
    }

    void addProgramCard(JobInput& job, const std::string& card) {
        // Parse the compact reference-code style (GD20PD20H) into 4-byte
        // instructions. H is a one-character instruction padded with spaces.
        std::string compact;
        for (char ch : card) {
            if (!std::isspace(static_cast<unsigned char>(ch))) compact += ch;
        }

        std::size_t pos = 0;
        while (pos < compact.size()) {
            std::string instruction(kWordSize, ' ');
            if (compact[pos] == 'H') {
                instruction[0] = 'H';
                pos++;
            } else {
                const std::size_t take = std::min<std::size_t>(kWordSize, compact.size() - pos);
                instruction.replace(0, take, compact.substr(pos, take));
                pos += take;
            }
            job.programInstructions.push_back(instruction);
            if (job.programInstructions.size() >= kMaxProgramInstructions) break;
        }
    }

    // -------------------- Initialization and memory --------------------

    void initializeJob(const JobInput& job) {
        currentJob = job;
        for (auto& word : memory) word.fill('-');
        frameUsed.fill(false);
        virtualToFrame.fill(-1);
        programPage.fill(false);
        IR.fill(' ');
        R.fill(' ');
        IC = 0;
        C = false;
        SI = PI = TI = 0;
        TTC = LLC = 0;
        currentRA = -1;
        operandVA = -1;
        dataCardIndex = 0;
        instructionCount = static_cast<int>(job.programInstructions.size());
        jobTerminated = false;

        PTR = allocateFrame();
        if (PTR < 0) {
            throw std::runtime_error("Could not allocate a frame for the page table.");
        }

        // Each page-table entry is one 4-byte word. "****" means unmapped.
        for (int page = 0; page < kVirtualAddressCount / kFrameSize; page++) {
            memory[PTR * kFrameSize + page] = {'*', '*', '*', '*'};
        }
    }

//finding a free physical memory frame, selecting one randomly, marking it as occupied, and returning its frame number
    int allocateFrame() {
        std::vector<int> freeFrames;
        for (int frame = 0; frame < kFrameCount; frame++) {
            if (!frameUsed[frame]) freeFrames.push_back(frame);
        }
        if (freeFrames.empty()) return -1;

        std::uniform_int_distribution<int> pick(0, static_cast<int>(freeFrames.size()) - 1);
        const int selected = freeFrames[pick(randomEngine)];
        frameUsed[selected] = true;
        return selected;
    }

    bool mapPageToFrame(int virtualPage, int frame) {
        if (virtualPage < 0 || virtualPage >= 10 || frame < 0 || frame >= kFrameCount) {
            return false;
        }

        virtualToFrame[virtualPage] = frame;
        const int pteAddress = PTR * kFrameSize + virtualPage;
        memory[pteAddress] = {
            static_cast<char>('0' + virtualPage / 10),
            static_cast<char>('0' + virtualPage % 10),
            static_cast<char>('0' + frame / 10),
            static_cast<char>('0' + frame % 10)
        };
        return true;
    }

    bool allocateVirtualPage(int virtualPage, bool isProgramPage) {
        if (virtualPage < 0 || virtualPage >= 10) return false;
        if (virtualToFrame[virtualPage] != -1) {
            if (isProgramPage) programPage[virtualPage] = true;
            return true;
        }

        const int frame = allocateFrame();
        if (frame < 0) return false;
        if (!mapPageToFrame(virtualPage, frame)) return false;
        if (isProgramPage) programPage[virtualPage] = true;
        return true;
    }
    //this is for loading program into frames 
    void loadProgram(const JobInput& job) {
        const int neededPages = (static_cast<int>(job.programInstructions.size()) + 9) / 10;
        for (int page = 0; page < neededPages && page < 10; page++) {
            if (!allocateVirtualPage(page, true)) {
                std::cerr << "Job " << job.jobId << ": insufficient memory while loading program.\n";
                break;
            }
        }

        for (int va = 0; va < static_cast<int>(job.programInstructions.size()) && va < 100; va++) {
            const int page = va / kFrameSize;
            const int offset = va % kFrameSize;
            if (virtualToFrame[page] == -1) continue;
            const int ra = virtualToFrame[page] * kFrameSize + offset;
            for (int b = 0; b < kWordSize; b++) {
                memory[ra][b] = job.programInstructions[va][b];
            }
        }
    }

    // -------------------- Address translation --------------------

    int addressMap(int virtualAddress) {
        if (virtualAddress < 0 || virtualAddress >= kVirtualAddressCount) {
            PI = 2; // Operand error
            return -1;
        }

        const int virtualPage = virtualAddress / kFrameSize;
        const int offset = virtualAddress % kFrameSize;

        if (virtualToFrame[virtualPage] == -1) {
            PI = 3; // Page fault; MOS decides whether it is valid.
            operandVA = virtualAddress;
            return -1;
        }

        return virtualToFrame[virtualPage] * kFrameSize + offset;
    }

    // -------------------- CPU instruction execution --------------------

    void executeUserProgram(JobInput& job) {
        currentJob = job;
        output << "Job " << currentJob.jobId << " started.\n";

        int steps = 0;
        while (steps < 1000) {
            SI = PI = TI = 0;

            currentRA = addressMap(IC);
            if (PI != 0) {
                masterMode();
                if (jobTerminated) break;
                continue;
            }

            IR = memory[currentRA];
            IC++;

            const std::string opcode{IR[0], IR[1]};
            operandVA = -1;
            currentRA = -1;

            if (opcode != "H ") {
                const int va = decodeOperand(IR);
                currentRA = addressMap(va);
                if (PI != 0) {
                    // A missing page for GD/SR may be a valid page fault.
                    masterMode(opcode);
                    if (jobTerminated) break;
                    continue; // Re-fetch/re-execute after a valid page fault.
                }
            }

            executeInstruction(opcode, currentRA, job);
            steps++;

            // Per the PDF's SIMULATION section, time advances after execution.
            if (opcode == "GD" || opcode == "SR") TTC += 2;
            else TTC += 1;

            if (currentJob.ttl > 0 && TTC >= currentJob.ttl) TI = 2;

            if (SI != 0 || PI != 0 || TI != 0) {
                masterMode(opcode);
                if (jobTerminated) break;
            }
        }

        if (!jobTerminated) terminateJob(3, "Instruction execution limit reached.");
        jobTerminated = false;
        output << "\n\n";
    }

    int decodeOperand(const std::array<char, kWordSize>& instruction) {
        if (!std::isdigit(static_cast<unsigned char>(instruction[2])) ||
            !std::isdigit(static_cast<unsigned char>(instruction[3]))) {
            PI = 2;
            return -1;
        }
        return (instruction[2] - '0') * 10 + (instruction[3] - '0');
    }

    void executeInstruction(const std::string& opcode, int realAddress, JobInput&) {
        if (opcode == "LR") {
            for (int b = 0; b < kWordSize; b++) R[b] = memory[realAddress][b];
        } else if (opcode == "SR") {
            for (int b = 0; b < kWordSize; b++) memory[realAddress][b] = R[b];
        } else if (opcode == "CR") {
            C = (memory[realAddress] == R);
        } else if (opcode == "BT") {
            if (C) IC = (IR[2] - '0') * 10 + (IR[3] - '0');
        } else if (opcode == "GD") {
            SI = 1;
        } else if (opcode == "PD") {
            SI = 2;
        } else if (opcode == "H ") {
            SI = 3;
        } else {
            PI = 1;
        }
    }

    // -------------------- Master mode and interrupt handling --------------------

    void masterMode(const std::string& opcode = "") {
        if (TI == 0) {
            if (PI == 1) terminateJob(4, "Operation code error.");
            else if (PI == 2) terminateJob(5, "Operand error.");
            else if (PI == 3) handlePageFault(opcode);
            else if (SI == 1) readData();
            else if (SI == 2) writeOutput();
            else if (SI == 3) terminateJob(0, "Normal termination.");
        } else if (TI == 2) {
            if (PI == 1) terminateJob(34, "Time limit exceeded and operation code error.");
            else if (PI == 2) terminateJob(35, "Time limit exceeded and operand error.");
            else if (PI == 3) terminateJob(3, "Time limit exceeded.");
            else if (SI == 1) terminateJob(3, "Time limit exceeded.");
            else if (SI == 2) {
                writeOutput();
                if (!jobTerminated) terminateJob(3, "Time limit exceeded.");
            } else if (SI == 3) terminateJob(0, "Normal termination.");
            else terminateJob(3, "Time limit exceeded.");
        }
    }

    void handlePageFault(const std::string& opcode) {
        // The PDF permits a valid page fault to allocate a page and resume.
        // In this educational model, only GD and SR may create a new data page.
        if (opcode != "GD" && opcode != "SR") {
            terminateJob(6, "Invalid page fault.");
            return;
        }

        const int virtualPage = operandVA / kFrameSize;
        if (!allocateVirtualPage(virtualPage, false)) {
            terminateJob(6, "Invalid page fault: no free frame.");
            return;
        }

        // IC was advanced after fetch; return to the same instruction.
        IC--;
        PI = 0;
    }

    void readData() {
        if (static_cast<std::size_t>(dataCardIndex) >= currentJob.dataCards.size()) {
            terminateJob(1, "Out of data.");
            return;
        }

        // The operand's real address points to the first word in the 10-word block.
        const int base = currentRA;
        if (base < 0 || base + kFrameSize > kMemoryWords) {
            terminateJob(5, "Operand error while reading data.");
            return;
        }

        const std::string& card = currentJob.dataCards[dataCardIndex++];
        for (int word = 0; word < kFrameSize; word++) {
            memory[base + word].fill(' ');
            for (int b = 0; b < kWordSize; b++) {
                const std::size_t sourceIndex = static_cast<std::size_t>(word * kWordSize + b);
                if (sourceIndex < card.size()) memory[base + word][b] = card[sourceIndex];
            }
        }
        SI = 0;
    }

    void writeOutput() {
        LLC++;
        if (LLC > currentJob.tll) {
            terminateJob(2, "Line limit exceeded.");
            return;
        }

        if (currentRA < 0 || currentRA + kFrameSize > kMemoryWords) {
            terminateJob(5, "Operand error while writing output.");
            return;
        }

        for (int word = 0; word < kFrameSize; word++) {
            for (int b = 0; b < kWordSize; b++) {
                const char ch = memory[currentRA + word][b];
                if (ch == '-' || ch == '\0') continue;
                output << ch;
            }
        }
        output << '\n';
        SI = 0;
    }

    void terminateJob(int errorCode, const std::string& message) {
        if (jobTerminated) return;

        static const std::array<std::string, 7> errorNames = {
            "No error",
            "Out of data",
            "Line limit exceeded",
            "Time limit exceeded",
            "Operation code error",
            "Operand error",
            "Invalid page fault"
        };

        jobTerminated = true;
        output << "Termination: " << message << '\n';
        output << "EM=" << errorCode << " (" << errorName(errorCode, errorNames)
               << "), IC=" << IC << ", IR=";
        for (char ch : IR) output << (ch == ' ' ? '_' : ch);
        output << ", TTC=" << TTC << "/" << currentJob.ttl
               << ", LLC=" << LLC << "/" << currentJob.tll << '\n';
    }

    std::string errorName(int code, const std::array<std::string, 7>& names) {
        if (code == 34) return "Time limit exceeded + operation code error";
        if (code == 35) return "Time limit exceeded + operand error";
        if (code >= 0 && code < static_cast<int>(names.size())) return names[code];
        return "Combined/unknown error";
    }

    bool jobTerminated = false;
};

int main() {
    try {
        MOS mos;
        return mos.run("input2.txt", "output2.txt");
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}
