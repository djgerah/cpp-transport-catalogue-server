BIN = transport_catalogue
TEST_BIN = tests_runner

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -Iinclude -pthread -MMD -MP

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# LIBS
LDLIBS = -lcpprest -lboost_system -lssl -lcrypto
GTEST_LIBS = -lgtest -lgtest_main -pthread

# DOCKER
DOCKER_IMAGE = transport-catalogue
DOCKER_CONTAINER = transport-catalogue-container

# SOURCES
SRCS = $(shell find $(SRC_DIR) -name "*.cpp" ! -name "main.cpp")
MAIN_SRC = $(SRC_DIR)/main.cpp
TEST_SRCS = $(shell find $(TEST_DIR) -name "*.cpp")

# OBJECTS
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/src/%.o,$(SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

# TARGETS
TARGET = $(BUILD_DIR)/$(BIN)
TEST_TARGET = $(BUILD_DIR)/$(TEST_BIN)

# -------- BUILD RULES --------

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------- LINKING --------

$(TARGET): $(OBJS) $(MAIN_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDLIBS) -o $@

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDLIBS) $(GTEST_LIBS) -o $@

# -------- DOCKER --------

docker-build:
	docker build -t $(DOCKER_IMAGE) .

docker-run:
	docker run -it -p 8080:8080 $(DOCKER_IMAGE)

docker-run-bg:
	docker run -d -p 8080:8080 --name $(DOCKER_CONTAINER) $(DOCKER_IMAGE)

docker-stop:
	docker stop $(DOCKER_CONTAINER) || true
	docker rm $(DOCKER_CONTAINER) || true

docker-logs:
	docker logs -f $(DOCKER_CONTAINER)

# -------- COMMANDS --------

all: build

build: $(TARGET)

run: build
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

clang-format:
	find $(SRC_DIR) include tests -type f \( -name "*.cpp" -o -name "*.h" \) \
	-exec clang-format -style=Microsoft -i {} +

.PHONY: all build run test clean rebuild format \
docker-build docker-run docker-run-bg docker-stop docker-logs