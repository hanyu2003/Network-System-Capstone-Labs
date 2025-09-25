# Lab2：NS3 模擬 LEO 衛星通訊中的 Beamforming

## 實驗簡介

本實驗延續 Lab1 的成果，結合 MATLAB 與 NS3 模擬 LEO（Low Earth Orbit）衛星通訊系統中的 Beamforming 技術。目標是透過 NS3 模擬衛星與地面站之間的通訊，並使用 Lab1 的波束成形結果來計算實際的接收功率（Rx Power）、訊號雜訊比（SNR）、資料傳輸速率（Data Rate）與端對端延遲（E2E Delay）。

## 實驗任務

### Task 1：拓撲設定與座標轉換

- 設定衛星與地面站的經緯度位置
- 將經緯度轉換為 ECEF 座標（x, y, z）
- 將座標輸出至 .txt 檔供 MATLAB 使用

### Task 2：計算 Tx Gain

- 使用 MATLAB 讀取座標並計算仰角（Elevation Angle）
- 根據仰角選擇最適波束方向（Optimal Beam）
- 計算 Tx Gain 並輸出至 .txt 檔供 NS3 使用

### Task 3：計算 Rx Power

- 修改 NS3 模組以讀取 Tx Gain 與 Pathloss
- 根據 Pathloss 計算接收功率（Rx Power）

### Task 4：計算 SNR 與資料速率

- 設定頻寬、噪聲與 Tx Power
- 使用 Shannon Capacity 計算資料速率

### Task 5：計算端對端延遲（E2E Delay）

- 根據資料速率更新連線設定
- 計算封包的平均傳輸延遲

## 模擬參數

- 頻率：24 GHz  
- Tx Power：105.9 dBm  
- Noise Power：-110 dBm  
- 頻寬：2 MHz  
- 波束方向 Codebook：[0:5:90]  
- 衛星位置：多組經緯度（如：(6.06692, 73.0213)）  
- 地面站位置：固定或與衛星相同經緯度但不同高度  

## 實驗觀察摘要

- 使用 Beamforming 可顯著提升 Rx Power 與 SNR
- 資料速率與延遲表現明顯優於未使用 Beamforming 的情況
- Tx Gain 與仰角的關係密切，選擇最佳波束方向至關重要

## 出處說明

本專案原屬於 NYCU-NETCAP2025 課程帳號（https://github.com/NYCU-NETCAP2025），原由助教建立，因原始倉庫未公開，故另建此專案。
