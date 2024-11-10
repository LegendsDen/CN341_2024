import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV data
data = pd.read_csv('csma_simulation_results.csv')

# Get unique Flow IDs in the data
flow_ids = data['FlowId'].unique()

# Plot Packet Loss vs. Time for each Flow ID and save as image
plt.figure(figsize=(10, 6))
for flow_id in flow_ids:
    flow_data = data[data['FlowId'] == flow_id]
    plt.plot(flow_data['Time'], flow_data['PacketLoss'], label=f'Flow ID {flow_id}')

plt.xlabel('Time (s)')
plt.ylabel('Packet Loss (packets)')
plt.title('Packet Loss vs. Time for Each Flow ID')
plt.legend()
plt.grid(True)
plt.savefig('packet_loss_vs_time.png')  # Save the plot as an image

# Plot Throughput vs. Time for each Flow ID and save as image
plt.figure(figsize=(10, 6))
for flow_id in flow_ids:
    flow_data = data[data['FlowId'] == flow_id]
    plt.plot(flow_data['Time'], flow_data['Throughput'], label=f'Flow ID {flow_id}')

plt.xlabel('Time (s)')
plt.ylabel('Throughput (Mbps)')
plt.title('Throughput vs. Time for Each Flow ID')
plt.legend()
plt.grid(True)
plt.savefig('throughput_vs_time.png')  # Save the plot as an image
