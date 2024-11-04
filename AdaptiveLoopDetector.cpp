#include "AdaptiveLoopDetector.h"

AdaptiveLoopDetector::AdaptiveLoopDetector() {}

void AdaptiveLoopDetector::addState(global::HistoryEntry entry) {
    stateHistory.push_back(entry);
}

void AdaptiveLoopDetector::setHistory(std::vector<global::HistoryEntry> history) {
    std::deque<global::HistoryEntry> newHistory(history.begin(), history.end());
    stateHistory = newHistory;
}

void AdaptiveLoopDetector::clearHistory() {
    stateHistory.clear();
}

global::HistoryEntry AdaptiveLoopDetector::getLastEntry()
{
    return stateHistory.back();
}

AdaptiveLoopDetector::LoopInfo AdaptiveLoopDetector::detectLoop() {
    LoopInfo result = { false, 0, 0, {} };
    
    // Need at least 4 states to detect any meaningful loop
    if (stateHistory.size() < 4) {
        return result;
    }

    // Try different window sizes from a quarter of the history size
    // up to half the history length
    for (size_t windowSize = 2; windowSize <= stateHistory.size() / 2; windowSize++) {
        // Check if current window size reveals a loop
        LoopInfo loop = checkForLoopWithSize(windowSize);
        // Return largest loop found        
        if (!result.found || (loop.found && loop.length > result.length)){
            result = loop;
        }        
    }

    return result;
}

AdaptiveLoopDetector::LoopInfo AdaptiveLoopDetector::checkForLoopWithSize(size_t windowSize) {
    LoopInfo result = { false, 0, 0, {} };

    // Get the recent window of states
    std::deque<global::HistoryEntry>::iterator windowEnd = stateHistory.end();
    std::deque<global::HistoryEntry>::iterator windowStart = windowEnd - windowSize;

    // Count how many times this pattern repeats
    size_t repetitions = 0;
    bool repeating = true;
    std::deque<global::HistoryEntry>::iterator compareEnd = windowStart;
    std::deque<global::HistoryEntry>::iterator compareStart = windowStart - windowSize;

    while (std::distance(stateHistory.begin(), compareStart) >= windowSize && repeating)
    {
        if (std::equal(windowStart, windowEnd, compareStart, compareEnd))
        {
            repetitions++;
        }
        else
        {
            repeating = false;
        }
        
        compareStart -= windowSize;
    }  

    // Consider it a loop if pattern appears at least twice
    if (repetitions >= 1) {
        result.found = true;
        result.length = windowSize;
        result.repetitions = repetitions;

        // Store the pattern
        result.pattern.assign(windowStart, windowEnd);
    }

    return result;
}