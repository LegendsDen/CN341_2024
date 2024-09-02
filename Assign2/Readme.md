
To run the server do the following

### Windows 

1. Install G++ if doesnt exist

2. run mingw32-make

3. run ./Q3.exe

### Linux (it might not run on linux because we have used windows specific libraries)
   1. if not installed
        sudo apt-get install g++ make

   2. To build the project execute
        make

   3. To remove the executable and object files, run:
        make clean

To connect to proxy server
 run
curl.exe -x http://localhost:8080 -H "Client-ID: random_client" -c cookies.txt http://www.google.com

random_client can be replaced with any client id you want.


TO run Q2 lru_cacheQ2.cpp
run g++ lru_cache.cpp -o lru_cache -lws2_32   and then to run .exe file we do 
./lru_cache

to run Q2 dns_lookupQ2.cpp
run g++ dns_lookupQ2.cpp -o dns_lookupQ2 -lws2_32   and then to run .exe file we do 
./dns_lookupQ2

and then user provides any website name whose dns query he wants to do ,for eg
www.google.com 
www.amazon.org
www.example.edu

in above format 

