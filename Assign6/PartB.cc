#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-mac-trailer.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhocGrid");



void ReceivePacket(Ptr<Socket> socket) {
    while (socket->Recv()) {
        // Packet reception code can be added here (e.g., logging)
    }
}

static void GenerateTraffic(Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval) {
    if (pktCount > 0) {
        socket->Send(Create<Packet>(pktSize));
        Simulator::Schedule(pktInterval, &GenerateTraffic, socket, pktSize, pktCount - 1, pktInterval);
    } else {
        socket->Close();
    }
}

int main(int argc, char* argv[]) {
    std::string phyMode("DsssRate1Mbps");
    double distance = 100.0;      // m
    uint32_t packetSize = 500;   // bytes
    uint32_t numPackets = 100;

    uint32_t packetSize2 = 500;  // bytes
    uint32_t numPackets2 = 100;

    uint32_t packetSize3 = 500;  // bytes
    uint32_t numPackets3 = 100;

    uint32_t numNodes = 25;  // 5x5 grid
    uint32_t sinkNode1 = 0, sourceNode1 = 24;  // Flow 1: Diagonal 1
    uint32_t sinkNode2 = 4, sourceNode2 = 20;  // Flow 2: Diagonal 2
    uint32_t sinkNode3 = 10, sourceNode3 = 14; // Flow 3: Middle horizontal

    double interval = 0.004; // seconds
    bool verbose = false;
    bool tracing = true;
    bool rtsCtsThreshold = false; // Default value: no RTS/CTS

    CommandLine cmd(__FILE__);
    cmd.AddValue("rtsCtsThreshold", "Enable RTS/CTS (true/false)", rtsCtsThreshold);

    cmd.Parse(argc, argv);

    Time interPacketInterval = Seconds(interval);

    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
if(rtsCtsThreshold){
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(0));
}
    // Log topology information
    // NS_LOG_UNCOND("Topology:");
    // NS_LOG_UNCOND("-----------");
    // NS_LOG_UNCOND("|n0  |  n1|");
    // NS_LOG_UNCOND("|n2  |  n3|");
    // NS_LOG_UNCOND("-----------");
    NS_LOG_UNCOND("UDP Flow Configurations:");
    NS_LOG_UNCOND("Flow 1: n0 ---> n24 (Diagonal 1)");
    NS_LOG_UNCOND("Flow 2: n4 ---> n20 (Diagonal 2)");
    NS_LOG_UNCOND("Flow 3: n10 ---> n14 (Middle Horizontal)");



    NodeContainer c;
    c.Create(numNodes);

    WifiHelper wifi;
    if (verbose) {
        WifiHelper::EnableLogComponents();  // Turn on all Wifi logging
    }
    wifi.SetStandard(WIFI_STANDARD_80211b);
    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("RxGain", DoubleValue(0));

    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue(phyMode),
                                 "ControlMode", StringValue(phyMode));
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(distance),
                                  "DeltaY", DoubleValue(distance),
                                  "GridWidth", UintegerValue(5),  // 5x5 grid
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(c);

    // Enable OLSR routing protocol
    OlsrHelper olsr;
    Ipv4ListRoutingHelper list;
    list.Add(olsr, 0);

    InternetStackHelper internet;
    internet.SetRoutingHelper(list);
    internet.Install(c);

    // Assign IP Addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign(devices);
    // Flow 1: Diagonal from n0 to n24
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    Ptr<Socket> recvSink1 = Socket::CreateSocket(c.Get(sinkNode1), tid);
    InetSocketAddress local1 = InetSocketAddress(Ipv4Address::GetAny(), 80);
    recvSink1->Bind(local1);
    recvSink1->SetRecvCallback(MakeCallback(&ReceivePacket));

    Ptr<Socket> source1 = Socket::CreateSocket(c.Get(sourceNode1), tid);
    InetSocketAddress remote1 = InetSocketAddress(i.GetAddress(sinkNode1, 0), 80);
    source1->Connect(remote1);

    // Flow 2: Diagonal from n4 to n20
    Ptr<Socket> recvSink2 = Socket::CreateSocket(c.Get(sinkNode2), tid);
    InetSocketAddress local2 = InetSocketAddress(Ipv4Address::GetAny(), 81);
    recvSink2->Bind(local2);
    recvSink2->SetRecvCallback(MakeCallback(&ReceivePacket));

    Ptr<Socket> source2 = Socket::CreateSocket(c.Get(sourceNode2), tid);
    InetSocketAddress remote2 = InetSocketAddress(i.GetAddress(sinkNode2, 0), 81);
    source2->Connect(remote2);

    // Flow 3: Middle row from n10 to n14
    Ptr<Socket> recvSink3 = Socket::CreateSocket(c.Get(sinkNode3), tid);
    InetSocketAddress local3 = InetSocketAddress(Ipv4Address::GetAny(), 82);
    recvSink3->Bind(local3);
    recvSink3->SetRecvCallback(MakeCallback(&ReceivePacket));

    Ptr<Socket> source3 = Socket::CreateSocket(c.Get(sourceNode3), tid);
    InetSocketAddress remote3 = InetSocketAddress(i.GetAddress(sinkNode3, 0), 82);
    source3->Connect(remote3);

    // Enable tracing (if needed)
    if (tracing) {
        AsciiTraceHelper ascii;
        wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifi-simple-adhoc-grid.tr"));
        wifiPhy.EnablePcap("wifi-simple-adhoc-grid", devices);
    }

    // Schedule traffic generation
    Simulator::Schedule(Seconds(15.0), &GenerateTraffic, source1, packetSize, numPackets, interPacketInterval);
    Simulator::Schedule(Seconds(30.0), &GenerateTraffic, source2, packetSize2, numPackets2, interPacketInterval);
    Simulator::Schedule(Seconds(45.0), &GenerateTraffic, source3, packetSize3, numPackets3, interPacketInterval);

    // Run the simulation
    Simulator::Stop(Seconds(100.0));

    // Flow monitor
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Animation interface
    AnimationInterface anim("animationwifi-adhoc-wireless2.xml");

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();

    // Flow monitor statistics
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
    for (auto iter = stats.begin(); iter != stats.end(); ++iter) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
        NS_LOG_UNCOND("Flow ID:- " << iter->first << " Source addr: " << t.sourceAddress << " Dest Addr: " << t.destinationAddress);
        uint32_t protocol = t.protocol;
        if (protocol == 6) {
            std::cout << "Type: TCP Flow" << std::endl;
        } else if (protocol == 17) {
            std::cout << "Type: UDP Flow" << std::endl;
        }
        NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
        NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
        NS_LOG_UNCOND("Lost packets = " << iter->second.lostPackets);
        NS_LOG_UNCOND("Times forwarded = " << iter->second.timesForwarded);
        NS_LOG_UNCOND("Delay = " << iter->second.delaySum);
        NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps");
        NS_LOG_UNCOND("-----------------------------------------------------------");
    }

    flowMonitor->SerializeToXmlFile("flow-monitor-output.xml", true, true);


    Simulator::Destroy();
    return 0;
}
