# Test su PC: make -C test
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined
FW := ../firmware/spinlite
BIN := bin

TESTS := $(BIN)/test_detector $(BIN)/test_protocol $(BIN)/test_oled $(BIN)/test_settings

all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t -q || exit 1; done

$(BIN)/test_detector: test_detector.cpp signal_sim.h $(FW)/rpm_detector.cpp $(FW)/rpm_detector.h
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ test_detector.cpp $(FW)/rpm_detector.cpp

$(BIN)/test_protocol: test_protocol.cpp $(FW)/bbp_protocol.cpp $(FW)/bbp_protocol.h
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ test_protocol.cpp $(FW)/bbp_protocol.cpp

$(BIN)/test_oled: test_oled.cpp $(FW)/oled_ui.cpp $(FW)/oled_ui.h
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ test_oled.cpp $(FW)/oled_ui.cpp

$(BIN)/test_settings: test_settings.cpp $(FW)/settings.cpp $(FW)/settings.h $(FW)/battery.cpp $(FW)/battery.h $(FW)/bbp_protocol.cpp
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ test_settings.cpp $(FW)/settings.cpp $(FW)/battery.cpp $(FW)/bbp_protocol.cpp

clean:
	rm -rf $(BIN)

.PHONY: all clean
