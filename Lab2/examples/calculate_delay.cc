/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"

using namespace ns3;

map<uint32_t, double> delay;

// static void EchoTxRx (std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket)
// {
//     // TODO: Calculate end-to-end delay
//     // Hint1: Parse the packet (you may refer context.find())
//     // Hint2: Store send/arrival time for the same sequence number
//     std::cout << Simulator::Now () << ":" << context << ":" << packet->GetUid() << ":" << socket->GetNode () << ":" << header.GetSequenceNumber () << std::endl;
//     // Hint3: Calculate end-to-end delay
//     delay[1] = 2.5; // This value is for reference only
// }

static std::map<uint32_t, Time> sendTimes;
static std::map<uint32_t, Time> arrivalTimes;

static void EchoTxRx (std::string context, const Ptr<const Packet> packet, const TcpHeader &header, const Ptr<const TcpSocketBase> socket)
{
    uint32_t seq = header.GetSequenceNumber().GetValue();
    Time now = Simulator::Now();
    
    // 解析發送時間與到達時間
    if (context.find("Tx") != std::string::npos) {
        if (sendTimes.find(seq) == sendTimes.end()) { // 第一次出現這個序列號
            sendTimes[seq] = now;
        }
    } else if (context.find("Rx") != std::string::npos) {
        arrivalTimes[seq] = now;

        // 計算 end-to-end 延遲
        if (sendTimes.find(seq) != sendTimes.end()) {  //sendTimes有紀錄了
            Time delayTime = arrivalTimes[seq] - sendTimes[seq];
            delay[seq] = delayTime.GetSeconds();
            cout << "seq: " << seq << "   n2n delay: " << delay[seq] << std::endl;
        }
    }


    // std::cout << now << " context:" << context << " uid:" << packet->GetUid() << " node:" 
    //           << socket->GetNode () << " seq:" << seq << " size:" << packet->GetSize() << std::endl;

    // std::cout << header << std::endl << std::endl;
}

void connect ()
{
    Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Tx", MakeCallback (&EchoTxRx));
    Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx", MakeCallback (&EchoTxRx));
}

void OutputSatellitePosition() {
    std::ofstream outputFile("contrib/leo/examples/satellite_positions_all.txt", std::ios::out);

    if (!outputFile.is_open()) {
        NS_LOG_UNCOND("Error opening file!");
        return;
    }

    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i) {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
        if (mobility) {
            Vector pos = mobility->GetPosition();
            
            // Convert position to latitude & longitude
            double r = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
            double lat = asin(pos.z / r) * 180.0 / M_PI;
            double lon = atan2(pos.y, pos.x) * 180.0 / M_PI;

            // Output (x, y, z) and latitude, longitude
            outputFile << "Node " << i << ": " << pos.x << " " << pos.y << " " << pos.z 
                       << " | Latitude: " << lat << " Longitude: " << lon << std::endl;
        }
    }

    outputFile.close();
    NS_LOG_UNCOND("Satellite positions written to satellite_positions_all.txt");
}

double OutputSatellitePosition_i(int i) {
    std::ofstream outputFile("contrib/leo/examples/satellite_positions_i.txt", std::ios::out);

    if (!outputFile.is_open()) {
        NS_LOG_UNCOND("Error opening file!");
        return -1;
    }

    //地面站的位置
    Ptr<Node> node = NodeList::GetNode(25);
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
    Vector pos_GA = mobility->GetPosition();
    // Output (x, y, z) and latitude, longitude
    outputFile << std::to_string(pos_GA.x) << " "
           << std::to_string(pos_GA.y) << " "
           << std::to_string(pos_GA.z) << std::endl;

    //衛星的位置
    node = NodeList::GetNode(i);
    mobility = node->GetObject<MobilityModel>();
    Vector pos_SAT = mobility->GetPosition();
    // Output (x, y, z) and latitude, longitude
    outputFile << std::to_string(pos_SAT.x) << " "
            << std::to_string(pos_SAT.y) << " "
            << std::to_string(pos_SAT.z) << std::endl;
   
    double distance = sqrt(pow(pos_GA.x - pos_SAT.x, 2) + 
                       pow(pos_GA.y - pos_SAT.y, 2) + 
                       pow(pos_GA.z - pos_SAT.z, 2));
    
    outputFile.close();
    NS_LOG_UNCOND("Satellite positions written to satellite_positions_i.txt");

    return distance;
}


void initial_position (const NodeContainer &satellites, int sz)
{
    for(int i = 0; i < min((int)satellites.GetN(), sz); i++){
        // Get satellite position
        Vector pos = satellites.Get(i)->GetObject<MobilityModel>()->GetPosition();
        // Convert position to latitude & longtitude
        double r = sqrt(pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
        double lat = asin(pos.z / r) * 180.0 / M_PI;
        double longit = atan2(pos.y, pos.x) * 180 / M_PI;
        cout << "Satellite " << i << " latitude = " << lat << ", longtitude = " << longit << endl;
    }
}

NS_LOG_COMPONENT_DEFINE ("LeoBulkSendTracingExample");

int main (int argc, char *argv[])
{

    int SATindex = 2;

    CommandLine cmd;
    std::string orbitFile;
    std::string traceFile;
    // LeoLatLong source (20, 0);
    // LeoLatLong destination (20, 0);
    // LeoLatLong source (6.06692, 73.0213);
    // LeoLatLong destination (6.06692, 73.0213);
    LeoLatLong source (-16.0634, 142.29);
    LeoLatLong destination (-16.0634, 142.29);
    std::string islRate = "2Gbps";
    std::string constellation = "TelesatGateway";
    uint16_t port = 9;
    uint32_t latGws = 20;
    uint32_t lonGws = 20;
    double duration = 100;
    bool islEnabled = false;
    bool pcap = false;
    uint64_t ttlThresh = 0;
    std::string routingProto = "aodv";

    cmd.AddValue("orbitFile", "CSV file with orbit parameters", orbitFile);
    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("precision", "ns3::LeoCircularOrbitMobilityModel::Precision");
    cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
    cmd.AddValue("source", "Traffic source", source);
    cmd.AddValue("destination", "Traffic destination", destination);
    cmd.AddValue("islRate", "ns3::MockNetDevice::DataRate");
    cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
    cmd.AddValue("routing", "Routing protocol", routingProto);
    cmd.AddValue("islEnabled", "Enable inter-satellite links", islEnabled);
    cmd.AddValue("latGws", "Latitudal rows of gateways", latGws);
    cmd.AddValue("lonGws", "Longitudinal rows of gateways", lonGws);
    cmd.AddValue("ttlThresh", "TTL threshold", ttlThresh);
    cmd.AddValue("destOnly", "ns3::aodv::RoutingProtocol::DestinationOnly");
    cmd.AddValue("routeTimeout", "ns3::aodv::RoutingProtocol::ActiveRouteTimeout");
    cmd.AddValue("pcap", "Enable packet capture", pcap);
    cmd.Parse (argc, argv);

    std::streambuf *coutbuf = std::cout.rdbuf();
    // redirect cout if traceFile
    std::ofstream out;
    out.open (traceFile);
    if (out.is_open ())
    {
        std::cout.rdbuf(out.rdbuf());
    }

    LeoOrbitNodeHelper orbit;
    NodeContainer satellites;
    if (!orbitFile.empty())
    {
        satellites = orbit.Install (orbitFile);
    }
    else
    {
        satellites = orbit.Install ({ LeoOrbit (1200, 20, 5, 5) });
    }

    LeoGndNodeHelper ground;
    NodeContainer users = ground.Install (source, destination);

    LeoChannelHelper utCh;
    utCh.SetConstellation (constellation);

    // utCh.SetGndDeviceAttribute("DataRate", StringValue("8kbps"));
    NetDeviceContainer utNet = utCh.Install (satellites, users);
    ////////////////////////////////////////////////////////////////////////
    double bandwidth_MHz = 2;
    double noise_dbm = -110;
    double tx_power_dbm = 105.9;
    Ptr<NetDevice> dev = utNet.Get(0);
    Ptr<LeoMockNetDevice> mockDev = DynamicCast<LeoMockNetDevice>(dev);
    Ptr<LeoMockChannel> channel = DynamicCast<LeoMockChannel>(mockDev->GetChannel());
    Ptr<LeoPropagationLossModel> lossModel = DynamicCast<LeoPropagationLossModel>(channel->GetPropagationLoss());
    Ptr<MobilityModel> txMobility = users.Get(0)->GetObject<MobilityModel>();
    Ptr<MobilityModel> rxMobility = satellites.Get(SATindex)->GetObject<MobilityModel>();

    double rx_power_dbm = lossModel->DoCalcRxPower(tx_power_dbm, txMobility, rxMobility);
    double SNR_dB = rx_power_dbm - (noise_dbm);
    double SNR_ratio = pow(10, SNR_dB / 10);
    double Shannon_Capacity_Mbps = bandwidth_MHz * log2(1 + SNR_ratio);

    std::ifstream infile("contrib/leo/examples/pathloss.txt", std::ios::out);
    double pathloss;
    
    if (infile.is_open())
    {
        infile >> pathloss;
        infile.close();
    }

    ////////////////////////////////////////////////////////////////////////
    std::ostringstream oss;
    oss << Shannon_Capacity_Mbps << "Mbps";
    utNet.Get(25)->GetObject<MockNetDevice>()->SetDataRate(DataRate(oss.str()));
    utNet.Get(SATindex)->GetObject<MockNetDevice>()->SetDataRate(DataRate("1Gbps"));
    ////////////////////////////////////////////////////////////////////////

    initial_position(satellites, 5);
    double distance = OutputSatellitePosition_i(SATindex);

    InternetStackHelper stack;
    AodvHelper aodv;
    aodv.Set ("EnableHello", BooleanValue (false));
    //aodv.Set ("HelloInterval", TimeValue (Seconds (10)));
    if (ttlThresh != 0)
    {
        aodv.Set ("TtlThreshold", UintegerValue (ttlThresh));
        aodv.Set ("NetDiameter", UintegerValue (2*ttlThresh));
    }
    stack.SetRoutingHelper (aodv);

    // Install internet stack on nodes
    stack.Install (satellites);
    stack.Install (users);

    Ipv4AddressHelper ipv4;

    ipv4.SetBase ("10.1.0.0", "255.255.0.0");
    ipv4.Assign (utNet);

    if (islEnabled)
    {
        std::cerr << "ISL enabled" << std::endl;
        IslHelper islCh;
        NetDeviceContainer islNet = islCh.Install (satellites);
        ipv4.SetBase ("10.2.0.0", "255.255.0.0");
        ipv4.Assign (islNet);
    }

    Ipv4Address remote = users.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
    // Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
    BulkSendHelper sender ("ns3::TcpSocketFactory",
            InetSocketAddress (remote, port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    sender.SetAttribute ("MaxBytes", UintegerValue (1024));
    sender.SetAttribute ("SendSize", UintegerValue (512));
    ApplicationContainer sourceApps = sender.Install (users.Get (0));
    sourceApps.Start (Seconds (0.0));

    //
    // Create a PacketSinkApplication and install it on node 1
    //
    PacketSinkHelper sink ("ns3::TcpSocketFactory",
            InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sink.Install (users.Get (1));
    sinkApps.Start (Seconds (0.0));

    // Fix segmentation fault
    Simulator::Schedule(Seconds(1e-7), &connect);

    //
    // Set up tracing if enabled
    //
    if (pcap)
    {
        AsciiTraceHelper ascii;
        utCh.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
        utCh.EnablePcapAll ("tcp-bulk-send", false);
    }

    std::cerr << "LOCAL =" << users.Get (0)->GetId () << std::endl;
    std::cerr << "REMOTE=" << users.Get (1)->GetId () << ",addr=" << Ipv4Address::ConvertFrom (remote) << std::endl;

    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
    std::cout << users.Get (0)->GetId () << ":" << users.Get (1)->GetId () << ": " << sink1->GetTotalRx () << std::endl;

    // TODO: Output End-to-end Delay
    // double avg_delay = 0;
    // for(auto &[seq, t]: delay){
    //     avg_delay += delay[seq];
    //     cout << "Packet average end-to-end delay is " << avg_delay << "s" << endl;
    // }

    // 計算並輸出平均 end-to-end 延遲
    double avg_delay = 0;
    if (!delay.empty()) {
        for (const auto& [seq_uid, t] : delay) {
            avg_delay += t;
        }
        avg_delay /= delay.size();
    }
    printf("\nSAT index: %d\n", SATindex);
    printf("Q1: Euclidean distance: %.6f m\n", distance);
    printf("Q2: pathloss: %.6f dBm\n", pathloss);
    printf("Q3: RxPower: %.6f dBm\n", rx_power_dbm);
    printf("Q4: SNR: %.6f dB\n", SNR_dB);
    printf("Q4: datarate: %.6f Mbps\n", Shannon_Capacity_Mbps);    
    std::cout << "Packet average end-to-end delay is " << avg_delay << "s" << std::endl;


    out.close ();
    std::cout.rdbuf(coutbuf);

    return 0;
}
