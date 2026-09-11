// patiencesort.cpp
//
// Ordenamiento mediante PATIENCE SORT, inspirado en el solitario
// homonimo. Complejidad teórica: O(n log n) en tiempo, O(n) en espacio.
//
// (1) Reparto en pilas: cada elemento x va a la pila mas a la
//     izquierda cuyo tope sea >= x (búsqueda binaria, ya que los topes
//     quedan siempre en orden no decreciente); si ninguna califica, se
//     abre una pila nueva. O(n log k), con k = cantidad de pilas.
// (2) Fusión de k vías con un min-heap sobre los topes. O(n log k).
//
// k depende del tipo de entrada y de cuántos valores se repiten:
// descendente siempre da k=1, mientras que ascendente con valores
// mayoritariamente distintos da k≈n (ver dominio D1 vs D7 en las
// mediciones); el peor caso teórico se mantiene en O(n log n) en
// tiempo, pero la memoria real depende fuertemente de k.
//
// Referencias / bibliografia:
// - Mallows, C. L. (1963). Patience Sorting. SIAM Review, 5(4),
//   375-376
// - Aldous, D., & Diaconis, P. (1999). Longest increasing subsequences:
//   from patience sorting to the Baik-Deift-Johansson theorem.
//   Bulletin of the American Mathematical Society, 36(4), 413-432
// - Knuth, D. E. (1998). The Art of Computer Programming, Vol. 3:
//   Sorting and Searching (2nd ed.), Sec. 5.1.4, ejercicios sobre el
//   solitario "Patience" como metodo de ordenamiento. Addison-Wesley.

#include <vector>
#include <algorithm>
#include <queue>
#include <utility>
#include <cstddef>
using namespace std;

void sortArray(vector<int>& arr) {
    if (arr.size() < 2) {
        return;
    }
    // Fase 1 repartir en pilas
    vector<vector<int>> piles;
    vector<int> tops; // tops[i] == piles[i].back(); se mantiene ordenado (no decreciente) para poder usar lower_bound
    for (int x : arr) {
        auto it = lower_bound(tops.begin(), tops.end(), x);
        if (it == tops.end()) {
            piles.push_back(vector<int>{x});
            tops.push_back(x);
        } else {
            size_t idx = static_cast<size_t>(it - tops.begin());
            piles[idx].push_back(x);
            tops[idx] = x;
        }
    }
    // Fase 2 fusion de k vias con un min-heap 
    using PQItem = pair<int, size_t>; // (valor, indice de pila)
    priority_queue<PQItem, vector<PQItem>, greater<PQItem>> pq;

    for (size_t i = 0; i < piles.size(); ++i) {
        pq.emplace(piles[i].back(), i);
    }

    vector<int> result;
    result.reserve(arr.size());

    while (!pq.empty()) {
        auto [val, idx] = pq.top();
        pq.pop();
        result.push_back(val);
        piles[idx].pop_back();
        if (!piles[idx].empty()) {
            pq.emplace(piles[idx].back(), idx);
        }
    }

    arr = move(result);
}