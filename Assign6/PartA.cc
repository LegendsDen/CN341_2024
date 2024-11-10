#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>

using namespace ns3;

// Enable logging for the CSMA/CD protocol
NS_LOG_COMPONENT_DEFINE("CsmaSimulation");

void TrafficGenerator(NodeContainer &nodes, Ipv4InterfaceContainer &interfaces)
{
    uint16_t port = 9;

    // Set up high-traffic load and configure each node as both sender and receiver
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        // Configure OnOffApplication to send data to a randomly selected node (e.g., next node in line)
        OnOffHelper onOff("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress((i + 1) % nodes.GetN()), port)));
        onOff.SetConstantRate(DataRate("100Mbps"), 1024); // High data rate with 1024-byte packets
        
        // Install the application on each node
        ApplicationContainer app = onOff.Install(nodes.Get(i));
        app.Start(Seconds(0.5 * i)); // Stagger start times to increase collisions
        app.Stop(Seconds(10.0));

        // Configure a PacketSink on each node to receive data on the specified port
        PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
        ApplicationContainer sinkApp = sink.Install(nodes.Get(i));
        sinkApp.Start(Seconds(0.0));
        sinkApp.Stop(Seconds(10.0));
    }
}


// Periodic logging function for flow metrics
void LogMetrics(Ptr<FlowMonitor> flowMonitor, FlowMonitorHelper &flowHelper, double interval)
{
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();

    std::ofstream resultsFile("csma_simulation_results.csv", std::ios_base::app);
    double currentTime = Simulator::Now().GetSeconds();
    
    for (auto iter = stats.begin(); iter != stats.end(); ++iter)
    {
        // Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);
        double throughput = iter->second.rxBytes * 8.0 / 
            (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024;
        double delay = iter->second.rxPackets > 0 ? iter->second.delaySum.GetSeconds() / iter->second.rxPackets : 0;
        double packetLoss = iter->second.txPackets - iter->second.rxPackets;

        resultsFile << iter->first << "," << currentTime << ","
                    << throughput << "," << packetLoss << "," << delay << "\n";
    }

    resultsFile.close();

    // Schedule the next logging event
    Simulator::Schedule(Seconds(interval), &LogMetrics, flowMonitor, std::ref(flowHelper), interval);
}

int main(int argc, char *argv[])
{
    // Enable CSMA/CD logging to observe collisions and backoff
    LogComponentEnable("CsmaNetDevice", LOG_LEVEL_INFO);

    uint32_t numNodes = 5;
    NodeContainer csmaNodes;
    csmaNodes.Create(numNodes);

    // Set up CSMA attributes with realistic Ethernet parameters
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(0.1)));

    NetDeviceContainer csmaDevices = csma.Install(csmaNodes);
    InternetStackHelper stack;
    stack.Install(csmaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign(csmaDevices);

    // Generate high-traffic load to encourage collisions
    TrafficGenerator(csmaNodes, csmaInterfaces);

    // Set up Flow Monitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> flowMonitor = flowHelper.InstallAll();

    // Initialize CSV file with headers
    std::ofstream resultsFile("csma_simulation_results.csv");
    resultsFile << "FlowId,Time,Throughput,PacketLoss,Delay\n";
    resultsFile.close();

    // Schedule periodic metric logging every second
    Simulator::Schedule(Seconds(0.0), &LogMetrics, flowMonitor, std::ref(flowHelper), 0.1);

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
