### Usage:

- **To compile the server and client:**
  mingw32-make

- **To clean up the object files and executables:**
  mingw32-make clean

- **Running server:**  
  ./Q2_server <network_condition>

  Network condition can be "poor", "normal" or "strong".

- **Running client:**
  ./Q2_client <client_id> <network_condition>

  Client id can be any integer. Network condition can be "poor", "moderate" or "strong".
  For using mutex, thread and zlib libraries you should have g++ library version higher than 13.
