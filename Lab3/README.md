# Lab3：Network Optimization with OR-Tools

## 實驗簡介
本次實驗延續 Lab1 與 Lab2 的成果，進一步結合 Google OR-Tools 解決衛星與地面站之間的資料傳輸最佳化問題。目標是透過數學模型與演算法，找出最有效率的衛星分配方式，並比較 Greedy 演算法與 OR-Tools 的表現差異。

## 實驗任務

### Task 1：計算 Data Rate
- 讀取 `network.pos` 檔案，包含 Tx power、noise、frequency、bandwidth、Rx power threshold 等參數。
- 解析地面站與衛星的座標，計算每個可能連線的 link 的 datarate。
- 使用 Lab1/Lab2 的 beamforming 方法，計算 optimal beam 與 Tx gain。
- 若 Rx power ≥ threshold，則建立 link 並計算其 datarate。
- 輸出 `network.graph` 作為 OR-Tools 的輸入。

### Task 2：OR-Tools Program
- 讀取 `network.graph`，計算每條 link 的傳輸時間（1000kb / datarate）。
- 建立 ILP 模型：
  - 每個地面站只能連接一個衛星。
  - 每顆衛星的資料收集時間不得超過最大時間。
  - 目標為最小化最大資料收集時間。
- 輸出結果至 `network.ortools.out`。

### Task 3：Greedy Program
- 每個地面站選擇 datarate 最高的衛星。
- 計算每顆衛星的資料收集時間。
- 輸出結果至 `network.greedy.out`。

## 模擬參數
- 資料單位大小：1000 kb
- 輸入檔案：`network.pos`
- 輸出檔案：`network.graph`, `network.ortools.out`, `network.greedy.out`

## 實驗觀察摘要
- **Greedy**：分配不均，部分衛星過載，部分未使用。最大傳輸時間較高。
- **OR-Tools**：分配平均，每顆衛星負載接近，最大傳輸時間明顯較低。
- **執行效率**：Greedy 快速但不保證最佳解；OR-Tools 計算較久但結果更公平。

## 優缺點比較

### Greedy
- 優點：
  - 實作簡單、計算快速
  - 適合即時處理或初步分配
- 缺點：
  - 分配容易不平均
  - 無法保證最佳解

### OR-Tools
- 優點：
  - 分配結果平均，有效降低最大負載
  - 適合需要效率與公平性的場景
- 缺點：
  - 計算複雜、執行時間較長
  - 需額外學習與設定 OR-Tools 工具與模型

## 出處說明
本專案原屬於 NYCU-NETCAP2025 課程帳號（https://github.com/NYCU-NETCAP2025），原由助教建立，因原始倉庫未公開，故另建此專案。
