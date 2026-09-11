# Makefile para code/sorting
#
# Compila un ejecutable distinto por cada algoritmo (los 4 comparten la
# función sortArray, por eso no se pueden enlazar juntos), corre las
# mediciones sobre todo lo que haya en data/array_input/, y genera los
# gráficos a partir de data/measurements/.

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
PYTHON   := python3

ALGORITHMS := mergesort quicksort patiencesort sort
BINARIES   := $(addprefix bin/,$(ALGORITHMS))

.PHONY: all run plot clean generate-data

all: $(BINARIES)

bin/%: algorithms/%.cpp sorting.cpp | bin
	$(CXX) $(CXXFLAGS) -o $@ sorting.cpp algorithms/$*.cpp -lpsapi

bin:
	mkdir -p bin

# Genera los arreglos de entrada usando el script provisto
# (scripts/array_generator.py). Solo es necesario correrlo una vez.
generate-data:
	cd scripts && $(PYTHON) array_generator.py

# Corre cada algoritmo sobre todos los arreglos de data/array_input/.
# Cada ejecutable recibe su propio nombre como argumento, usado solo
# como etiqueta para data/measurements/<algoritmo>.csv
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
	rm -f data/array_output/*.txt
	rm -f data/plots/*.png