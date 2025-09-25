#include <iostream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nix-vector-helper.h"

using namespace ns3;
using namespace std;

// Satellite network setup
uint16_t port = 9;
NodeContainer satellites;
NodeContainer groundStations;
NetDeviceContainer utNet;
NetDeviceContainer islNet;
int Task = 2;


static void EchoMacTxRx(std::string context, const Ptr< const Packet > packet);
/////////////////////////////////////////////////////////////
// void SendPacket(Ptr<Node> src, Ptr<Node> dst);
Ptr<PacketSink> SendPacket (int srcId, int dstId);
/////////////////////////////////////////////////////////////
void connect();
LeoLatLong Vec2LatLong(Vector pos);
LeoLatLong id2LatLong(const NodeContainer &satellites, int id);

static void EchoMacTxRx(std::string context, const Ptr< const Packet > packet) {
    // Task 2.3: Complete this function
    // Hint: you can extract the header of the packet to check whether this packet is TCP data payload
    
    // 1) 先用大小過濾，ACK／控制封包通常 <100 bytes
    uint32_t pktSize = packet->GetSize ();
    if (pktSize <= 100)
        {
        return;
        }

    // 2) 由 context 解析 nodeId
    int nodeId = -1;
    {
        std::istringstream iss (context);
        std::string token;
        while (std::getline (iss, token, '/'))
        {
            if (token == "NodeList" && std::getline (iss, token, '/'))
            {
                nodeId = std::stoi (token);
                break;
            }
        }
    }

    // 3) 決定是 MacTx 還是 MacRx
    std::string ev = (context.find ("MacTx") != std::string::npos
                        ? "MacTx" : "MacRx");

    // 4) 印出
    std::cout << ev
                << " at node: " << nodeId
                << ", now: "  << Simulator::Now ()
                << std::endl;

} 


Ptr<PacketSink> SendPacket(int srcId, int dstId) {
    // Task 2.1: Complete this function
    // Task 2.1: Set MaxBytes to 512 & Task 3.1: Set MaxBytes to 0
    // 找出 src, dst
    Ptr<Node> src = NodeList::GetNode (srcId);
    Ptr<Node> dst = NodeList::GetNode (dstId);

    // 在 dst 安裝 PacketSink
    PacketSinkHelper sinkHelper (
        "ns3::TcpSocketFactory",
        InetSocketAddress (Ipv4Address::GetAny (), port));
    ApplicationContainer sinkApps = sinkHelper.Install (dst);
    sinkApps.Start (Seconds (0.0));
    // 取出 Ptr<PacketSink>
    Ptr<PacketSink> sink = DynamicCast<PacketSink> (sinkApps.Get (0));

    // 取 dst IP
    Ptr<Ipv4> ipv4 = dst->GetObject<Ipv4> ();
    Ipv4Address dstAddr = ipv4->GetAddress (1, 0).GetLocal ();

    // 在 src 安裝 BulkSend，MaxBytes 依 Task 而定
    BulkSendHelper sendHelper (
        "ns3::TcpSocketFactory",
        InetSocketAddress (dstAddr, port));
    sendHelper.SetAttribute ("MaxBytes",
        UintegerValue (Task == 3 ? 0 : 512)); // Task3 持續發
    sendHelper.SetAttribute ("SendSize", UintegerValue (512));
    ApplicationContainer sendApps = sendHelper.Install (src);
    sendApps.Start (Seconds (0.0));

    return sink;

}

void connect() {
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacTx", MakeCallback (&EchoMacTxRx));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::MockNetDevice/MacRx", MakeCallback (&EchoMacTxRx));
}

// Vec2LatLong function change xyz coordinate to latitude and longtitude
LeoLatLong Vec2LatLong(Vector pos){
    double r = sqrt (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
    double lat = asin (pos.z / r) * 180.0 / M_PI;
    double longit = atan2 (pos.y, pos.x) * 180 / M_PI;
    return LeoLatLong(lat, longit);
}

LeoLatLong id2LatLong(const NodeContainer &satellites, int id){
    Vector pos = satellites.Get(id)->GetObject<MobilityModel>()->GetPosition();
    return Vec2LatLong(pos);
}

NS_LOG_COMPONENT_DEFINE ("Lab6");

int main (int argc, char *argv[]) {
    CommandLine cmd;
    string constellation = "TelesatGateway";
    double duration = 100;
    string inputFile = "<Your Input File>";
    string outputFile;

    cmd.AddValue("in", "Input File", inputFile);
    cmd.AddValue("out", "Output File", outputFile);
    cmd.AddValue("Task", "2 or 3", Task);
    cmd.Parse (argc, argv);

    // Redirect cout if outputFile is specified
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::ofstream out;
    out.open (outputFile);
    if (out.is_open ()) {
        std::cout.rdbuf(out.rdbuf());
    }
    
    if(Task != 2 && Task != 3){
        cerr<<"Error: Task is not 2 or 3"<<endl;
        exit(1);
    }

    // Default setting
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(2.0)));

    // Satellite
    LeoOrbitNodeHelper orbit;
    satellites = orbit.Install ({ LeoOrbit (1200, 20, 6, 6) });

    // Ground station
    LeoGndNodeHelper ground;
    groundStations.Add(ground.Install(id2LatLong(satellites, 0), id2LatLong(satellites, 2)));  // node id=36, 37
    groundStations.Add(ground.Install(id2LatLong(satellites, 3), id2LatLong(satellites, 9)));  // node id=38, 39
    groundStations.Add(ground.Install(id2LatLong(satellites, 20), id2LatLong(satellites, 34))); // node id=40, 41

    // Set network
    LeoChannelHelper utCh;
    utCh.SetConstellation (constellation);
    utCh.SetGndDeviceAttribute("DataRate", StringValue("11kbps"));
    utNet = utCh.Install (satellites, groundStations);
    IslHelper islCh;
    islNet = islCh.Install (satellites);

    InternetStackHelper stack;
    // Set NVR as routing protocol 
    Ipv4NixVectorHelper nixRouting;
    nixRouting.SetPathFile(inputFile); // Implement the function
    stack.SetRoutingHelper(nixRouting);
    // Install internet stack on nodes
    stack.Install (satellites);
    stack.Install (groundStations);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.0.0", "255.255.0.0");
    ipv4.Assign (utNet);
    ipv4.SetBase ("10.2.0.0", "255.255.0.0");
    ipv4.Assign (islNet);

    // Task 2.2 & Task 3.2 : Call SendPacket()
    ////////////////////
    // NEW: Task2 只傳 36→38；Task3 傳三對 SD-pair
    Ptr<PacketSink> sink36_38, sink37_40, sink39_41;
    if (Task == 2)
        {
        sink36_38 = SendPacket (36, 38);
        }
    else
        {
        sink36_38 = SendPacket (36, 38);
        sink37_40 = SendPacket (37, 40);
        sink39_41 = SendPacket (39, 41);
        }
    ////////////////////


    if(Task == 2) {
        // trace the packet in task2
        Simulator::Schedule(Seconds(1e-9), &connect);
    }
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();
    Simulator::Destroy ();

    // Task 3.3 : Calculate throughput
    ////////////////////
    if (Task == 3)
        {
        uint64_t rx1 = sink36_38->GetTotalRx ();
        uint64_t rx2 = sink37_40->GetTotalRx ();
        uint64_t rx3 = sink39_41->GetTotalRx ();
        uint64_t total = rx1 + rx2 + rx3;
        std::cout << "36->38: " << rx1 << std::endl;
        std::cout << "37->40: " << rx2 << std::endl;
        std::cout << "39->41: " << rx3 << std::endl;
        std::cout << "Total throughput: " << total << std::endl;
        }
    ////////////////////

    out.close ();
    std::cout.rdbuf(coutbuf);

    return 0;
}