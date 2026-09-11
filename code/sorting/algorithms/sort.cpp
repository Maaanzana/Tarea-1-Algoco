// sort.cpp
//
// Ordenamiento de un arreglo de enteros usando el algoritmo SORT de la
// biblioteca estándar de C++ (std::sort), incluido como baseline de
// comparacion 
// Complejidad teorica resultante: O(n log n) en el peor caso 
// Referencias / bibliografía:
// - Musser, D. R. (1997). Introspective sorting and selection
//   algorithms. Software: Practice and Experience, 27(8), 983-993.
// - Documentación de libstdc++, implementación de std::sort
//   (bits/stl_algo.h, función interna __introsort_loop).

#include <vector>
#include <algorithm>
using namespace std;

void sortArray(vector<int>& arr) {
    sort(arr.begin(), arr.end());
}