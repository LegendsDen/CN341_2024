#TO run the code  
Installation procedure
 sudo apt update
 
 sudo apt install -y     build-essential     g++     python3     python3-dev     python3-pip     gcc     g++     libsqlite3-dev     git     qt5-qmake     libqt5webkit5-dev     libxml2-dev     libgtk-3-dev     libboost-all-dev     libssl-dev     libgsl-dev     libc6-dev     libncurses5-dev     libsdl1.2-dev     libgconf-2-4     libx11-dev     pkg-config     cmake     libpcap-dev     libjpeg-dev
 
 cd ~
 
 git clone https://gitlab.com/nsnam/ns-3-allinone.git
 
 cd ns-3-allinone
 
 ./download.py -n ns-3.35
 
 cd ns-3.35  # Or the version you downloaded
 
 ./waf configure
 
 ./waf build
 
 chmod +x waf

Execution:

For part B:
 Execute ./waf --run "scratch/partB --rtsCtsThreshold=true" in ns3 folder where waf is present. For enabling rtsCts as true , use true above, else use false.

