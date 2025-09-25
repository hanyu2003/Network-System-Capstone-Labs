close all; clear; clc;
addpath ./ewa_function;

function generate_network_graph(filename)
    % 讀取 network.pos
    fileID = fopen(filename, 'r');

    % 讀取第一行 (tx power, noise, frequency, bandwidth, rx power threshold)
    header = fscanf(fileID, '%f %f %f %f %f', [5 1])';
    P_tx_dBm = header(1);
    N0_dBm = header(2);
    freq = header(3);
    bandwidth = header(4);
    P_rx_thr_dBm = header(5);

    % 讀取第二行 (#ground station, #satellite)
    numStations = fscanf(fileID, '%d %d', [2 1])';
    numGroundStations = numStations(1);
    numSatellites = numStations(2);

    % 讀取 ground station 座標
    gs_data = zeros(numGroundStations, 4);
    for i = 1:numGroundStations
        gs_data(i, :) = fscanf(fileID, '%d %f %f %f', [4 1])';
    end

    % 讀取 satellite 座標
    sat_data = zeros(numSatellites, 4);
    for i = 1:numSatellites
        sat_data(i, :) = fscanf(fileID, '%d %f %f %f', [4 1])';
    end

    fclose(fileID);

    % 計算每個 Ground Station 到 Satellite 的數據傳輸率
    links = [];
    for i = 1:numGroundStations
        for j = 1:numSatellites
            % 計算歐幾里得距離
            distance = sqrt(sum((gs_data(i, 2:4) - sat_data(j, 2:4)).^2));

            % 計算仰角 θ
            dis_xy = sqrt((gs_data(i, 2) - sat_data(j, 2))^2 + (gs_data(i, 3) - sat_data(j, 3))^2);
            theta_degree = abs(atan2(dis_xy, sat_data(j, 4) - gs_data(i, 4)) * (180 / pi));
            if theta_degree > 90
                theta_degree = 180 - theta_degree;
            end

            % 波束方向對準
            tx_beam_direction = 0:5:90;
            [~, index1] = min(abs(tx_beam_direction - theta_degree));
            op_beam = tx_beam_direction(index1);

            % 計算 beamforming gain
            d = 0.5;
            tx_antenna_number = 16;
            phi_degree = 0.5 : 0.5 : 180;
            phi_rad = phi_degree * pi / 180;
            psi = 2 * pi * d * sin(phi_rad);
            a1 = uniform(d, op_beam, tx_antenna_number);
            A1 = dtft(a1, -psi);
            gain_table = abs(A1).^2;
            rx_sector_index = round(theta_degree / 0.5);
            Tx_gain = gain_table(rx_sector_index);

            % 計算 Path Loss (Friis 公式)
            path_loss_dB = friis_equation(freq, Tx_gain, 1, distance);

            % 計算接收功率
            Rx_power_dBm = P_tx_dBm + path_loss_dB;

            % 若 Rx Power >= 門檻，則計算 Data Rate
            if Rx_power_dBm >= P_rx_thr_dBm
                Rx_power_W = 10^(Rx_power_dBm / 10) / 1000;
                N0_W = 10^(N0_dBm / 10) / 1000;
                SNR = Rx_power_W / N0_W;
                data_rate_kbps = bandwidth * log2(1 + SNR)/1000   ;
                links = [links; gs_data(i,1), sat_data(j,1), data_rate_kbps];
            end
        end
    end

    % 輸出 network.graph
    fileID = fopen('network.graph', 'w');
    fprintf(fileID, '%d %d %d\n\n', numGroundStations, numSatellites, size(links, 1));
    for k = 1:size(links, 1)
        fprintf(fileID, '%d %d %f\n', links(k, 1), links(k, 2), links(k, 3));
    end
    fclose(fileID);
    disp('network.graph 已成功生成！');
end

% 執行程式
generate_network_graph('network.pos');
