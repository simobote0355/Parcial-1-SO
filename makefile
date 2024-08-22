# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++11 -Iinclude -Wall -Wextra

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Archivos fuente y de cabecera
SRCS = $(wildcard $(SRC_DIR)/*.cpp) main.cpp 
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# Archivo ejecutable
TARGET = $(BUILD_DIR)/chat

# Regla por defecto
all: $(TARGET)

# Regla para crear el directorio de compilación
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para compilar los archivos objeto
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	rm -rf $(BUILD_DIR)

# Ejecutar el servidor
run-server1: $(TARGET)
	./$(TARGET) servidor 12346
	
run-server2: $(TARGET)
	./$(TARGET) servidor 12347

# Ejecutar el cliente
run-cliente1: $(TARGET)
	./$(TARGET) cliente 127.0.0.1 12346

run-cliente2: $(TARGET)
	./$(TARGET) cliente 127.0.0.1 12347

run-monitor1: $(TARGET)
	./$(TARGET) monitor 127.0.0.1 12346

run-monitor2: $(TARGET)
	./$(TARGET) monitor 127.0.0.1 12347

# Declarar reglas como phony
.PHONY: all clean run-servidor run-cliente
