# Lab6：Nix-Vector Routing Path Simulation

## 實驗簡介

本實驗透過 NS-3 撰寫程式，模擬使用 nix-vector-routing 套件進行負載平衡傳輸。實驗目標包括設定使用者自定義路徑、追蹤封包傳輸路徑，以及比較不同路徑組的總吞吐量。

## 實驗任務

### Task 1：設定路由路徑
- 修改 NixVectorRouting 套件以支援使用者自定義路徑（由輸入檔提供）
- 讓 NVR 從輸入檔讀取路徑並儲存至 Table
- 取代預設的 BFS 路徑
- 停用路徑快取機制

### Task 2：封包傳送與路徑追蹤
- 使用 BulkSendHelper 與 PacketSinkHelper 傳送 TCP 封包
- 設定封包大小為 512 Byte，模擬時間起始為 0.0 秒
- 追蹤 Mac 層的 Tx/Rx 時間戳，確認封包是否依照指定路徑傳送

### Task 3：計算吞吐量
- 使用三組 SD pair 傳送封包：36→38、37→40、39→41
- 設定 MaxBytes 為 0，持續傳送封包
- 使用 GetTotalRx() 計算每組 SD pair 的接收位元組數，並輸出總吞吐量

## 模擬參數

- Routing 套件：NixVectorRouting
- 封包大小：512 Byte
- 傳輸協定：TCP
- 路徑輸入檔：paths1.in、paths2.in

## 實驗觀察摘要

- 若多條路徑共用相同節點，容易造成壅塞，導致吞吐量下降
- 使用 paths1.in 時，節點 35 頻繁被使用，造成壅塞；paths2.in 則無此問題
- 分開傳送各 SD pair 可有效減少壅塞，提升總吞吐量

## 問題與分析

- Q1：parentVector 描述路徑的方式為每個節點儲存其前驅節點，透過反向追蹤可重建完整路徑
- Q2：paths1.in 因節點重複使用造成壅塞，吞吐量低於 paths2.in
- Q3：分別傳送各 SD pair 可驗證壅塞情況，paths1.in 在同時傳送時吞吐量明顯下降
