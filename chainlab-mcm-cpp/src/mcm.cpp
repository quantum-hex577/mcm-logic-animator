#include "mcm.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace chainlab {
namespace {

using GridCost = std::vector<std::vector<Cost>>;
using GridSplit = std::vector<std::vector<int>>;

void appendFrame(std::vector<Frame>& frames,
                 FrameType type,
                 const std::string& message,
                 int line,
                 int i,
                 int j,
                 int k,
                 Cost candidate,
                 Cost bestBefore,
                 Cost leftCost,
                 Cost rightCost,
                 Cost multiplyCost,
                 std::size_t comparisons,
                 std::size_t updates,
                 const GridCost& cost,
                 const GridSplit& split,
                 const std::vector<TreeNode>& tree = {},
                 std::size_t visibleTreeNodeCount = 0,
                 int activeTreeNodeId = -1) {
    Frame frame;
    frame.index = frames.size();
    frame.type = type;
    frame.message = message;
    frame.pseudocodeLine = line;
    frame.i = i;
    frame.j = j;
    frame.k = k;
    frame.candidate = candidate;
    frame.bestBefore = bestBefore;
    frame.leftCost = leftCost;
    frame.rightCost = rightCost;
    frame.multiplyCost = multiplyCost;
    frame.comparisons = comparisons;
    frame.updates = updates;
    frame.costTable = cost;
    frame.splitTable = split;
    frame.treeNodes = tree;
    frame.visibleTreeNodeCount = visibleTreeNodeCount;
    frame.activeTreeNodeId = activeTreeNodeId;
    frames.push_back(std::move(frame));
}

void buildTreeRecursive(const GridSplit& split,
                        int i,
                        int j,
                        int parentId,
                        int depth,
                        std::vector<TreeNode>& nodes) {
    const int id = static_cast<int>(nodes.size());
    const bool leaf = i == j;
    const int k = leaf ? -1 : split[i][j];

    TreeNode node;
    node.id = id;
    node.i = i;
    node.j = j;
    node.split = k;
    node.depth = depth;
    node.parentId = parentId;
    if (leaf) {
        node.label = "A" + std::to_string(i + 1);
    } else {
        node.label = "A" + std::to_string(i + 1) + "..A" + std::to_string(j + 1);
    }
    nodes.push_back(node);

    if (!leaf) {
        buildTreeRecursive(split, i, k, id, depth + 1, nodes);
        buildTreeRecursive(split, k + 1, j, id, depth + 1, nodes);
    }
}

Cost leftToRightCost(const std::vector<int>& p) {
    const int n = static_cast<int>(p.size()) - 1;
    Cost total = 0;
    // Left-to-right folding: ((A1*A2)*A3)...*An.
    for (int i = 1; i < n; ++i) {
        total += static_cast<Cost>(p[0]) * p[i] * p[i + 1];
    }
    return total;
}

} // namespace

std::string frameTypeName(FrameType type) {
    switch (type) {
        case FrameType::Diagonal: return "BASE CASE";
        case FrameType::Initialize: return "INITIALIZE SUBCHAIN";
        case FrameType::Candidate: return "EVALUATE SPLIT";
        case FrameType::Update: return "UPDATE BEST COST";
        case FrameType::Finalize: return "FINALIZE SUBCHAIN";
        case FrameType::Traceback: return "TRACEBACK";
    }
    return "UNKNOWN";
}

std::string pseudocodeLine(int line) {
    static const std::vector<std::string> lines = {
        "m[i][i] = 0                                      // one matrix costs zero",
        "for (L = 2; L <= n; ++L)",
        "  for (i = 0; i <= n - L; ++i)",
        "    j = i + L - 1",
        "    m[i][j] = INF; s[i][j] = UNDEFINED",
        "    for (k = i; k < j; ++k)",
        "      q = m[i][k] + m[k+1][j] + p[i]*p[k+1]*p[j+1]",
        "      if (q < m[i][j])",
        "        m[i][j] = q; s[i][j] = k",
        "    end for k",
        "    finalize interval [i, j]",
        "traceback(i, j): recursively follow stored split s[i][j]"
    };
    if (line < 0 || line >= static_cast<int>(lines.size())) return "";
    return lines[static_cast<std::size_t>(line)];
}

std::string formatCost(Cost value) {
    if (value >= INF) return "INF";
    return std::to_string(value);
}

void validateDimensions(const std::vector<int>& dimensions) {
    if (dimensions.size() < 3 || dimensions.size() > 9) {
        throw std::invalid_argument(
            "Enter 3 to 9 dimension values (representing 2 to 8 matrices).");
    }
    for (int value : dimensions) {
        if (value <= 0 || value > 10000) {
            throw std::invalid_argument(
                "Every dimension must be an integer from 1 through 10000.");
        }
    }
}

std::string buildParenthesization(const GridSplit& split, int i, int j) {
    if (i == j) return "A" + std::to_string(i + 1);
    const int k = split[i][j];
    if (k < i || k >= j) {
        throw std::logic_error("Invalid split table while reconstructing parenthesization.");
    }
    return "(" + buildParenthesization(split, i, k) + " * " +
           buildParenthesization(split, k + 1, j) + ")";
}

std::vector<Frame> buildFrames(const std::vector<int>& p) {
    validateDimensions(p);
    const int n = static_cast<int>(p.size()) - 1;
    GridCost cost(n, std::vector<Cost>(n, INF));
    GridSplit split(n, std::vector<int>(n, -1));
    std::vector<Frame> frames;
    std::size_t comparisons = 0;
    std::size_t updates = 0;

    // A single matrix requires no multiplication.
    for (int i = 0; i < n; ++i) {
        cost[i][i] = 0;
        appendFrame(frames, FrameType::Diagonal,
                    "Base case: matrix A" + std::to_string(i + 1) +
                    " requires zero scalar multiplications.",
                    0, i, i, -1, INF, INF, INF, INF, INF,
                    comparisons, updates, cost, split);
    }

    // Fill the table by increasing subchain length.
    for (int length = 2; length <= n; ++length) {
        for (int i = 0; i <= n - length; ++i) {
            const int j = i + length - 1;
            cost[i][j] = INF;
            split[i][j] = -1;
            appendFrame(frames, FrameType::Initialize,
                        "Begin subchain A" + std::to_string(i + 1) + "..A" +
                        std::to_string(j + 1) + "; no candidate has been selected yet.",
                        4, i, j, -1, INF, INF, INF, INF, INF,
                        comparisons, updates, cost, split);

            for (int k = i; k < j; ++k) {
                const Cost left = cost[i][k];
                const Cost right = cost[k + 1][j];
                const Cost multiply = static_cast<Cost>(p[i]) * p[k + 1] * p[j + 1];
                const Cost candidate = left + right + multiply;
                const Cost bestBefore = cost[i][j];
                ++comparisons;
                std::ostringstream explanation;
                explanation << "Try split after A" << (k + 1) << ": "
                            << left << " + " << right << " + " << multiply
                            << " = " << candidate << ".";
                appendFrame(frames, FrameType::Candidate, explanation.str(),
                            6, i, j, k, candidate, bestBefore, left, right, multiply,
                            comparisons, updates, cost, split);

                if (candidate < cost[i][j]) {
                    cost[i][j] = candidate;
                    split[i][j] = k;
                    ++updates;
                    appendFrame(frames, FrameType::Update,
                                "New best cost " + std::to_string(candidate) +
                                " found for A" + std::to_string(i + 1) + "..A" +
                                std::to_string(j + 1) + "; store split after A" +
                                std::to_string(k + 1) + ".",
                                8, i, j, k, candidate, bestBefore, left, right, multiply,
                                comparisons, updates, cost, split);
                }
            }

            appendFrame(frames, FrameType::Finalize,
                        "Subchain A" + std::to_string(i + 1) + "..A" +
                        std::to_string(j + 1) + " is complete; minimum cost = " +
                        std::to_string(cost[i][j]) + ".",
                        10, i, j, split[i][j], cost[i][j], INF, INF, INF, INF,
                        comparisons, updates, cost, split);
        }
    }

    std::vector<TreeNode> tree;
    buildTreeRecursive(split, 0, n - 1, -1, 0, tree);
    for (std::size_t visible = 1; visible <= tree.size(); ++visible) {
        const TreeNode& node = tree[visible - 1];
        std::string message;
        if (node.i == node.j) {
            message = "Traceback reaches leaf A" + std::to_string(node.i + 1) + ".";
        } else {
            message = "Traceback at A" + std::to_string(node.i + 1) + "..A" +
                      std::to_string(node.j + 1) + ": follow the split after A" +
                      std::to_string(node.split + 1) + ".";
        }
        appendFrame(frames, FrameType::Traceback, message, 11,
                    node.i, node.j, node.split, INF, INF, INF, INF, INF,
                    comparisons, updates, cost, split, tree, visible, node.id);
    }

    return frames;
}

Result solve(const std::vector<int>& p) {
    const std::vector<Frame> frames = buildFrames(p);
    if (frames.empty()) throw std::logic_error("MCM frame sequence unexpectedly empty.");
    const Frame& lastDpFrame = *std::find_if(frames.rbegin(), frames.rend(), [](const Frame& f) {
        return f.type == FrameType::Finalize;
    });
    const int n = static_cast<int>(p.size()) - 1;
    Result result;
    result.dimensions = p;
    result.matrixCount = n;
    result.costTable = lastDpFrame.costTable;
    result.splitTable = lastDpFrame.splitTable;
    result.optimalCost = result.costTable[0][n - 1];
    result.leftToRightCost = leftToRightCost(p);
    result.savingsPercent = result.leftToRightCost == 0 ? 0.0 :
        100.0 * static_cast<double>(result.leftToRightCost - result.optimalCost) /
        static_cast<double>(result.leftToRightCost);
    result.parenthesization = buildParenthesization(result.splitTable, 0, n - 1);
    buildTreeRecursive(result.splitTable, 0, n - 1, -1, 0, result.treeNodes);
    return result;
}

} // namespace chainlab
