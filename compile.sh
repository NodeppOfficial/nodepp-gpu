export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH

time g++ -o main main.cpp -L./lib -I./include -lraylib -lssl -lcrypto ; ./main