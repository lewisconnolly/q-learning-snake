#include "Global.h"
#include <deque>
#include <unordered_map>

class AdaptiveLoopDetector {
private:
    std::deque<global::HistoryEntry> stateHistory;

public:
    struct LoopInfo {
        bool found;           // Whether a loop was found
        size_t length;        // Length of the loop
        size_t repetitions;   // Number of times the pattern repeated
        std::vector<global::HistoryEntry> pattern;  // The repeating pattern
    };

    AdaptiveLoopDetector();
    void addState(global::HistoryEntry entry);
    void setHistory(std::vector<global::HistoryEntry> history);
    void clearHistory();
    global::HistoryEntry getLastEntry();
    LoopInfo detectLoop();

private:
    LoopInfo checkForLoopWithSize(size_t windowSize);
};