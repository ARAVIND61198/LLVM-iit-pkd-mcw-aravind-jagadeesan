CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

lvn: lvn.cpp
	$(CXX) $(CXXFLAGS) -o lvn lvn.cpp

.PHONY: run test clean
run: lvn
	./lvn input.quad

test: lvn
	@echo "===== PDF example (input.quad) ====="
	@./lvn input.quad
	@echo ""
	@echo "===== Extra: commutativity + arrays (input_extra.quad) ====="
	@./lvn input_extra.quad

clean:
	rm -f lvn
