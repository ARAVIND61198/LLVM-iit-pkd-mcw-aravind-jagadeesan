CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

lvn: lvn.cpp
	$(CXX) $(CXXFLAGS) -o lvn lvn.cpp

.PHONY: run test sample clean
run: lvn
	./lvn input.quad

sample: lvn
	@echo "===== PDF example (input.quad) ====="      >  sample_output.txt
	@./lvn input.quad                                  >> sample_output.txt
	@echo ""                                           >> sample_output.txt
	@echo "===== Extra (input_extra.quad) ====="       >> sample_output.txt
	@./lvn input_extra.quad                            >> sample_output.txt
	@cat sample_output.txt

test: lvn
	@echo "===== PDF example (input.quad) ====="
	@./lvn input.quad
	@echo ""
	@echo "===== Extra: commutativity + arrays (input_extra.quad) ====="
	@./lvn input_extra.quad

clean:
	rm -f lvn sample_output.txt
