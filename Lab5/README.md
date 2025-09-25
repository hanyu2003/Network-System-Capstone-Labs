# Lab5：Load Balancing Routing Optimization

## 實驗簡介
本次實驗目標是針對一組 source-destination (SD) 配對，在給定的網路拓撲下，設計一條路徑使得整體 throughput 最大化。除了使用 OR-Tools 解整數線性規劃（ILP）模型外，也需設計一個多載平衡的啟發式演算法（MyAlgo），並與 baseline 演算法進行比較。

## 實驗任務

### Task 1：建構 ILP 模型
- 問題建模：
  - 單一路徑限制（無迴圈）
  - 每條 link 的流量不得超過其容量
  - 每個節點最多只能有一個傳送與接收 link
  - 流量守恆：進出流量相等
- 變數定義：
  - `X[f,u,v]`：SD pair f 在 link (u,v) 上的資料率
  - `Y[f,u,v]`：是否使用該 link（binary）
  - `Z[u,v]`：該 link 是否被任何 SD pair 使用（binary）
- 額外約束：
  1. **Y-Z 綁定**：若某 SD pair 使用某 link，則該 link 被視為「使用中」
  2. **單一傳送器限制**：每個節點最多只能有一個 outgoing link
  3. **單一接收器限制**：每個節點最多只能有一個 incoming link

### Task 2：OR-Tools 程式
- 使用 OR-Tools 解 ILP 模型，實作於 `lab5_ortools.cc`
- 輸入：`network.graph`
- 輸出：`network.ortools.out`
- 可使用 `sample.graph` 測試，結果不需完全一致，但 throughput 必須相同

### Task 3：設計 MyAlgo
- 設計一個多載平衡演算法，需符合所有 ILP 約束條件
- 執行時間需為多項式時間（不可暴力解）
- 輸出結果需優於 baseline（平均 throughput 更高）
- 實作於 `lab5_myalgo.cpp`，並提交至 Codeforces 驗證正確性

  
## 模擬參數與格式
- 輸入檔案：`network.graph`
- 輸出檔案：
  - OR-Tools：`network.ortools.out`
  - MyAlgo：`network.myalgo.out`
- 使用 stdin/stdout 執行程式：
  ```bash
  ./program < input.txt > output.txt
  
## 實驗觀察摘要
### MyAlgo 設計理念
- 基於 BFS 找出所有可能路徑
- 評估每條路徑的負載與剩餘容量
- 優先選擇負載較低的路徑以達到多載平衡
- 避免使用已接近容量上限的 link

### MyAlgo 時間複雜度分析
- BFS 尋路：O(V + E)
- 每個 SD pair 評估路徑：O(k × (V + E))，其中 k 為 SD pair 數量
- 整體為多項式時間，符合要求

### 平均 throughput 比例
- Ratio = average throughput of network.myalgo.out / average throughput of network.ortools.out
（請根據實際結果補上數值）

## 出處說明
本專案原屬於 NYCU-NETCAP2025 課程帳號（https://github.com/NYCU-NETCAP2025），原由助教建立，因原始倉庫未公開，故另建此專案。
