#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class MOSPhase1 {
private:
    static const int kMemoryWords = 100;
    static const int kWordSize = 4;
    static const int kBlockWords = 10;
    static const int kCardSize = kWordSize * kBlockWords;

    typedef std::array<char, kWordSize> Word;
    typedef std::array<Word, kMemoryWords> Memory;

    Memory M;
    Word R;
    Word IR;
    int IC;
    int SI;
    bool C;
    std::ifstream input;
    std::ofstream output;
    std::vector<std::string> programCards;
    std::vector<std::string> dataCards;
    std::size_t dataCardIndex;
    std::string jobId;

    static std::string trimLineEnding(std::string line) {
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
            line.pop_back();
        }
        return line;
    }

    static bool startsWith(const std::string& line, const std::string& prefix) {
        return line.compare(0, prefix.size(), prefix) == 0;
    }

    void clearBuffer(Word& word) {
        word.fill(' ');
    }

    void initialize() {
        for (std::size_t i = 0; i < M.size(); ++i) {
            M[i].fill(' ');
        }
        R.fill(' ');
        IR.fill(' ');
        IC = 0;
        SI = 0;
        C = false;
        dataCardIndex = 0;
    }

    int getLocation() const {
        if (!std::isdigit(static_cast<unsigned char>(IR[2])) ||
            !std::isdigit(static_cast<unsigned char>(IR[3]))) {
            return -1;
        }
        return (IR[2] - '0') * 10 + (IR[3] - '0');
    }

    bool validBlock(int location) const {
        return location >= 0 && location + kBlockWords <= kMemoryWords;
    }

    bool validWord(int location) const {
        return location >= 0 && location < kMemoryWords;
    }

    void loadProgram() {
        int address = 0;
        for (std::size_t cardIndex = 0; cardIndex < programCards.size(); ++cardIndex) {
            const std::string& card = programCards[cardIndex];
            for (std::size_t pos = 0; pos < card.size() && address < kMemoryWords; pos += kWordSize) {
                M[address].fill(' ');
                for (int j = 0; j < kWordSize && pos + j < card.size(); ++j) {
                    M[address][j] = card[pos + j];
                }
                ++address;
            }
        }
    }

    void readData() {
        int location = getLocation();
        if (!validBlock(location)) {
            output << "ERROR: invalid address for GD\n\n";
            return;
        }
        if (dataCardIndex >= dataCards.size()) {
            output << "ERROR: out of data\n\n";
            return;
        }

        std::string card = dataCards[dataCardIndex++];
        if (card.size() > static_cast<std::size_t>(kCardSize)) {
            card.resize(kCardSize);
        }
        card.resize(kCardSize, ' ');

        std::size_t pos = 0;
        for (int word = 0; word < kBlockWords; ++word) {
            for (int ch = 0; ch < kWordSize; ++ch) {
                M[location + word][ch] = card[pos++];
            }
        }
    }

    void writeData() {
        int location = getLocation();
        if (!validBlock(location)) {
            output << "ERROR: invalid address for PD\n\n";
            return;
        }
        for (int word = 0; word < kBlockWords; ++word) {
            for (int ch = 0; ch < kWordSize; ++ch) {
                output << M[location + word][ch];
            }
        }
        output << '\n';
    }

    void terminateJob() {
        output << "\n\n";
    }

    void masterMode() {
        switch (SI) {
            case 1: readData(); break;
            case 2: writeData(); break;
            case 3: terminateJob(); break;
            default: break;
        }
        SI = 0;
    }

    void executeInstruction() {
        const std::string opcode(IR.begin(), IR.begin() + 2);
        const int location = getLocation();

        if (opcode == "GD") {
            SI = 1;
            masterMode();
        } else if (opcode == "PD") {
            SI = 2;
            masterMode();
        } else if (opcode == "H ") {
            SI = 3;
            masterMode();
        } else if (opcode == "LR") {
            if (!validWord(location)) { output << "ERROR: invalid address for LR\n\n"; IC = kMemoryWords; return; }
            R = M[location];
        } else if (opcode == "SR") {
            if (!validWord(location)) { output << "ERROR: invalid address for SR\n\n"; IC = kMemoryWords; return; }
            M[location] = R;
        } else if (opcode == "CR") {
            if (!validWord(location)) { output << "ERROR: invalid address for CR\n\n"; IC = kMemoryWords; return; }
            C = (M[location] == R);
        } else if (opcode == "BT") {
            if (C) {
                if (validWord(location)) IC = location;
                else { output << "ERROR: invalid branch address\n\n"; IC = kMemoryWords; }
            }
        } else {
            output << "ERROR: invalid opcode '" << opcode << "'\n\n";
            IC = kMemoryWords;
        }
    }

    void startExecution() {
        IC = 0;
        int steps = 0;
        while (validWord(IC) && steps < 1000) {
            IR = M[IC];
            ++IC;
            executeInstruction();
            ++steps;
            if (std::string(IR.begin(), IR.begin() + 2) == "H ") break;
        }
        if (steps >= 1000) {
            output << "ERROR: instruction execution limit reached\n\n";
        }
    }

    bool readNextJob(std::string& line) {
        while (std::getline(input, line)) {
            line = trimLineEnding(line);
            if (startsWith(line, "$AMJ")) return true;
        }
        return false;
    }

    bool readJobBody() {
        std::string line;
        bool foundDTA = false;
        while (std::getline(input, line)) {
            line = trimLineEnding(line);
            if (startsWith(line, "$DTA")) {
                foundDTA = true;
                break;
            }
            if (startsWith(line, "$END")) return false;
            if (!line.empty()) programCards.push_back(line);
        }
        if (!foundDTA) return false;

        while (std::getline(input, line)) {
            line = trimLineEnding(line);
            if (startsWith(line, "$END")) return true;
            dataCards.push_back(line);
        }
        return true; // permit a final job without an explicit $END
    }

public:
    MOSPhase1() : IC(0), SI(0), C(false), dataCardIndex(0) {
        initialize();
    }

    int run(const std::string& inputFile, const std::string& outputFile) {
        input.open(inputFile.c_str());
        if (!input) {
            std::cerr << "Error: cannot open " << inputFile << ".\n";
            return 1;
        }
        output.open(outputFile.c_str(), std::ios::out | std::ios::trunc);
        if (!output) {
            std::cerr << "Error: cannot create " << outputFile << ".\n";
            return 1;
        }

        int jobsProcessed = 0;
        std::string header;
        while (readNextJob(header)) {
            initialize();
            programCards.clear();
            dataCards.clear();
            dataCardIndex = 0;
            jobId = header.size() >= 8 ? header.substr(4, 4) : "UNKN";
            output << "Job " << jobId << " started.\n";

            if (!readJobBody()) {
                output << "ERROR: malformed job input\n\n";
                continue;
            }
            loadProgram();
            startExecution();
            ++jobsProcessed;
        }

        output.close();
        input.close();
        std::cout << "Finished. Processed " << jobsProcessed
                  << " job(s). Results are in " << outputFile << ".\n";
        return 0;
    }
};

int main(int argc, char* argv[]) {
    const std::string inputFile = argc > 1 ? argv[1] : "input1.txt";
    const std::string outputFile = argc > 2 ? argv[2] : "output1.txt";
    MOSPhase1 mos;
    return mos.run(inputFile, outputFile);
}
