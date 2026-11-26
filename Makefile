OBJ_DIR = build

server: $(OBJ_DIR) $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o
	g++ -std=c++17 $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o -o $(OBJ_DIR)/server

$(OBJ_DIR)/main.o: $(OBJ_DIR) src/main.cpp
	g++ -std=c++17 -c src/main.cpp -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/server.o: $(OBJ_DIR) src/server.cpp src/server.hpp
	g++ -std=c++17 -c src/server.cpp -o $(OBJ_DIR)/server.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf build/*
