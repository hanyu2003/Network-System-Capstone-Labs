#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <limits>

using namespace std;

struct Link {
    int ground_station;
    int satellite;
    double data_rate;
};

// 讀取 network.graph
vector<Link> read_graph(string filename, int &num_gs, int &num_sat) {
    ifstream file(filename);
    vector<Link> links;

    if (!file) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    file >> num_gs >> num_sat;
    int num_links;
    file >> num_links;

    for (int i = 0; i < num_links; i++) {
        Link link;
        file >> link.ground_station >> link.satellite >> link.data_rate;
        links.push_back(link);
    }

    file.close();
    return links;
}

// 執行 Greedy 分配演算法
map<int, int> greedy_assignment(const vector<Link>& links) {
    map<int, int> assignment;
    map<int, double> max_data_rate;

    // 每個 Ground Station 選擇數據率最高的 Satellite
    for (const auto& link : links) {
        if (max_data_rate.find(link.ground_station) == max_data_rate.end() ||
            link.data_rate > max_data_rate[link.ground_station]) {
            max_data_rate[link.ground_station] = link.data_rate;
            assignment[link.ground_station] = link.satellite;
        }
    }

    return assignment;
}

// 計算 Satellite 的數據收集時間
map<int, double> calculate_collection_time(const vector<Link>& links, const map<int, int>& assignment) {
    map<int, double> collection_time;

    for (const auto& link : links) {
        if (assignment.at(link.ground_station) == link.satellite) {
            collection_time[link.satellite] += 1000.0 / link.data_rate; // 假設 1000kb 為 1 個數據單元
        }
    }

    return collection_time;
}

// 輸出結果到 network.greedy.out
void write_output(string filename, const map<int, int>& assignment, const map<int, double>& collection_time) {
    ofstream file(filename);
    
    if (!file) {
        cerr << "Error: Unable to create file " << filename << endl;
        exit(1);
    }

    // 計算最大傳輸時間
    double max_time = 0;
    for (const auto& entry : collection_time) {
        max_time = max(max_time, entry.second);
    }

    file << max_time << endl;

    // Ground Station - Satellite 分配結果
    for (const auto& entry : assignment) {
        file << entry.first << " " << entry.second << endl;
    }

    // 每個 Satellite 的數據收集時間
    for (const auto& entry : collection_time) {
        file << entry.first << " " << entry.second << endl;
    }

    file.close();
}

int main() {
    string input_file = "network.graph";
    string output_file = "network.greedy.out";
    
    int num_gs, num_sat;
    vector<Link> links = read_graph(input_file, num_gs, num_sat);

    // 執行 Greedy 分配演算法
    map<int, int> assignment = greedy_assignment(links);
    map<int, double> collection_time = calculate_collection_time(links, assignment);

    // 輸出結果
    write_output(output_file, assignment, collection_time);

    cout << "✅ `network.greedy.out` 已生成！" << endl;
    return 0;
}
