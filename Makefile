OBJ_DIR = build

server: $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o
	g++ $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o -o $(OBJ_DIR)/server

$(OBJ_DIR)/main.o: src/main.cpp
	g++ -c src/main.cpp -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/server.o: src/server.cpp src/server.hpp
	g++ -c src/server.cpp -o $(OBJ_DIR)/server.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf build/*
