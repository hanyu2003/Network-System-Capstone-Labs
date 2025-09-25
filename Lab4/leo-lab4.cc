#include <iostream>
#include <algorithm>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"

using namespace ns3;

// Satellite network setup
int port = 9;
NodeContainer satellites;
NodeContainer groundStations;
NetDeviceContainer utNet;
ApplicationContainer sinkApps;

std::map<int, int> gsToSat;  // ground station ID → satellite ID
std::map<std::pair<int, int>, double> linkRate;  // (GS, Sat) → Mbps

std::map<int, bool> gsStarted;   // ground station 是否開始傳送
std::map<int, bool> gsFinished;  // ground station 是否完成傳送
std::map<int, bool> satBusy;     // satellite 是否正在接收資料

std::map<int, double> startTime;      // 每個 ground station 的開始時間
std::map<int, double> endTime;        // 每個 ground station 的結束時間
std::map<int, double> collectionTime; // 每顆 satellite 的收集時間

std::map<int, uint64_t> currentRxBytes;  // gsId → 已接收的位元組數

uint64_t maxbytes = 125000;

static void EchoRx(std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket);
void SendPacket(int gsId, int satId);
string GetNodeId(string str);
void Connect();

static void EchoRx(std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket){	
  // Task 3: Complete this function
  int nodeId = std::stoi(GetNodeId(context));
  if (nodeId >= (int)satellites.GetN()) return;  // 非衛星不處理

  int satId = nodeId;
  Ptr<Node> sat = satellites.Get(satId);
  Ptr<PacketSink> sink = DynamicCast<PacketSink>(sat->GetApplication(0));
  if (!sink) return;

  uint64_t totalRx = sink->GetTotalRx();

  for (auto &pair : gsToSat) {
      int gsId = pair.first;
      if (pair.second == satId && gsStarted[gsId] && !gsFinished[gsId]) {
          uint64_t rxSoFar = totalRx - currentRxBytes[gsId];

          if (rxSoFar >= maxbytes) {
              endTime[gsId] = Simulator::Now().GetSeconds();
              gsFinished[gsId] = true;
              satBusy[satId] = false;

              collectionTime[satId] = std::max(collectionTime[satId], endTime[gsId]);

              // 嘗試啟動下一筆任務
              for (auto &next : gsToSat) {
                  int nextGs = next.first;
                  int nextSat = next.second;
                  if (!gsStarted[nextGs] && !satBusy[nextSat]) {
                      SendPacket(nextGs, nextSat);
                      break;
                  }
              }

              // 更新 currentRxBytes 為目前新基準
              currentRxBytes[gsId] = totalRx;

              break;
          }
      }
  }


}

void SendPacket(int gsId, int satId){
  // Task 2.1: Complete this function

  Ptr<Node> gs = groundStations.Get(gsId);
  Ptr<Node> sat = satellites.Get(satId);

  // 初始化該地面站的接收起始點
  Ptr<PacketSink> sink = DynamicCast<PacketSink>(sat->GetApplication(0));
  if (sink) {
      currentRxBytes[gsId] = sink->GetTotalRx();  // 記錄這次傳輸前的總接收量
  }

  // 設定雙向 DataRate
  std::ostringstream oss;
  oss << linkRate[{gsId, satId}] << "Kbps";
  utNet.Get(gs->GetId())->GetObject<MockNetDevice>()->SetDataRate(DataRate(oss.str()));
  utNet.Get(sat->GetId())->GetObject<MockNetDevice>()->SetDataRate(DataRate(oss.str()));
  
  // 取得 Satellite IP
  Ptr<Ipv4> ipv4 = sat->GetObject<Ipv4>();
  Ipv4Address dstAddr = ipv4->GetAddress(1, 0).GetLocal();

  // 建立 BulkSend TCP App
  BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(dstAddr, port));
  source.SetAttribute("MaxBytes", UintegerValue(maxbytes)); 
  source.SetAttribute("SendSize", UintegerValue(512));

  ApplicationContainer srcApp = source.Install(gs);
  srcApp.Start(Seconds(0));

  // 記錄傳輸狀態
  gsStarted[gsId] = true;
  satBusy[satId] = true;
  startTime[gsId] = Simulator::Now().GetSeconds();

}

string GetNodeId(string str) {
  // Which node
  size_t pos1 = str.find("/", 0);           // The first "/"
  size_t pos2 = str.find("/", pos1 + 1);    // The second "/"
  size_t pos3 = str.find("/", pos2 + 1);    // The third "/"
  return str.substr(pos2 + 1, pos3 - pos2 - 1); // Node id
}

void Connect(){
  Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx", MakeCallback (&EchoRx));
}

NS_LOG_COMPONENT_DEFINE ("Lab4");

int main(int argc, char *argv[]){

  CommandLine cmd;
  string constellation = "TelesatGateway";
  double duration = 100;
  string inputFile = "network.ortools.out";
  string outputFile = "lab4.ortools.out";

  cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
  cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
  cmd.AddValue("inputFile", "Input file", inputFile);
  cmd.AddValue("outputFile", "Output file", outputFile);
  cmd.Parse (argc, argv);

  // Default setting
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
  Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(Seconds(2.0)));

  // Satellite
  LeoOrbitNodeHelper orbit;
  satellites = orbit.Install({ LeoOrbit(1200, 20, 1, 60)});

  // Ground station
  LeoGndNodeHelper ground;
  ground.Add(groundStations, LeoLatLong(20, 4));
  ground.Add(groundStations, LeoLatLong(19, 12));
  ground.Add(groundStations, LeoLatLong(19, 10));
  ground.Add(groundStations, LeoLatLong(19, 19));
  ground.Add(groundStations, LeoLatLong(19, 20));
  ground.Add(groundStations, LeoLatLong(18, 20));
  ground.Add(groundStations, LeoLatLong(18, 22));
  ground.Add(groundStations, LeoLatLong(17, 26));
  ground.Add(groundStations, LeoLatLong(18, 30));
  ground.Add(groundStations, LeoLatLong(15, 40));
  ground.Add(groundStations, LeoLatLong(14, 25));
  ground.Add(groundStations, LeoLatLong(14, 30));
  ground.Add(groundStations, LeoLatLong(14, 40));
  ground.Add(groundStations, LeoLatLong(14, 50));
  ground.Add(groundStations, LeoLatLong(14, 52));
  ground.Add(groundStations, LeoLatLong(13, 50));
  ground.Add(groundStations, LeoLatLong(13, 48));
  ground.Add(groundStations, LeoLatLong(12, 50));
  ground.Add(groundStations, LeoLatLong(13, 52));
  ground.Add(groundStations, LeoLatLong(15, 30));

  // Set network
  LeoChannelHelper utCh;
  utCh.SetConstellation (constellation);
  utNet = utCh.Install (satellites, groundStations);

  AodvHelper aodv;
  aodv.Set ("EnableHello", BooleanValue (false));
  
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (satellites);
  stack.Install (groundStations);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  ipv4.Assign (utNet);

  // Receiver: satellites
  PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  for(int i=0; i<60; i++){
    sinkApps.Add(sink.Install(satellites.Get(i)));
  }

  // Task 1: Input File
  // Step 1: Read network.graph for link data rates
  std::ifstream graphFile("contrib/leo/examples/network.graph");
  if (!graphFile) {
      std::cerr << "Cannot open network.graph file" << std::endl;
      return 1;
  }
  int numGs, numSat, numLinks;
  graphFile >> numGs >> numSat >> numLinks;
  std::string dummy;
  std::getline(graphFile, dummy);
  for (int i = 0; i < numLinks; ++i) {
      int gsId, satId;
      double rate;
      graphFile >> gsId >> satId >> rate;
      linkRate[{gsId, satId}] = rate;
  }
  graphFile.close();

  // Step 2: Read inputFile (e.g., network.ortools.out) for GS-Sat mapping
  std::string fullPath = "contrib/leo/examples/" + inputFile;
  std::ifstream inFile(fullPath);
  if (!inFile) {
      std::cerr << "Cannot open input file: " << inputFile << std::endl;
      return 1;
  }
  // Skip the first line (header)
  std::getline(inFile, dummy);

  int gsId, satId;
  int i=0;
  while (inFile >> gsId >> satId) {
      gsToSat[gsId] = satId;
      i++;
      if (i==20) break;
  }
  inFile.close();

  // Task 2.2: Call SendPacket()
  for (auto &pair : gsToSat) {
    int gsId = pair.first;
    int satId = pair.second;
    if (!gsStarted[gsId] && !satBusy[satId]) {
      SendPacket(gsId, satId);
    }
  }

  Simulator::Schedule(Seconds(1e-7), &Connect);
  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();

  // Task 4: Output File
  fullPath = "contrib/leo/examples/" + outputFile;
  std::ofstream outFile(fullPath);
  if (!outFile) {
      std::cerr << "Cannot open output file: " << outputFile << std::endl;
      return 1;
  }

  // 計算總收集時間
  double totalTime = 0;
  for (auto& pair : endTime) {
      totalTime = std::max(totalTime, pair.second);
  }
  outFile << totalTime << std::endl;

  // 每顆衛星的 collection time（ID 排序）
  for (uint32_t i = 0; i < satellites.GetN(); ++i) {
      double time = collectionTime.count(i) ? collectionTime[i] : 0.0;
      if (time != 0) {
        outFile << i << " " << time << std::endl;
      }
  }

  // 每個 ground station 的 start / end time（ID 排序）
  for (uint32_t i = 0; i < groundStations.GetN(); ++i) {
      double start = startTime.count(i) ? startTime[i] : 0.0;
      double end = endTime.count(i) ? endTime[i] : 0.0;
      outFile << i << " " << start << " " << end << std::endl;
  }
  outFile.close();


  return 0;
}