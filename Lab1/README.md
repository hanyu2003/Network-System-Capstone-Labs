# Lab1：Analog Beamforming 模擬實驗

## 實驗簡介

本實驗旨在透過 MATLAB 撰寫程式，模擬 Analog Beamforming（類比波束成形）的運作方式，並分析在不同天線數量與 codebook 設定下，對接收功率（Rx Power）、訊號雜訊比（SNR）與訊號干擾雜訊比（SINR）的影響。

模擬場景中包含一個發射端（Tx）與兩個接收端（Rx），透過 beam scanning 技術選擇最佳波束方向，以達到最大化的 Tx gain。實驗分為三個主要任務：

## 實驗任務

### Task 1：模擬 Beam Scanning

- 計算兩位使用者的實際角度（AoD）
- 從 codebook 中選出最接近的 beam 方向（最大 Tx gain）

### Task 2：計算 Rx Power 與 SNR

- 根據 optimal beam 計算 Tx gain
- 使用 Friis 傳輸公式計算 Rx power
- 根據接收功率與固定的 noise power 計算 SNR

### Task 3：計算 SINR（兩條 beam 同時存在）

- 模擬兩個 beam 同時發射的情境
- 計算干擾功率與 SINR，分析波束間的干擾影響

## 模擬參數

- Carrier Frequency：24 GHz  
- Tx Power：20 dBm  
- Noise Power (N₀)：-88 dBm  
- Tx 天線數量：4、8、16  
- Rx 天線數量：1  
- Codebook：[0:5:180]、[0:10:180]、[0:15:180]  
- Beam Resolution：180 度範圍切成 360 個角度（每 0.5 度）

## 實驗觀察摘要

- 天線數量越多 → Tx gain 越集中 → Rx power 與 SNR 提升明顯  
- Codebook 越精細 → beam 對準越準 → SNR 有小幅提升  
- SINR 分析：干擾功率與 beam 對準程度有關，天線數量與 codebook 精度皆影響 SINR 表現

## 出處說明

本專案原屬於 NYCU-NETCAP2025 課程帳號（https://github.com/NYCU-NETCAP2025），原由助教建立，因原始倉庫未公開，故另建此專案。
