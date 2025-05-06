# 📡 CS341 Networking Project – A Comprehensive Systems-Level Exploration

This repository contains the full set of assignments completed as part of the **CS341: Computer Networks Laboratory** course. The project is divided into five progressively complex modules, each exploring key aspects of computer networking through simulation, protocol implementation, monitoring, and performance analysis.

---

## 🧭 Table of Contents
- [Overview](#overview)
- [Assignment Structure](#assignment-structure)
- [Technologies Used](#technologies-used)
- [How to Run](#how-to-run)
- [Project Modules](#project-modules)
  - [1. HTTP Traffic & Proxy Simulation](#1-http-traffic--proxy-simulation)
  - [2. Multi-Mode Networked Systems](#2-multi-mode-networked-systems)
  - [3. Packet Scheduling Algorithms](#3-packet-scheduling-algorithms)
  - [4. Distance Vector Routing Protocol](#4-distance-vector-routing-protocol)
  - [5. Collision Management in Ethernet & Wi-Fi (ns-3)](#5-collision-management-in-ethernet--wi-fi-ns-3)
- [Author](#author)

---

## 📖 Overview

This project presents a hands-on implementation of key computer networking principles including:
- Application-layer protocols (HTTP, DNS)
- Proxy and caching mechanisms
- Congestion control
- Router-level scheduling
- Routing algorithms
- Wired vs Wireless collision management (CSMA/CD & CSMA/CA)

Each module is implemented in Python or C++, using raw sockets or simulation tools such as **ns-3**, and validated via real traffic patterns or simulations.

---

## 📂 Assignment Structure

cs341-networking-project/
├── Assignment1_HTTP_Proxy_DNS/
├── Assignment2_Drone_Weather_TCP_UDP/
├── Assignment3_Scheduling_iSLIP/
├── Assignment4_Distance_Vector_Routing/
├── Assignment5_NS3_CSMA_CD_CA/
└── README.md


---

## 🛠️ Technologies Used

- **Python (3.8+)** – socket programming, threading
- **C++ with ns-3** – discrete event simulation
- **Wireshark & tcpdump** – traffic capture and inspection
- **Matplotlib** – performance graphing
- **Linux/WSL** – development environment

---

## ▶️ How to Run

1. Clone this repo:
   ```bash
   git clone https://github.com/yourusername/cs341-networking-project.git
2. Navigate to a module:
    cd Assignment1_HTTP_Proxy_DNS
3. Follow the instructions in each folder's README.
4. For ns-3 simulations:
    Make sure ns-3 is installed.
    Build and run simulations from the Assignment5_NS3_CSMA_CD_CA directory.

## 📦 Project Modules

### 1. HTTP Traffic & Proxy Simulation
- Analyzed live HTTP traffic using **Wireshark**.
- Implemented recursive DNS resolution and an **LRU-caching HTTP proxy** using sockets.
- Extended proxy with session-level cookie management and analytics.

### 2. Multi-Mode Networked Systems
- Developed a **tri-mode communication stack** for drones (control, telemetry, and file transfer).
- Simulated real-time weather data streaming with **TCP Reno-style congestion control** and compression.

### 3. Packet Scheduling Algorithms
- Simulated an **8×8 input/output router switch**.
- Implemented and compared:
  - Priority Scheduling  
  - Weighted Fair Queuing  
  - Round Robin  
  - **iSLIP** (iterative matching)
- Analyzed metrics: **throughput**, **latency**, **fairness**, **drop rate**.

### 4. Distance Vector Routing Protocol
- Built a complete **DVR simulator** handling topology changes.
- Implemented count-to-infinity prevention using:
  - Poisoned Reverse  
  - Split Horizon
- Simulated **edge failures** and **routing table stabilization**.

### 5. Collision Management in Ethernet & Wi-Fi (ns-3)
- Used **ns-3** to simulate:
  - **Ethernet (CSMA/CD)** with collision and backoff behavior.
  - **Wi-Fi (CSMA/CA)** in a 5×5 ad-hoc grid with UDP flows.
- Ran experiments **with and without RTS/CTS** to compare throughput and drop rate.

---

## 👨‍💻 Author

Course: **CS341 – Computer Networks Lab**  
Institution: **IIT Guwahati**  
Year: **2024**

- [Sushant Kumar](https://github.com/LegendsDen)
- [Sneh Dhandania](https://github.com/GittySneh)
- [Priyanshu Pratyay](https://github.com/PPratyay)
- [Simon Lalremsiama Shangpliang](https://github.com/SimonShangpliang)

