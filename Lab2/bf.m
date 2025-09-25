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

% fprintf('Rx1 location: (%.2f, %.2f)\n', rx_location(1,1), rx_location(1,2));
% fprintf('Rx2 location: (%.2f, %.2f)\n\n', rx_location(2,1), rx_location(2,2));

resolution = 360;           % Number of angles dividing 180 degrees
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TA Section End %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



phi_degree = 0.5 : 0.5 : 180;
phi_rad = phi_degree * pi / 180;

% task %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 設置參數
tx_antenna_number = 16;
tx_beam_direction = 0:5:90;


data = load('position.txt'); 
% GS 座標
x1 = data(1,1); 
y1 = data(1,2); 
z1 = data(1,3);
% SAT 座標
x2 = data(2,1); 
y2 = data(2,2); 
z2 = data(2,3); 

% 調用函數
[gain_table_1, pathloss, Rx1_power, Rx1_SNR, theta1_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, x1, y1, z1, x2, y2, z2);

% 輸出結果
fprintf('Task1: Calculate AoD\n');
fprintf('Rx1 Actual Angle: %f degree\n', theta1_degree);
fprintf('Task2: Calculate Rx power and SNR\n');
fprintf('Rx1 pathloss: %f dBm\n', pathloss);
fprintf('Rx1 power: %f dBm\n', Rx1_power);
fprintf('Rx1 SNR: %f dB\n', Rx1_SNR);
fprintf('Task3: Calculate SINR for two concurrent beams\n');
figure;

plot(phi_degree, gain_table_1);
xlabel('Angle (degrees)');
ylabel('Tx gain');
title('Figure 1: Cartesian plot for user 1');
grid 

figure;
polarplot(phi_rad, gain_table_1);
title('Figure 2: Polar plot for user 1');

fileID = fopen('pathloss.txt', 'w'); % 打開文件以寫入
fprintf(fileID, '%f\n', pathloss); % 寫入字符串
fclose(fileID); % 關閉文件



function [gain_table_1, pathloss, Rx1_power, Rx1_SNR, theta1_degree] = beamforming_simulation(tx_antenna_number, tx_beam_direction, x1, y1, z1, x2, y2, z2)

    % 環境配置
    freq = 24e9;
    d = 0.5; % 天線間距（波長的倍數）
    P_tx_dBm = 105.9; % 發射功率（dBm）
    N0_dBm = -110; % 噪聲功率（dBm）

    %%%%%%%%%%%%%%%%%%%%%%task1
    % 計算 User 1 的角度

    dis_xy = sqrt((x2 - x1)^2 + (y2 - y1)^2);
    distance = sqrt((dis_xy)^2 + (z2 - z1)^2);
    fprintf("Q1: Euclidean distance: %.6f m\n", distance);
    
    % 計算仰角 θ
    theta1_degree = abs( atan2(dis_xy, z2 - z1) * (180 / pi) ); % 轉換成角度
    if theta1_degree > 90
        theta1_degree = 180-theta1_degree;
    end

    % 找到最接近 theta1 的波束方向
    [~, index1] = min(abs(tx_beam_direction - theta1_degree));
    op_beam1 = tx_beam_direction(index1);


    % 計算增益
    phi_degree = 0.5 : 0.5 : 180;
    phi_rad = phi_degree * pi / 180;
    psi = 2 * pi * d * sin(phi_rad);

    a1 = uniform(d, op_beam1, tx_antenna_number);
    A1 = dtft(a1, -psi);
    gain_table_1 = abs(A1).^2;



    %%%%%%%%%%%%%%%%%%%%%%task2
    % 計算 Rx1 的接收功率和 SNR
    rx1_sector_index = round(theta1_degree / 0.5);
    Tx_gain_1 = 1;
    % Tx_gain_1 = gain_table_1(rx1_sector_index);
    pathloss = -friis_equation(freq, Tx_gain_1, 1, distance);
    Rx1_power = P_tx_dBm - pathloss;
    Rx1_SNR = Rx1_power - N0_dBm;

  

end
