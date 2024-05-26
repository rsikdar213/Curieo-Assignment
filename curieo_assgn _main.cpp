#include <bits/stdc++.h>
using namespace std;

struct LogEntry {
    long long timestamp;
    string logType;
    double severity;

    LogEntry(long long ts, const string& type, double sev) : timestamp(ts), logType(type), severity(sev) {}
};

class ErrorLogMonitor {
private:
    priority_queue<LogEntry, vector<LogEntry>, greater<LogEntry>> minHeap;
    priority_queue<LogEntry, vector<LogEntry>, less<LogEntry>> maxHeap;

public:
    void submitLog(long long timestamp, const string& logType, double severity) {
        LogEntry logEntry(timestamp, logType, severity);
        minHeap.push(logEntry);
        maxHeap.push(logEntry);
    }

    pair<double, pair<double, double>> getStatsBefore(long long timestamp) {
        while (!maxHeap.empty() && maxHeap.top().timestamp >= timestamp) {
            maxHeap.pop();
        }
        // Compute statistics using maxHeap
    }

    pair<double, pair<double, double>> getStatsAfter(long long timestamp) {
        while (!minHeap.empty() && minHeap.top().timestamp <= timestamp) {
            minHeap.pop();
        }
        // Compute statistics using minHeap
    }

    pair<double, pair<double, double>> getStatsBeforeOfType(const string& logType, long long timestamp) {
        // Implement based on your requirements
    }

    pair<double, pair<double, double>> getStatsAfterOfType(const string& logType, long long timestamp) {
        // Implement based on your requirements
    }

    pair<double, pair<double, double>> computeStats(const vector<LogEntry>& entries) {
        // Implement based on your requirements
    }
};

int main() {
    ErrorLogMonitor monitor;

    // Sample usage
    monitor.submitLog(1715744138011, "INTERNAL_SERVER_ERROR", 23.72);
    // Submit more logs...

    // Example operations:

    return 0;
}
