#include "mcm.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace chainlab;

int main() {
    {
        const Result result = solve({10, 20, 30});
        assert(result.matrixCount == 2);
        assert(result.optimalCost == 6000);
        assert(result.leftToRightCost == 6000);
        assert(result.parenthesization == "(A1 * A2)");
    }
    {
        const Result result = solve({30, 35, 15, 5, 10});
        assert(result.matrixCount == 4);
        assert(result.optimalCost == 9375);
        assert(result.leftToRightCost == 19500);
        assert(result.parenthesization == "((A1 * (A2 * A3)) * A4)");
        assert(result.treeNodes.size() == 7);
    }
    {
        const auto frames = buildFrames({5, 10, 3, 12});
        bool sawDiagonal = false, sawCandidate = false, sawUpdate = false;
        bool sawFinalize = false, sawTraceback = false;
        std::size_t previousComparisons = 0;
        for (const auto& frame : frames) {
            sawDiagonal |= frame.type == FrameType::Diagonal;
            sawCandidate |= frame.type == FrameType::Candidate;
            sawUpdate |= frame.type == FrameType::Update;
            sawFinalize |= frame.type == FrameType::Finalize;
            sawTraceback |= frame.type == FrameType::Traceback;
            assert(frame.index < frames.size());
            assert(frame.comparisons >= previousComparisons);
            previousComparisons = frame.comparisons;
        }
        assert(sawDiagonal && sawCandidate && sawUpdate && sawFinalize && sawTraceback);
        assert(frames.back().visibleTreeNodeCount == 5); // 3 leaves + 2 internal nodes
    }
    {
        bool rejected = false;
        try { validateDimensions({10, 0, 20}); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    {
        bool rejected = false;
        try { validateDimensions({10001, 2, 3}); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    std::cout << "All 5 C++ regression test groups passed.\n";
    return 0;
}
