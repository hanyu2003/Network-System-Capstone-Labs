// lab5_ortools.cc
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <string>
#include <limits>
#include <iomanip>
#include "ortools/linear_solver/linear_solver.h"

using namespace operations_research;

// 結構 Edge：代表一條有向邊
// pair_id = -1 表示物理邊；若不等於 -1，表示這是一條虛擬邊，且僅屬於該 SD pair (索引)
struct Edge {
  int u;
  int v;
  double cap;
  int pair_id;
};

int main() {
  int t;
  std::cin >> t;
  
  // 輸出學生學號 (請自行修改為正確的學號)
  std::cout << "111550164" << "\n";
  
  double overall_throughput = 0.0;
  
  for (int test = 0; test < t; test++) {
    int U, E, F;
    std::cin >> U >> E >> F;
    
    // 儲存物理邊原始順序（每筆 undirected 邊）
    // 並同時建立兩條 directed 物理邊 (pair_id = -1)
    std::vector<std::tuple<int,int,double>> input_edges;
    std::vector<Edge> edges;
    for (int i = 0; i < E; i++) {
      int u, v;
      double cap;
      std::cin >> u >> v >> cap;
      input_edges.push_back({u, v, cap});
      // 依讀入順序產生兩條有向邊 (注意：兩條邊的順序固定：第一為 u->v, 第二為 v->u)
      edges.push_back({u, v, cap, -1});
      edges.push_back({v, u, cap, -1});
    }
    
    // 讀取 SD pair 並同時建立專屬虛擬邊：
    // 虛擬邊1：從虛擬源 S (編號 U) -> SD pair 的 source，容量設為 1e9，pair_id = i
    // 虛擬邊2：從 SD pair 的 destination -> 虛擬匯 D (編號 U+1)，容量設為 1e9，pair_id = i
    std::vector<std::pair<int,int>> sd_pairs;
    for (int i = 0; i < F; i++) {
      int src, dst;
      std::cin >> src >> dst;
      sd_pairs.push_back({src, dst});
      edges.push_back({U, src, 1e9, i});
      edges.push_back({dst, U + 1, 1e9, i});
    }
    
    // 新圖總節點數：原圖 U 個節點，加上 S (編號 U) 與 D (編號 U+1)
    int total_nodes = U + 2;
    int N_edges = edges.size();
    
    // 建立求解器 (CBC MIP)，設定 big-M 常數
    MPSolver solver("lab5_ortools", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);
    double M_big = 1e9;
    
    // ------------------- 變數建立 -------------------
    // Y[f][e] : SD pair f 是否採用邊 e (二元變數)
    std::vector<std::vector<const MPVariable*>> Y(F, std::vector<const MPVariable*>(N_edges, nullptr));
    for (int f = 0; f < F; f++) {
      for (int e = 0; e < N_edges; e++) {
        Y[f][e] = solver.MakeIntVar(0, 1, "Y_" + std::to_string(f) + "_" + std::to_string(e));
      }
    }
    
    // Z[e] : 邊 e 是否被使用 (只要有任一 SD pair 採用則為 1)
    std::vector<const MPVariable*> Z(N_edges, nullptr);
    for (int e = 0; e < N_edges; e++) {
      Z[e] = solver.MakeIntVar(0, 1, "Z_" + std::to_string(e));
    }
    
    // X[f] : SD pair f 的傳輸速率 (連續變數)
    // z[f] : SD pair f 是否啟用 (0 表示完全不傳輸)
    std::vector<const MPVariable*> X(F, nullptr);
    std::vector<const MPVariable*> z(F, nullptr);
    for (int f = 0; f < F; f++) {
      X[f] = solver.MakeNumVar(0.0, solver.infinity(), "X_" + std::to_string(f));
      z[f] = solver.MakeIntVar(0, 1, "z_" + std::to_string(f));
    }
    
    // ------------------- 目標函數 -------------------
    MPObjective* objective = solver.MutableObjective();
    for (int f = 0; f < F; f++) {
      objective->SetCoefficient(X[f], 1);
    }
    objective->SetMaximization();
    
    // ------------------- 流守恆約束 -------------------
    // 對每個 SD pair f 與每個節點 v (v = 0...total_nodes-1) 設定流守恆
    // S (編號 U) : (流出 - 流入) - z[f] = 0
    // D (編號 U+1): (流出 - 流入) + z[f] = 0
    // 其他節點 : 流出 - 流入 = 0
    for (int f = 0; f < F; f++) {
      for (int v = 0; v < total_nodes; v++) {
        MPConstraint* c = solver.MakeRowConstraint(0.0, 0.0, "flow_" + std::to_string(f) + "_" + std::to_string(v));
        for (int e = 0; e < N_edges; e++) {
          if (edges[e].u == v)
            c->SetCoefficient(Y[f][e], 1);
          if (edges[e].v == v)
            c->SetCoefficient(Y[f][e], -1);
        }
        if (v == U) {         // S
          c->SetCoefficient(z[f], -1);
        } else if (v == U + 1) { // D
          c->SetCoefficient(z[f], 1);
        }
      }
    }
    
    // ------------------- 虛擬邊歸屬約束 -------------------
    // 若邊 e 為虛擬邊 (edges[e].pair_id != -1) 且其 pair_id ≠ f，則要求 Y[f][e] = 0
    for (int f = 0; f < F; f++) {
      for (int e = 0; e < N_edges; e++) {
        if (edges[e].pair_id != -1 && edges[e].pair_id != f) {
          MPConstraint* c = solver.MakeRowConstraint(0, 0, "virtual_tag_" + std::to_string(f) + "_" + std::to_string(e));
          c->SetCoefficient(Y[f][e], 1);
        }
      }
    }
    
    // ------------------- Y–Z binding -------------------
    // 若 SD pair f 採用邊 e (Y[f][e] = 1)，則必須讓 Z[e] = 1
    for (int f = 0; f < F; f++) {
      for (int e = 0; e < N_edges; e++) {
        MPConstraint* c = solver.MakeRowConstraint(-solver.infinity(), 0, "bind_" + std::to_string(f) + "_" + std::to_string(e));
        c->SetCoefficient(Y[f][e], 1);
        c->SetCoefficient(Z[e], -1);
      }
    }
    
    // // ------------------- 物理邊唯一使用約束 -------------------
    // // 僅對物理邊 (edges[e].pair_id == -1 且 u,v ∈ [0, U-1]) 施加：所有 SD pair 使用該邊的和 ≤ 1
    // for (int e = 0; e < N_edges; e++) {
    //   if (edges[e].pair_id == -1 && edges[e].u < U && edges[e].v < U) {
    //     MPConstraint* c = solver.MakeRowConstraint(0, 1, "unique_edge_" + std::to_string(e));
    //     for (int f = 0; f < F; f++) {
    //       c->SetCoefficient(Y[f][e], 1);
    //     }
    //   }
    // }
    
    // ------------------- Link Capacity 限制 -------------------
    // 對物理邊 (edges[e].pair_id == -1 且 cap < 1e8) 及每個 SD pair f，設 X[f] + (M_big - cap(e)) * Y[f][e] ≤ M_big
    for (int f = 0; f < F; f++) {
      for (int e = 0; e < N_edges; e++) {
        if (edges[e].pair_id == -1 && edges[e].cap < 1e8) {
          MPConstraint* c = solver.MakeRowConstraint(-solver.infinity(), M_big, "cap_" + std::to_string(f) + "_" + std::to_string(e));
          c->SetCoefficient(X[f], 1);
          c->SetCoefficient(Y[f][e], (M_big - edges[e].cap));
        }
      }
    }
    
    // ------------------- Rate 與啟用綁定 -------------------
    // 若 SD pair 未啟用 (z[f] == 0) 則其傳輸速率必須為 0： X[f] - M_big * z[f] ≤ 0
    for (int f = 0; f < F; f++) {
      MPConstraint* c = solver.MakeRowConstraint(-solver.infinity(), 0, "rate_bind_" + std::to_string(f));
      c->SetCoefficient(X[f], 1);
      c->SetCoefficient(z[f], -M_big);
    }
    
    // ------------------- 單一發射／接收限制 -------------------
    // 對每個物理節點 v ∈ [0, U-1]，僅考慮物理邊，限制出向及入向各 ≤ 1
    for (int v = 0; v < U; v++) {
      MPConstraint* c_out = solver.MakeRowConstraint(0, 1, "transmitter_" + std::to_string(v));
      MPConstraint* c_in  = solver.MakeRowConstraint(0, 1, "receiver_" + std::to_string(v));
      for (int e = 0; e < N_edges; e++) {
        if (edges[e].pair_id == -1) {
          if (edges[e].u == v)
            c_out->SetCoefficient(Z[e], 1);
          if (edges[e].v == v)
            c_in->SetCoefficient(Z[e], 1);
        }
      }
    }
    
    // ------------------- 求解模型 -------------------
    MPSolver::ResultStatus result_status = solver.Solve();
    if (result_status != MPSolver::OPTIMAL) {
      std::cout << "No optimal solution found.\n";
      return 1;
    }
    
    double test_throughput = 0.0;
    
    // ------------------- 輸出使用的物理有向連結 -------------------
    // 根據題目要求，除了 SD pair 路徑部分外，
    // 物理連結的輸出順序要求依"輸入給的順序"進行調整，
    // 但根據你所期望的例子，最終輸出的順序應依邊的起點 (u) 由小到大排序。
    // 因此我們先從 edges 的前 2×E 個（物理邊）中，找出有 SD pair 使用的邊，
    // 接著排序後再輸出。
    std::vector<std::pair<int,int>> used_links;
    for (int e = 0; e < 2 * E; e++) {  // 物理邊儲存在 indices [0, 2*E-1]
      bool used = false;
      for (int f = 0; f < F; f++) {
        if (Y[f][e]->solution_value() > 0.5) {
          used = true;
          break;
        }
      }
      if (used)
        used_links.push_back({edges[e].u, edges[e].v});
    }
    // 按照邊的起點由小到大、起點相同再比終點由小到大排序
    std::sort(used_links.begin(), used_links.end(), [](const std::pair<int,int>& a, const std::pair<int,int>& b) {
      return (a.first == b.first) ? (a.second < b.second) : (a.first < b.first);
    });
    
    std::cout << used_links.size() << "\n";
    for (auto &p : used_links) {
      std::cout << p.first << " " << p.second << "\n";
    }
    
    // ------------------- 重建並輸出每個 SD pair 的路徑 -------------------
    // 從虛擬源 S (編號 U) 沿著 Y[f][e]==1 的邊走到虛擬匯 D (編號 U+1)，
    // 並在輸出時去除 S 與 D，SD pair 輸出順序與讀入順序一致。
    for (int f = 0; f < F; f++) {
      int cur = U;  // 從 S 開始
      std::vector<int> full_path;
      full_path.push_back(cur);
      bool path_found = true;
      while (cur != U + 1) {
        bool found_edge = false;
        for (int e = 0; e < N_edges; e++) {
          if (edges[e].u == cur && Y[f][e]->solution_value() > 0.5) {
            cur = edges[e].v;
            full_path.push_back(cur);
            found_edge = true;
            break;
          }
        }
        if (!found_edge) {
          path_found = false;
          break;
        }
      }
      
      double rf = X[f]->solution_value();
      test_throughput += rf;
      
      if (!path_found || rf < 1e-9) {
        std::cout << "0 0\n";
      } else {
        // 去除虛擬端點 S 與 D
        std::vector<int> actual_path;
        for (size_t i = 1; i + 1 < full_path.size(); i++) {
          actual_path.push_back(full_path[i]);
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << rf << " " << actual_path.size();
        for (size_t i = 0; i < actual_path.size(); i++) {
          oss << " " << actual_path[i];
        }
        std::cout << oss.str() << "\n";
      }
    }
    
    std::cout << std::fixed << std::setprecision(0) << test_throughput << "\n";
    overall_throughput += test_throughput;
  }
  
  std::cout << std::fixed << std::setprecision(6) << overall_throughput / t;
  
  return 0;
}
