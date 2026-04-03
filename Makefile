CXX ?= g++
PKG_CONFIG ?= pkg-config

CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -g
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags glew glfw3 2>/dev/null)

GLEW_LIBS := $(shell $(PKG_CONFIG) --libs glew 2>/dev/null)
GLFW_LIBS := $(shell $(PKG_CONFIG) --libs glfw3 2>/dev/null)

ifeq ($(strip $(GLEW_LIBS)),)
GLEW_LIBS := -lGLEW
endif

ifeq ($(strip $(GLFW_LIBS)),)
GLFW_LIBS := -lglfw
endif

LDLIBS += $(GLEW_LIBS) $(GLFW_LIBS) -lGL

MAIN_SRC := gravity_sim.cpp
GRID_SRC := gravity_sim_3Dgrid.cpp
TEST_SRC := 3D_test.cpp

MAIN_BIN := gravity_sim
GRID_BIN := gravity_sim_3Dgrid
TEST_BIN := 3D_test

.PHONY: all help main grid test run run-grid run-test clean

all: $(MAIN_BIN)

help:
	@printf '%s\n' \
		'Targets:' \
		'  make            Build the main gravity simulator' \
		'  make main       Build gravity_sim' \
		'  make grid       Build gravity_sim_3Dgrid' \
		'  make test       Build 3D_test' \
		'  make run        Build and run gravity_sim' \
		'  make run-grid   Build and run gravity_sim_3Dgrid' \
		'  make run-test   Build and run 3D_test' \
		'  make clean      Remove compiled binaries'

main: $(MAIN_BIN)

grid: $(GRID_BIN)

test: $(TEST_BIN)

run: $(MAIN_BIN)
	./$(MAIN_BIN)

run-grid: $(GRID_BIN)
	./$(GRID_BIN)

run-test: $(TEST_BIN)
	./$(TEST_BIN)

$(MAIN_BIN): $(MAIN_SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(GRID_BIN): $(GRID_SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(MAIN_BIN) $(GRID_BIN) $(TEST_BIN)
