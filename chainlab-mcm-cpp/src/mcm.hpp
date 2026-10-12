#ifndef CHAINLAB_MCM_HPP
#define CHAINLAB_MCM_HPP

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace chainlab {

using Cost = long long;
constexpr Cost INF = std::numeric_limits<Cost>::max() / 4;

struct TreeNode {
    int id{};
    int i{};              // zero-based inclusive matrix interval
    int j{};
    int split{-1};        // zero-based split after matrix k; -1 for leaf
    int depth{};
    int parentId{-1};
    std::string label;
};

enum class FrameType {
    Diagonal,
    Initialize,
    Candidate,
    Update,
    Finalize,
    Traceback
};

struct Frame {
    std::size_t index{};
    FrameType type{FrameType::Diagonal};
    std::string message;
    int pseudocodeLine{};
    int i{-1};
    int j{-1};
    int k{-1};
    Cost candidate{INF};
    Cost bestBefore{INF};
    Cost leftCost{INF};
    Cost rightCost{INF};
    Cost multiplyCost{INF};
    std::size_t comparisons{};
    std::size_t updates{};
    std::vector<std::vector<Cost>> costTable;
    std::vector<std::vector<int>> splitTable;
    std::vector<TreeNode> treeNodes;
    std::size_t visibleTreeNodeCount{};
    int activeTreeNodeId{-1};
};

struct Result {
    std::vector<int> dimensions;
    int matrixCount{};
    Cost optimalCost{};
    Cost leftToRightCost{};
    double savingsPercent{};
    std::string parenthesization;
    std::vector<std::vector<Cost>> costTable;
    std::vector<std::vector<int>> splitTable;
    std::vector<TreeNode> treeNodes;
};

std::string frameTypeName(FrameType type);
std::string pseudocodeLine(int line);
std::string formatCost(Cost value);
void validateDimensions(const std::vector<int>& dimensions);
std::string buildParenthesization(const std::vector<std::vector<int>>& split,
                                  int i, int j);
std::vector<Frame> buildFrames(const std::vector<int>& dimensions);
Result solve(const std::vector<int>& dimensions);

} // namespace chainlab

#endif
