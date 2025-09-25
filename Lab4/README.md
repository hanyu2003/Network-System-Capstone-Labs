# Lab4：NS-3 模擬衛星資料傳輸

## 實驗簡介
本次實驗延續 Lab3 的成果，使用 NS-3 模擬地面站與衛星之間的資料傳輸過程。透過模擬 Lab3 的分配結果（Greedy 與 OR-Tools），觀察實際傳輸時間與協議開銷對系統效能的影響。

## 實驗任務

### Task 1：讀取輸入檔案
- 讀取 `network.graph`，取得每個地面站與衛星之間的 datarate。
- 讀取 `network.ortools.out` 或 `network.greedy.out`，取得地面站與衛星的配對結果。
- 建立 `linkRate` 與 `gsToSat` 映射表。

### Task 2：實作 SendPacket
- 取得地面站與衛星節點，初始化接收基準點（避免累積干擾）。
- 設定雙向 datarate。
- 使用 BulkSendHelper 建立 TCP 傳輸應用，設定：
  - `MaxBytes`：最大傳輸量（預設 125000 Bytes）
  - `SendSize`：每次傳輸區塊大小（預設 512 Bytes）
- 安排地面站依序傳送資料，若衛星忙碌則等待下一輪。

### Task 3：實作 EchoRx
- 當衛星接收完一筆資料時，更新該地面站的結束時間與衛星的收集時間。
- 若有尚未開始的地面站且衛星空閒，則啟動下一筆傳輸。
- 使用 `GetTotalRx()` 計算實際接收量，判斷是否達到 `MaxBytes`。

### Task 4：輸出結果
- 輸出至 `lab4.ortools.out` 或 `lab4.greedy.out`，格式包含：
  - 總收集時間（最久的衛星）
  - 每顆衛星的收集時間
  - 每個地面站的傳輸開始與結束時間

## 模擬參數
- 傳輸協定：TCP
- MaxBytes：可調整（預設 125000 Bytes）
- SendSize：512 Bytes
- 輸入檔案：`network.graph`, `network.xxx.out`
- 輸出檔案：`lab4.ortools.out`, `lab4.greedy.out`

## 實驗觀察摘要

### 分配策略比較
- **Greedy**：部分衛星負載過重，導致總收集時間較長。
- **OR-Tools**：分配平均，收集時間較短，效能更佳。

### 協議開銷影響
- `network.xxx.out` 為理論值，未考慮協議開銷。
- NS-3 模擬包含 TCP 握手、封包處理、擁塞控制等，導致實際收集時間增加。

### MaxBytes 調整實驗

- MaxBytes 越大 → 傳輸時間越長
- MaxBytes 越小 → 傳輸時間越短，但差距逐漸縮小，可能受 TCP overhead 限制
- Greedy 對 MaxBytes 調整更敏感，因為分配不均導致延遲累積

## 出處說明
本專案原屬於 NYCU-NETCAP2025 課程帳號（https://github.com/NYCU-NETCAP2025），原由助教建立，因原始倉庫未公開，故另建此專案。
