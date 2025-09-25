#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <iomanip>
using namespace std;

// 讀入無向邊的結構
struct InputEdge {
    int u, v;
    double cap;
};

// 物理 directed 邊的結構
struct Edge {
    int u, v;       // directed edge: from u to v
    double cap;     // 邊容量（原始滿額值，用於 getCandidatePath 初步計算）
    int origIndex;  // 表示該邊所屬的原始 undirected 邊編號（用於更新剩餘容量及後續輸出）
    // 不再維護 used 狀態，允許同一條邊被多個 SD pair 共用
};

// SD pair 結構
struct SDPair {
    int src, dest;
};

// SD pair 結果
struct SDResult {
    double flow;          // 分配到的流量
    vector<int> path;     // 路徑上依序包含的節點（物理節點），若 flow==0 則 path 為空
};

// 用於 BFS 時記錄前驅資訊
struct NodeInfo {
    int parent;   // 前驅節點
    int edgeIdx;  // 從 parent 到當前節點所用的邊的索引（在 directed 邊陣列中的 index）
};

// BFS 函數：在物理網路中從 source 到 dest 搜尋一條有向路徑
// 注意：此 BFS 不檢查 transmitter/receiver 限制（會放到候選檢查中處理）
// U 為物理節點個數（節點編號在 [0, U-1]）
bool bfsPath(int U, int source, int dest,
             const vector<vector<int>> &adj, 
             const vector<Edge> &pEdges,
             vector<NodeInfo> &pred) {
    int n = U;
    vector<bool> visited(n, false);
    queue<int> q;
    q.push(source);
    visited[source] = true;
    pred.assign(n, {-1, -1});
    
    while (!q.empty()){
        int u = q.front();
        q.pop();
        for (int edgeIdx : adj[u]) {
            int v = pEdges[edgeIdx].v;
            if (!visited[v]) {
                visited[v] = true;
                pred[v] = {u, edgeIdx};
                if (v == dest)
                    return true;
                q.push(v);
            }
        }
    }
    return false;
}
    
// 取得從 source 到 dest 的候選路徑，返回候選路徑上 directed 邊的索引序列與經過節點序列，
// 並計算該路徑的初步“潛在流量”，其為所有邊的原始容量最小值（之後會根據剩餘容量重新評估）。
bool getCandidatePath(int U, int source, int dest, const vector<vector<int>> &adj, const vector<Edge> &pEdges, vector<int> &pathEdges, vector<int> &pathNodes, double &potentialFlow) {
    vector<NodeInfo> pred;
    bool found = bfsPath(U, source, dest, adj, pEdges, pred);
    if (!found)
        return false;
    
    // 重建路徑（從 dest 回到 source）
    pathNodes.clear();
    pathEdges.clear();
    int cur = dest;
    while(cur != source) {
        pathNodes.push_back(cur);
        int eIdx = pred[cur].edgeIdx;
        pathEdges.push_back(eIdx);
        cur = pred[cur].parent;
    }
    pathNodes.push_back(source);
    reverse(pathNodes.begin(), pathNodes.end());
    reverse(pathEdges.begin(), pathEdges.end());
    
    // 初步計算：沿候選路徑上所有 directed 邊的原始容量最小值
    potentialFlow = numeric_limits<double>::max();
    for (int idx : pathEdges)
        potentialFlow = min(potentialFlow, pEdges[idx].cap);
    
    return true;
}
    
// 主程式：採用 greedy 策略分配 SD pair 的路徑
// 每次從未分配的 SD pair 中選擇候選路徑（經過檢查後必須滿足新邊若使用時 transmitter/receiver 無衝突且剩餘容量足夠）的 SD pair，
// 並對候選路徑中所有新邊更新剩餘容量，並更新 transmitter/receiver 限制（對新分配的 undirected 邊）。
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int t;
    cin >> t;
    cout << "111550164" << "\n";
    
    double overallThroughput = 0.0;
    
    for (int test = 0; test < t; test++){
        int U, E, F;
        cin >> U >> E >> F;
        
        // 讀取物理無向邊（輸入順序）
        vector<InputEdge> inputEdges(E);
        for (int i = 0; i < E; i++){
            cin >> inputEdges[i].u >> inputEdges[i].v >> inputEdges[i].cap;
        }
        
        // 產生 directed 物理邊（2*E 條），記錄每筆邊所屬的原始 undirected 邊編號
        vector<Edge> pEdges;
        for (int i = 0; i < E; i++){
            // u -> v
            pEdges.push_back({inputEdges[i].u, inputEdges[i].v, inputEdges[i].cap, i});
            // v -> u
            pEdges.push_back({inputEdges[i].v, inputEdges[i].u, inputEdges[i].cap, i});
        }
        int numPhysicalEdges = pEdges.size(); // = 2*E
        
        // 建立 remainingCapacity：對於每筆 undirected 邊，初始容量為輸入容量
        vector<double> remainingCapacity(E);
        for (int i = 0; i < E; i++){
            remainingCapacity[i] = inputEdges[i].cap;
        }
        
        // 用於輸出時記錄哪些 directed 邊曾被 SD pair 路徑使用
        vector<bool> edgeOutputUsed(numPhysicalEdges, false);
        
        // 讀取 SD pair
        vector<SDPair> sdPairs(F);
        for (int i = 0; i < F; i++){
            cin >> sdPairs[i].src >> sdPairs[i].dest;
        }
        
        // 建立物理圖鄰接串列：僅考慮節點編號 0 ~ U-1
        vector<vector<int>> adj(U);
        for (int i = 0; i < numPhysicalEdges; i++){
            if (pEdges[i].u >= 0 && pEdges[i].u < U)
                adj[pEdges[i].u].push_back(i);
        }
        
        // 用於單一 transmitter/receiver 限制：
        // 每個物理節點最多只被新分配一個出向或入向物理邊「佔用」
        vector<bool> usedOutgoing(U, false);
        vector<bool> usedIncoming(U, false);
        
        // 儲存每個 SD pair 的結果（分配的流量與重建的路徑）
        vector<SDResult> sdResults(F, {0.0, {}});
        vector<bool> assigned(F, false);
        
        double testThroughput = 0.0;
        
        // Greedy loop：當還有未分配 SD pair 存在時
        while (true) {
            double bestFlow = 0.0;
            int bestIdx = -1;
            vector<int> bestPathEdges, bestPathNodes;
            
            // 嘗試所有未分配的 SD pair
            for (int i = 0; i < F; i++){
                if (assigned[i]) continue;
                int src = sdPairs[i].src, dest = sdPairs[i].dest;
                vector<int> candEdges, candNodes;
                double potFlow;
                if (getCandidatePath(U, src, dest, adj, pEdges, candEdges, candNodes, potFlow)) {
                    // 重新計算候選路徑實際可用的流量，根據剩餘容量更新
                    double availableFlow = numeric_limits<double>::max();
                    bool candidateFeasible = true;
                    for (int edgeIdx : candEdges) {
                        int undirected = pEdges[edgeIdx].origIndex;
                        availableFlow = min(availableFlow, remainingCapacity[undirected]);
                        // 若此邊尚未被使用，則要求該物理節點的 transmitter/receiver尚未被佔用
                        if (! (remainingCapacity[undirected] < pEdges[edgeIdx].cap)) {
                            int u = pEdges[edgeIdx].u;
                            int v = pEdges[edgeIdx].v;
                            if (usedOutgoing[u] || usedIncoming[v]) {
                                candidateFeasible = false;
                                break;
                            }
                        }
                    }
                    if (!candidateFeasible)
                        continue;
                    
                    // 可分配的流量取兩者最小值
                    double candFlow = min(potFlow, availableFlow);
                    
                    if (candFlow > bestFlow) {
                        bestFlow = candFlow;
                        bestIdx = i;
                        bestPathEdges = candEdges;
                        bestPathNodes = candNodes;
                    }
                }
            }
            
            if (bestIdx == -1 || bestFlow < 1e-9)
                break; // 沒有找到可分配的 SD pair
            
            // 記錄該 SD pair 的分配結果
            sdResults[bestIdx].flow = bestFlow;
            sdResults[bestIdx].path = bestPathNodes;
            testThroughput += bestFlow;
            assigned[bestIdx] = true;
            
            // 更新候選路徑上所有邊的剩餘容量及節點 transmitter/receiver 狀態
            // 遍歷候選路徑中的每一個 directed 邊
            for (int edgeIdx : bestPathEdges) {
                int undirected = pEdges[edgeIdx].origIndex;
                // 扣除本次分配的流量
                remainingCapacity[undirected] -= bestFlow;
                // 若這筆 undirected 邊第一次被使用（即仍有剩餘且此前為未使用），更新節點狀態
                if (remainingCapacity[undirected] + bestFlow == pEdges[edgeIdx].cap) {
                    usedOutgoing[pEdges[edgeIdx].u] = true;
                    usedIncoming[pEdges[edgeIdx].v] = true;
                }
                edgeOutputUsed[edgeIdx] = true; // 此 directed 邊被使用過
            }
        }
        
        // ------------------- 輸出物理連結 -------------------
        // 根據原始輸入順序，對於每個 undirected 邊，
        // 如果該邊的兩個 directed 邊中至少有一個被使用（edgeOutputUsed 為 true），則輸出該邊
        vector<pair<int,int>> usedLinks;
        for (int i = 0; i < E; i++){
            int idx1 = 2 * i;
            int idx2 = 2 * i + 1;
            if (edgeOutputUsed[idx1] || edgeOutputUsed[idx2]) {
                if (edgeOutputUsed[idx1])
                    usedLinks.push_back({pEdges[idx1].u, pEdges[idx1].v});
                else
                    usedLinks.push_back({pEdges[idx2].u, pEdges[idx2].v});
            }
        }
        
        sort(usedLinks.begin(), usedLinks.end(), [](const pair<int,int>& a, const pair<int,int>& b){
            return (a.first == b.first) ? (a.second < b.second) : (a.first < b.first);
        });
        cout << usedLinks.size() << "\n";
        for (auto &p : usedLinks)
            cout << p.first << " " << p.second << "\n";
        
        // ------------------- 輸出每個 SD pair 的路徑 -------------------
        // 依 SD pair 輸入順序，輸出其重建路徑（節點序列）與分配的流量
        for (int i = 0; i < F; i++){
            if (sdResults[i].flow < 1e-9 || sdResults[i].path.empty()){
                cout << "0 0\n";
            } else {
                ostringstream oss;
                oss << fixed << setprecision(0) << sdResults[i].flow << " " << sdResults[i].path.size();
                for (int node : sdResults[i].path)
                    oss << " " << node;
                cout << oss.str() << "\n";
            }
        }
        cout << fixed << setprecision(0) << testThroughput << "\n";
        overallThroughput += testThroughput;
    }
    
    cout << fixed << setprecision(6) << overallThroughput / t;
    
    return 0;
}
