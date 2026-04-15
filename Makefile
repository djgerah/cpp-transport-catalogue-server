BIN = transport_catalogue
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -Iinclude -pthread

SRC_DIR = src
OBJ_DIR = build

SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = $(OBJ_DIR)/$(BIN)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -lcpprest -lboost_system -lssl -lcrypto -o $@

all: build

build: $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR)

rebuild: clean all

clang-format:
	find $(SRC_DIR) include -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -style=Microsoft -i {} +

.PHONY: all run clean rebuild clang-format