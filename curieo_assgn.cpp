#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <tuple>
#include <cfloat>

using namespace std;

struct LogEntry {
    long long timestamp;
    string logType;
    double severity;
};

struct CurrentStats {
    double min;
    double max;
    double sum;
    int count;
};

class ErrorLogMonitor {
private:
    map<long long, LogEntry> timestampMap;
    multimap<string, LogEntry> logTypeMap;
    unordered_map<string, tuple<double, double, double, double>> logTypeStats;

    tuple<double, double, double> globalStats;
    int numEntries;
    vector<tuple<long long, CurrentStats>> timestampStats;

public:
    ErrorLogMonitor() : globalStats{0.0, 0.0, 0.0}, numEntries(0) {}

    void insertLog(long long timestamp, const string& logType, double severity) {
        LogEntry newEntry = {timestamp, logType, severity};
        timestampMap[timestamp] = newEntry;
        logTypeMap.insert({logType, newEntry});

        updateGlobalStats(severity);
        updateLogTypeStats(logType, severity);
        updateTimestampStats(timestamp);

        cout<<"No output"<<endl;
    }

    void updateGlobalStats(double severity) {
        if(get<0>(globalStats)==0) get<0>(globalStats)=severity;
        else get<0>(globalStats) = min(get<0>(globalStats), severity);
        get<1>(globalStats) = max(get<1>(globalStats), severity);
        get<2>(globalStats) += severity;
        numEntries++;
    }

    void updateLogTypeStats(const string& logType, double severity) {
        auto& stats = logTypeStats[logType];
        if(get<0>(stats)==0) get<0>(stats)=severity;
        else get<0>(stats) = min(get<0>(stats), severity);
        get<1>(stats) = max(get<1>(stats), severity);
        get<2>(stats) += severity;
        get<3>(stats) += 1;
    }

    void updateTimestampStats(long long timestamp) {
        CurrentStats newStats = {get<0>(globalStats), get<1>(globalStats), get<2>(globalStats), numEntries};
        timestampStats.push_back(make_tuple(timestamp, newStats));
    }

    tuple<double, double, double> computeLogTypeStats(const string& logType) {
        auto it = logTypeStats.find(logType);
        if (it != logTypeStats.end()) {
            auto& stats = it->second;
            double meanSeverity = get<2>(stats) / get<3>(stats);
            // cout<<"Min: "<<get<0>(stats)<<", Max: "<<get<1>(stats)<<", Mean: "<<meanSeverity<<endl;
            return make_tuple(get<0>(stats), get<1>(stats), meanSeverity);
        }
        return make_tuple(0.0, 0.0, 0.0);
        // cout<<"Min: 0.0, Max: , Mean: 0.0"<<endl;
    }

    tuple<double, double, double> computeBeforeTimestampStats(long long timestamp) {
        auto it = timestampMap.lower_bound(timestamp);
        if(it==timestampMap.begin()) return make_tuple(0.0, 0.0, 0.0);
        it--;
        long long beforeTimestamp = it->first;
        int idx = searchTimestampStats(timestampStats, beforeTimestamp);
        double meanSeverity = get<2>(globalStats) / numEntries;
        return make_tuple(get<1>(timestampStats[idx]).min, get<1>(timestampStats[idx]).max, get<1>(timestampStats[idx]).sum/get<1>(timestampStats[idx]).count);
    }

    tuple<double, double, double> computeAfterTimestampStats(long long timestamp) {
        auto it = timestampMap.upper_bound(timestamp);
        if(it==timestampMap.end()) return make_tuple(0.0, 0.0, 0.0);
        if(it==timestampMap.begin()) return make_tuple(get<0>(globalStats), get<1>(globalStats), get<2>(globalStats)/numEntries);
        long long afterTimestamp = it->first;
        int idx = searchTimestampStats(timestampStats, afterTimestamp);
        double minSeverity = 0.0, maxSeverity = 0.0, meanSeverity = 0.0;
        meanSeverity = (get<1>(timestampStats[timestampStats.size()-1]).sum - get<1>(timestampStats[idx]).sum)/(get<1>(timestampStats[timestampStats.size()-1]).count - get<1>(timestampStats[idx]).count);
        for(int i = idx+1; i<timestampStats.size(); i++) {
            if(minSeverity==0.0) minSeverity = get<1>(timestampStats[i]).min;
            else minSeverity = min(minSeverity, get<1>(timestampStats[i]).min);
            maxSeverity = max(maxSeverity, get<1>(timestampStats[i]).max);
        }
        return make_tuple(minSeverity, maxSeverity, meanSeverity);
    }

    tuple<double, double, double> computeBeforeTimestampLogTypeStats(const string& logType, long long timestamp) {
        auto range = logTypeMap.equal_range(logType);
        double minSeverity = 0.0, maxSeverity = 0.0, sumSeverity = 0.0;
        int count = 0;
        for (auto it = range.first; it != range.second && it->second.timestamp < timestamp; ++it) {
            double severity = it->second.severity;
            if(minSeverity==0.0) minSeverity = severity;
            else minSeverity = min(minSeverity, severity);
            maxSeverity = max(maxSeverity, severity);
            sumSeverity += severity;
            count++;
        }
        if(count==0) return make_tuple(0.0, 0.0, 0.0);
        double meanSeverity = sumSeverity / count;
        return make_tuple(minSeverity, maxSeverity, meanSeverity);
    }

    tuple<double, double, double> computeAfterTimestampLogTypeStats(const string& logType, long long timestamp) {
        auto range = logTypeMap.equal_range(logType);
        double minSeverity = 0.0, maxSeverity = 0.0, sumSeverity = 0.0;
        int count = 0;
        for (auto it = range.first; it != range.second && it->second.timestamp > timestamp; ++it) {
            double severity = it->second.severity;
            if(minSeverity==0.0) minSeverity = severity;
            else minSeverity = min(minSeverity, severity);
            maxSeverity = max(maxSeverity, severity);
            sumSeverity += severity;
            count++;
        }
        if(count==0) return make_tuple(0.0, 0.0, 0.0);
        double meanSeverity = sumSeverity / count;
        return make_tuple(minSeverity, maxSeverity, meanSeverity);
    }

private:

    int searchTimestampStats(const vector<tuple<long long, CurrentStats>>& arr, long long timestamp) {
        int low = 0;
        int high = arr.size() - 1;

        while (low <= high) {
            int mid = low + (high - low) / 2;

            if (get<0>(arr[mid]) == timestamp) {
                return mid;
            } else if (get<0>(arr[mid]) < timestamp) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        return -1;
    }
};

int main() {

    ErrorLogMonitor monitor;

    monitor.insertLog(1715744138011, "INTERNAL_SERVER_ERROR", 23.72);
    monitor.insertLog(1715744138012, "INTERNAL_SERVER_ERROR", 10.17);
    auto result1 = monitor.computeLogTypeStats("INTERNAL_SERVER_ERROR");
    monitor.insertLog(1715744138012, "BAD_REQUEST", 15.22);
    monitor.insertLog(1715744138013, "INTERNAL_SERVER_ERROR", 23.72);
    auto result2 = monitor.computeBeforeTimestampStats(1715744138011);
    auto result3 = monitor.computeAfterTimestampStats(1715744138010);
    auto result4 = monitor.computeLogTypeStats("BAD_REQUEST");
    auto result5 = monitor.computeBeforeTimestampLogTypeStats("INTERNAL_SERVER_ERROR", 1715744138011);
    auto result6 = monitor.computeAfterTimestampLogTypeStats("INTERNAL_SERVER_ERROR", 1715744138010);


    cout << "Min Severity: " << get<0>(result1) << ", Max Severity: " << get<1>(result1) << ", Mean Severity: " << get<2>(result1) << endl;
    cout << "Before Timestamp - Min Severity: " << get<0>(result2) << ", Max Severity: " << get<1>(result2) << ", Mean Severity: " << get<2>(result2) << endl;
    cout << "After Timestamp - Min Severity: " << get<0>(result3) << ", Max Severity: " << get<1>(result3) << ", Mean Severity: " << get<2>(result3) << endl;
    cout << "Before Timestamp & Log Type - Min Severity: " << get<0>(result4) << ", Max Severity: " << get<1>(result4) << ", Mean Severity: " << get<2>(result4) << endl;
    cout << "After Timestamp & Log Type - Min Severity: " << get<0>(result5) << ", Max Severity: " << get<1>(result5) << ", Mean Severity: " << get<2>(result5) << endl;
    cout << "Min Severity: " << get<0>(result6) << ", Max Severity: " << get<1>(result6) << ", Mean Severity: " << get<2>(result6) << endl;



    // Sample Input

    // 1 1715744138011;INTERNAL_SERVER_ERROR;23.72
    // 1 1715744138012;INTERNAL_SERVER_ERROR;10.17
    // 2 INTERNAL_SERVER_ERROR
    // 1 1715744138012;BAD_REQUEST;15.22
    // 1 1715744138013;INTERNAL_SERVER_ERROR;23.72
    // 3 BEFORE 1715744138011
    // 3 AFTER 1715744138010
    // 2 BAD_REQUEST
    // 4 BEFORE INTERNAL_SERVER_ERROR 1715744138011
    // 4 AFTER INTERNAL_SERVER_ERROR 1715744138010

    // Sample Output

    // No output
    // No output
    // Min: 10.17, Max: 23.72, Mean: 16.945
    // No output
    // No output
    // Min: 0.0, Max: 0.0, Mean: 0.0
    // Min: 10.17, Max: 23.72, Mean: 18.2075
    // Min: 15.22, Max: 15.22, Mean: 15.22
    // Min: 0.0, Max: 0.0, Mean: 0.0
    // Min: 10.17, Max: 23.72, Mean: 19.203333

    return 0;
}
