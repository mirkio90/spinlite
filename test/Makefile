# Test su PC: make -C test
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined
FW := ../firmware/spinlite
BIN := bin

TESTS := $(BIN)/test_detector

all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

$(BIN)/test_detector: test_detector.cpp signal_sim.h $(FW)/rpm_detector.cpp $(FW)/rpm_detector.h
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ test_detector.cpp $(FW)/rpm_detector.cpp

clean:
	rm -rf $(BIN)

.PHONY: all clean
