# Makefile para code/matrix_multiplication
#
# Compila un ejecutable distinto por cada algoritmo (ambos comparten la
# función multiplyMatrices, por eso no se pueden enlazar juntos), corre
# las mediciones sobre todo lo que haya en data/matrix_input/, y genera
# los gráficos a partir de data/measurements/.

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
PYTHON   := python3

ALGORITHMS := naive strassen
BINARIES   := $(addprefix bin/,$(ALGORITHMS))

.PHONY: all run plot clean generate-data

all: $(BINARIES)

bin/%: algorithms/%.cpp matrix_multiplication.cpp | bin
	$(CXX) $(CXXFLAGS) -o $@ matrix_multiplication.cpp algorithms/$*.cpp -lpsapi

bin:
	mkdir -p bin

# Genera las matrices de entrada usando el script provisto
# (scripts/matrix_generator.py). Solo es necesario correrlo una vez.
generate-data:
	cd scripts && $(PYTHON) matrix_generator.py

# Corre cada algoritmo sobre todos los pares de matrices de
# data/matrix_input/. Cada ejecutable recibe su propio nombre como
# argumento, usado solo como etiqueta para data/measurements/<algoritmo>.csv
run: all
	@for alg in $(ALGORITHMS); do \
		echo "== Corriendo $$alg =="; \
		./bin/$$alg $$alg; \
	done

plot:
	cd scripts && $(PYTHON) plot_generator.py

clean:
	rm -rf bin
	rm -f data/measurements/*.csv
	rm -f data/measurements/*_omitidos.txt
	rm -f data/matrix_output/*.txt
	rm -f data/plots/*.png