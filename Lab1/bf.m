%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DO NOT MODIFY THIS SECTION. Any changes will result in a penalty.
close all; clear; clc;
addpath ./ewa_function;
rng(77);

% Environment Configurations
freq = 24e9;
tx_node_number = 1;      % Number of Tx users
tx_antenna_number = 16;  % Number of Tx antennas
rx_node_number = 2;      % Number of Rx users
rx_antenna_number = 1;   % Number of Rx antennas
tx_beam_direction = 0:10:180; % degree
d = 0.5;                 % Distance between antennas (multiple of wavelength)
P_tx_dBm = 20;           % Transmission power of Tx (dBm)
N0_dBm = -88;            % Noise power (dBm)
tx_location = [0, 0];    % Tx location

% Random Rx location
rx_location = zeros(rx_node_number, 2);
for i = 1:rx_node_number
    r = 5 + 20 * rand();    % Random distance between 5 and 25 meters (m)
    angle = 180 * rand();
    x = r * cosd(angle);    % Beam direction with a small random offset
    y = r * sind(angle);    % Beam direction with a small random offset
    rx_location(i, :) = [x, y];
end

fprintf('Rx1 location: (%.2f, %.2f)\n', rx_location(1,1), rx_location(1,2));
fprintf('Rx2 location: (%.2f, %.2f)\n\n', rx_location(2,1), rx_location(2,2));

resolution = 360;           % Number of angles dividing 180 degrees
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



phi_degree = [0.5 : 0.5 : 180];
phi_rad = phi_degree * pi / 180;

% task %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 設置參數
tx_antenna_number = 16;
tx_beam_direction = 0:10:180;
rx_location_user1 = [rx_location(1,1), rx_location(1,2)]; % User 1 的位置
rx_location_user2 = [rx_location(2,1), rx_location(2,2)]; % User 2 的位置

% 調用函數
[gain_table_1, gain_table_2, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2);

% 輸出結果
fprintf('Task1: Calculate AoD\n');
fprintf('Rx1 Actual Angle: %f degree\n', theta1_degree);
fprintf('Rx2 Actual Angle: %f degree\n', theta2_degree);
fprintf('Task2: Calculate Rx power and SNR\n');
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n', Rx1_SNR);
fprintf('Task3: Calculate SINR for two concurrent beams\n');
fprintf('Rx1 interference power: %f dBm\n', Rx1_interference_power);
fprintf('Rx1 SINR: %f dB\n', Rx1_SINR);
figure;

plot(phi_degree, gain_table_1);
xlabel('Angle (degrees)');
ylabel('Tx gain');
title('Figure 1: Cartesian plot for user 1');
grid 

figure;
plot(phi_degree, gain_table_2);
xlabel('Angle (degrees)');
ylabel('Tx gain');
title('Figure 2: Cartesian plot for user 2');
grid 

figure;
polarplot(phi_rad, gain_table_1);
title('Figure 3: Polar plot for user 1');

figure;
polarplot(phi_rad, gain_table_2);
title('Figure 4: Polar plot for user 2');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fprintf('\nReport\n\n');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tx_antenna_number = 4;
fprintf('--- Tx Antennas: 4 ---\n\n');
tx_beam_direction = 0:5:180;
fprintf('--- Codebook size: 5 ---\n');

% 調用函數
[gain_table_11, gain_table_21, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2);

% 輸出結果
fprintf('Calculate Rx1 power and SNR\n');
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n\n', Rx1_SNR);
fprintf('Calculate SINR of two concurrent beams\n');
fprintf('Rx1 interference power: %f dBm\n', Rx1_interference_power);
fprintf('Rx1 SINR: %f dB\n\n', Rx1_SINR);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tx_beam_direction = 0:15:180;
fprintf('--- Codebook size: 15 ---\n');

% 調用函數
[gain_table_12, gain_table_22, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2);

% 輸出結果
fprintf('Calculate Rx1 power and SNR\n');
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n\n', Rx1_SNR);
fprintf('Calculate SINR of two concurrent beams\n');
fprintf('Rx1 interference power: %f dBm\n', Rx1_interference_power);
fprintf('Rx1 SINR: %f dB\n\n', Rx1_SINR);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tx_antenna_number = 8;
fprintf('--- Tx Antennas: 8 ---\n\n');
tx_beam_direction = 0:5:180;
fprintf('--- Codebook size: 5 ---\n');

% 調用函數
[gain_table_13, gain_table_23, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2);

% 輸出結果
fprintf('Calculate Rx1 power and SNR\n');
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n\n', Rx1_SNR);
fprintf('Calculate SINR of two concurrent beams\n');
fprintf('Rx1 interference power: %f dBm\n', Rx1_interference_power);
fprintf('Rx1 SINR: %f dB\n\n', Rx1_SINR);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tx_beam_direction = 0:15:180;
fprintf('--- Codebook size: 15 ---\n');

% 調用函數
[gain_table_14, gain_table_24, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2);

% 輸出結果
fprintf('Calculate Rx1 power and SNR\n');
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n\n', Rx1_SNR);
fprintf('Calculate SINR of two concurrent beams\n');
fprintf('Rx1 interference power: %f dBm\n', Rx1_interference_power);
fprintf('Rx1 SINR: %f dB\n\n', Rx1_SINR);



figure;
hold on;
plot(phi_degree, gain_table_11, 'r-', 'DisplayName', '4 Antennas, [0:5:180]');
plot(phi_degree, gain_table_12, 'g-', 'DisplayName', '4 Antennas, [0:15:180]');
plot(phi_degree, gain_table_13, 'b-', 'DisplayName', '8 Antennas, [0:5:180]');
plot(phi_degree, gain_table_14, 'm-', 'DisplayName', '8 Antennas, [0:15:180]');
xlabel('Angle (degrees)');
ylabel('Tx gain');
title('Cartesian Plot for User 1');
legend;
grid on;
hold off;

figure;
hold on;
plot(phi_degree, gain_table_21, 'r-', 'DisplayName', '4 Antennas, [0:5:180]');
plot(phi_degree, gain_table_22, 'g-', 'DisplayName', '4 Antennas, [0:15:180]');
plot(phi_degree, gain_table_23, 'b-', 'DisplayName', '8 Antennas, [0:5:180]');
plot(phi_degree, gain_table_24, 'm-', 'DisplayName', '8 Antennas, [0:15:180]');
xlabel('Angle (degrees)');
ylabel('Tx gain');
title('Cartesian Plot for User 2');
legend;
grid on;
hold off;

figure;
polarplot(phi_rad, gain_table_11, 'r-', 'DisplayName', '4 Antennas, [0:5:180]');
hold on;
polarplot(phi_rad, gain_table_12, 'g-', 'DisplayName', '4 Antennas, [0:15:180]');
polarplot(phi_rad, gain_table_13, 'b-', 'DisplayName', '8 Antennas, [0:5:180]');
polarplot(phi_rad, gain_table_14, 'm-', 'DisplayName', '8 Antennas, [0:15:180]');
title('Polar Plot for User 1');
legend;
hold off;

figure;
polarplot(phi_rad, gain_table_21, 'r-', 'DisplayName', '4 Antennas, [0:5:180]');
hold on;
polarplot(phi_rad, gain_table_22, 'g-', 'DisplayName', '4 Antennas, [0:15:180]');
polarplot(phi_rad, gain_table_23, 'b-', 'DisplayName', '8 Antennas, [0:5:180]');
polarplot(phi_rad, gain_table_24, 'm-', 'DisplayName', '8 Antennas, [0:15:180]');
title('Polar Plot for User 2');
legend;
hold off;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% random 20 times
% 設置參數
tx_antenna_number = 16;
tx_beam_direction = 0:10:180;
rx_node_number = 2; % Number of Rx users

% 初始化變數
total_Rx1_power = 0;
total_Rx1_SNR = 0;
total_Rx1_interference_power = 0;
total_Rx1_SINR = 0;

% 進行 20 次隨機運行
for run = 1:20
    % 隨機生成 Rx 位置
    rx_location = zeros(rx_node_number, 2);
    for i = 1:rx_node_number
        r = 5 + 20 * rand();    % Random distance between 5 and 25 meters (m)
        angle = 180 * rand();
        x = r * cosd(angle);    % Beam direction with a small random offset
        y = r * sind(angle);    % Beam direction with a small random offset
        rx_location(i, :) = [x, y];
    end

    % 調用 beamforming_simulation 函數
    [gain_table_1, gain_table_2, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location(1, :), rx_location(2, :));

    % 累加結果
    total_Rx1_power = total_Rx1_power + Rx1_power;
    total_Rx1_SNR = total_Rx1_SNR + Rx1_SNR;
    total_Rx1_interference_power = total_Rx1_interference_power + Rx1_interference_power;
    total_Rx1_SINR = total_Rx1_SINR + Rx1_SINR;
end

% 計算平均值
avg_Rx1_power = total_Rx1_power / 20;
avg_Rx1_SNR = total_Rx1_SNR / 20;
avg_Rx1_interference_power = total_Rx1_interference_power / 20;
avg_Rx1_SINR = total_Rx1_SINR / 20;

% 輸出結果
fprintf('Average Rx1 power: %f dBm\n', avg_Rx1_power);
fprintf('Average Rx1 SNR: %f dB\n', avg_Rx1_SNR);
fprintf('Average Rx1 interference power: %f dBm\n', avg_Rx1_interference_power);
fprintf('Average Rx1 SINR: %f dB\n', avg_Rx1_SINR);







function [gain_table_1, gain_table_2, Rx1_power, Rx1_SNR, Rx1_interference_power, Rx1_SINR, theta1_degree, theta2_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, rx_location_user1, rx_location_user2)

    % 環境配置
    freq = 24e9;
    d = 0.5; % 天線間距（波長的倍數）
    P_tx_dBm = 20; % 發射功率（dBm）
    N0_dBm = -88; % 噪聲功率（dBm）

    %%%%%%%%%%%%%%%%%%%%%%task1
    % 計算 User 1 的角度
    x1 = rx_location_user1(1);
    y1 = rx_location_user1(2);
    theta1_degree = atan2d(y1, x1); % 計算角度
    if theta1_degree < 0 % 確保角度在 0 到 360 度
        theta1_degree = theta1_degree + 360;
    end
    
    % 計算 User 2 的角度
    x2 = rx_location_user2(1);
    y2 = rx_location_user2(2);
    theta2_degree = atan2d(y2, x2); % 計算角度
    if theta2_degree < 0 % 確保角度在 0 到 360 度
        theta2_degree = theta2_degree + 360;
    end

    % 找到最接近 theta1 和 theta2 的波束方向
    [~, index1] = min(abs(tx_beam_direction - theta1_degree));
    op_beam1 = tx_beam_direction(index1);
    [~, index2] = min(abs(tx_beam_direction - theta2_degree));
    op_beam2 = tx_beam_direction(index2);

    % 計算增益
    phi_degree = [0.5 : 0.5 : 180];
    phi_rad = phi_degree * pi / 180;
    psi = 2 * pi * d * cos(phi_rad);

    a1 = uniform(d, op_beam1, tx_antenna_number);
    % a1 = uniform(d, op_beam1, tx_antenna_number) / tx_antenna_number;
    A1 = dtft(a1, -psi);
    gain_table_1 = abs(A1).^2;

    a2 = uniform(d, op_beam2, tx_antenna_number);
    % a2 = uniform(d, op_beam2, tx_antenna_number) / tx_antenna_number;
    A2 = dtft(a2, -psi);
    gain_table_2 = abs(A2).^2;

    %%%%%%%%%%%%%%%%%%%%%%task2
    % 計算 Rx1 的接收功率和 SNR
    rx1_sector_index = round(theta1_degree / 0.5);
    Tx_gain_1 = gain_table_1(rx1_sector_index);
    Rx1_power = P_tx_dBm + friis_equation(freq, Tx_gain_1, 1, sqrt(x1^2 + y1^2));
    Rx1_SNR = Rx1_power - N0_dBm;

    %%%%%%%%%%%%%%%%%%%%%%task3  to two user
    P_tx_dBm_half = 10 * log10(10 ^(P_tx_dBm/10)/2);

    % 計算 Rx1 的接收功率
    Rx1_power_2 = P_tx_dBm_half + friis_equation(freq, Tx_gain_1, 1, sqrt(x1^2 + y1^2));

    % 計算干擾功率和 SINR
    Tx_gain_interference = gain_table_2(rx1_sector_index);
    Rx1_interference_power = P_tx_dBm_half + friis_equation(freq, Tx_gain_interference, 1, sqrt(x1^2 + y1^2));
    Rx1_SINR = Rx1_power_2 - 10*log10(10^((Rx1_interference_power)/10) + 10^((N0_dBm)/10));


end
