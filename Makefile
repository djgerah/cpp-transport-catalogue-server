BIN = transport_catalogue
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -pthread -Iinclude
SRC_DIR = src
OBJ_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(OBJ_DIR)/$(BIN)

all: run

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR)

clang-format:
	find $(SRC_DIR) include -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -style=Microsoft -i {} +

rebuild: clean all

.PHONY: all run clean rebuild format