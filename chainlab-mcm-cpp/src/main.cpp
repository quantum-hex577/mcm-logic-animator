#include "mcm.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {
using namespace chainlab;

struct DisplayOptions {
    bool color{true};
    bool clearBetweenFrames{true};
};

std::string paint(const std::string& text, const std::string& code, bool color) {
    if (!color) return text;
    return "\033[" + code + "m" + text + "\033[0m";
}

void clearConsole(const DisplayOptions& options) {
    if (options.clearBetweenFrames) std::cout << "\033[2J\033[H";
}

std::string cellText(Cost value, bool active, bool referenced, bool color) {
    std::string text = formatCost(value);
    if (active) return paint("[" + text + "]", "1;96", color);
    if (referenced) return paint("{" + text + "}", "1;93", color);
    if (value >= INF) return paint(text, "90", color);
    return text;
}

void printCostTable(const Frame& frame, bool color) {
    const int n = static_cast<int>(frame.costTable.size());
    std::cout << "\nMinimum-cost table m[i][j]  (* active cell, {referenced cell})\n";
    std::cout << std::setw(8) << " ";
    for (int j = 0; j < n; ++j) std::cout << std::setw(12) << ("A" + std::to_string(j + 1));
    std::cout << '\n';
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(8) << ("A" + std::to_string(i + 1));
        for (int j = 0; j < n; ++j) {
            const bool active = i == frame.i && j == frame.j;
            const bool isCandidateFrame = frame.type == FrameType::Candidate || frame.type == FrameType::Update;
            const bool referenced = isCandidateFrame && frame.k >= 0 &&
                ((i == frame.i && j == frame.k) ||
                 (i == frame.k + 1 && j == frame.j));
            const std::string cell = cellText(frame.costTable[i][j], active, referenced, color);
            std::cout << std::setw(12) << cell;
        }
        std::cout << '\n';
    }
}

void printSplitTable(const Frame& frame, bool color) {
    const int n = static_cast<int>(frame.splitTable.size());
    std::cout << "\nSelected split table s[i][j]  (number is the matrix after which to split)\n";
    std::cout << std::setw(8) << " ";
    for (int j = 0; j < n; ++j) std::cout << std::setw(12) << ("A" + std::to_string(j + 1));
    std::cout << '\n';
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(8) << ("A" + std::to_string(i + 1));
        for (int j = 0; j < n; ++j) {
            std::string value = frame.splitTable[i][j] < 0
                ? "--" : std::to_string(frame.splitTable[i][j] + 1);
            if (i == frame.i && j == frame.j) value = paint("[" + value + "]", "1;96", color);
            std::cout << std::setw(12) << value;
        }
        std::cout << '\n';
    }
}

void printPseudocode(int activeLine, bool color) {
    std::cout << "\nAlgorithm trace\n";
    for (int line = 0; line <= 11; ++line) {
        std::ostringstream number;
        number << std::setw(2) << (line + 1) << ". ";
        const std::string content = number.str() + pseudocodeLine(line);
        if (line == activeLine) {
            std::cout << paint(" >> " + content, "1;92", color) << '\n';
        } else {
            std::cout << "    " << content << '\n';
        }
    }
}

void printTree(const Frame& frame, bool color) {
    if (frame.type != FrameType::Traceback || frame.treeNodes.empty()) return;
    std::cout << "\nTraceback tree (revealed nodes: " << frame.visibleTreeNodeCount
              << "/" << frame.treeNodes.size() << ")\n";
    const std::size_t count = std::min(frame.visibleTreeNodeCount, frame.treeNodes.size());
    for (std::size_t i = 0; i < count; ++i) {
        const TreeNode& node = frame.treeNodes[i];
        std::string line(static_cast<std::size_t>(node.depth) * 3, ' ');
        line += (node.id == frame.activeTreeNodeId ? "> " : "  ");
        line += node.label;
        if (node.i != node.j) line += "  [split after A" + std::to_string(node.split + 1) + "]";
        if (node.id == frame.activeTreeNodeId) line = paint(line, "1;95", color);
        std::cout << line << '\n';
    }
}

void printFrame(const std::vector<int>& dimensions,
                const std::vector<Frame>& frames,
                std::size_t frameIndex,
                const DisplayOptions& options) {
    const Frame& frame = frames[frameIndex];
    clearConsole(options);
    std::cout << paint("CHAINLAB | C++17 MATRIX CHAIN MULTIPLICATION LOGIC ANIMATOR", "1;96", options.color) << "\n";
    std::cout << "Frame " << (frameIndex + 1) << "/" << frames.size()
              << " | " << frameTypeName(frame.type)
              << " | comparisons: " << frame.comparisons
              << " | minimum updates: " << frame.updates << "\n\n";

    std::cout << "Dimensions: [";
    for (std::size_t i = 0; i < dimensions.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << dimensions[i];
    }
    std::cout << "]  =>  " << (dimensions.size() - 1) << " matrices\n";
    std::cout << frame.message << "\n";

    if ((frame.type == FrameType::Candidate || frame.type == FrameType::Update) &&
        frame.k >= 0 && frame.candidate < INF) {
        std::cout << "Candidate details: left=" << formatCost(frame.leftCost)
                  << ", right=" << formatCost(frame.rightCost)
                  << ", multiplication=" << formatCost(frame.multiplyCost)
                  << ", q=" << formatCost(frame.candidate)
                  << ", previous best=" << formatCost(frame.bestBefore) << "\n";
    }

    printCostTable(frame, options.color);
    printSplitTable(frame, options.color);
    printPseudocode(frame.pseudocodeLine, options.color);
    printTree(frame, options.color);
    if (frameIndex + 1 == frames.size()) {
        const Result result = solve(dimensions);
        std::cout << "\n" << paint("FINAL RESULT", "1;92", options.color) << "\n";
        std::cout << "Minimum scalar multiplications : " << result.optimalCost << '\n';
        std::cout << "Left-to-right multiplication   : " << result.leftToRightCost << '\n';
        std::cout << "Operation-count reduction      : " << std::fixed << std::setprecision(2)
                  << result.savingsPercent << "%\n";
        std::cout << "Optimal parenthesization       : " << result.parenthesization << '\n';
        std::cout << "Complexity                     : O(n^3) time, O(n^2) DP-table space\n";
    }
    std::cout << "\n";
}

bool parsePositiveInt(const std::string& input, int& value) {
    std::istringstream stream(input);
    long long parsed;
    char extra;
    if (!(stream >> parsed) || (stream >> extra)) return false;
    if (parsed < 1 || parsed > 10000) return false;
    value = static_cast<int>(parsed);
    return true;
}

std::vector<int> parseDimensions(const std::string& line) {
    std::vector<int> dimensions;
    std::string normalized = line;
    for (char& ch : normalized) if (ch == ',' || ch == ';') ch = ' ';
    std::istringstream stream(normalized);
    std::string token;
    while (stream >> token) {
        int value = 0;
        if (!parsePositiveInt(token, value)) {
            throw std::invalid_argument("Use positive whole numbers separated by spaces or commas.");
        }
        dimensions.push_back(value);
    }
    validateDimensions(dimensions);
    return dimensions;
}

void printHelp() {
    std::cout << "Commands: n=next, b=back, a=autoplay, g=go to frame, r=reset, "
                 "f=final result, s=speed, h=help, q=quit\n";
    std::cout << "Autoplay can be interrupted with Ctrl+C. Pressing Enter between manual commands pauses the trace.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    DisplayOptions display;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--no-color") display.color = false;
        else if (arg == "--no-clear") display.clearBetweenFrames = false;
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: chainlab [--no-color] [--no-clear]\n";
            return 0;
        }
    }

    std::cout << "ChainLab - C++17 Matrix Chain Multiplication Logic Animator\n";
    std::cout << "Enter matrix dimensions (3 to 9 positive integers, max 10000 each).\n";
    std::cout << "Example: 30 35 15 5 10\nPress Enter to use that example: ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) input = "30 35 15 5 10";

    std::vector<int> dimensions;
    std::vector<Frame> frames;
    try {
        dimensions = parseDimensions(input);
        frames = buildFrames(dimensions);
    } catch (const std::exception& error) {
        std::cerr << "Input error: " << error.what() << '\n';
        return 1;
    }

    std::size_t current = 0;
    int delayMs = 180;
    bool running = true;
    while (running) {
        printFrame(dimensions, frames, current, display);
        printHelp();
        std::cout << "Current frame " << (current + 1) << ". Command: ";
        std::string command;
        if (!std::getline(std::cin, command)) break;
        if (command.empty()) continue;
        const char action = command[0];

        switch (action) {
            case 'n': case 'N':
                if (current + 1 < frames.size()) ++current;
                else std::cout << "Already at the final frame.\n";
                break;
            case 'b': case 'B':
                if (current > 0) --current;
                else std::cout << "Already at the first frame.\n";
                break;
            case 'r': case 'R':
                current = 0;
                break;
            case 'f': case 'F':
                current = frames.size() - 1;
                break;
            case 'g': case 'G': {
                std::cout << "Frame number (1-" << frames.size() << "): ";
                std::string frameInput;
                std::getline(std::cin, frameInput);
                try {
                    const unsigned long parsed = std::stoul(frameInput);
                    if (parsed >= 1 && parsed <= frames.size()) current = parsed - 1;
                    else std::cout << "Frame number is out of range.\n";
                } catch (...) {
                    std::cout << "Please enter a valid whole number.\n";
                }
                break;
            }
            case 's': case 'S': {
                std::cout << "Autoplay delay in milliseconds (50-2000): ";
                std::string speedInput;
                std::getline(std::cin, speedInput);
                try {
                    int parsed = std::stoi(speedInput);
                    if (parsed < 50 || parsed > 2000) std::cout << "Use a value between 50 and 2000.\n";
                    else delayMs = parsed;
                } catch (...) { std::cout << "Invalid delay.\n"; }
                break;
            }
            case 'a': case 'A': case 'p': case 'P':
                while (current + 1 < frames.size()) {
                    ++current;
                    printFrame(dimensions, frames, current, display);
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
                }
                break;
            case 'h': case 'H':
                printHelp();
                std::cout << "Legend: cyan brackets mark the active cell; yellow braces mark candidate inputs.\n";
                break;
            case 'q': case 'Q':
                running = false;
                break;
            default:
                std::cout << "Unknown command. Enter h for help.\n";
        }
    }

    std::cout << "ChainLab session ended.\n";
    return 0;
}
