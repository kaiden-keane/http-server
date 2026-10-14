OBJ_DIR = build

server: $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o
	g++ $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o -o $(OBJ_DIR)/server

$(OBJ_DIR)/main.o: main.cpp
	g++ -c main.cpp -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/server.o: server.cpp server.hpp
	g++ -c server.cpp -o $(OBJ_DIR)/server.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf build/*
