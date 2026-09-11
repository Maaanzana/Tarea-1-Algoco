// mergesort.cpp
//
// sort de un arreglo de enteros mediante MERGE SORT.
// Complejidad teórica: O(n log n) en tiempo, en todos los casos (mejor,
// promedio y peor) 
//
// Se reserva un único buffer auxiliar (mismo tamaño que el arreglo)
// antes de la recursión y se reutiliza en cada fusión, en vez de crear
// un vector nuevo por llamada recursiva (que sería el enfoque más "de
// libro" pero con O(n log n) allocaciones). Reduce las allocaciones a
// una sola sin cambiar la complejidad asintótica en tiempo.
//
// Referencias / bibliografía:
// - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
//   Introduction to Algorithms (3rd ed.), Cap. 2.3 "Divide-and-conquer:
//   merge sort". MIT Press.
// - Knuth, D. E. (1998). The Art of Computer Programming, Vol. 3:
//   Sorting and Searching (2nd ed.), Sec. 5.2.4 "Sorting by Merging".
//   Addison-Wesley.

#include <vector>
#include <cstddef>
using namespace std;
namespace {

// Ordena arr[lo, hi) usando buffer como espacio auxiliar de fusion
void mergeSortRec(vector<int>& arr, vector<int>& buffer, size_t lo, size_t hi) {
    if (hi - lo <= 1) {
        return;
    }
    size_t mid = lo + (hi - lo) / 2;
    mergeSortRec(arr, buffer, lo, mid);
    mergeSortRec(arr, buffer, mid, hi);

    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (arr[i] <= arr[j]) {
            buffer[k++] = arr[i++];
        } else {
            buffer[k++] = arr[j++];
        }
    }
    while (i < mid) buffer[k++] = arr[i++];
    while (j < hi) buffer[k++] = arr[j++];

    for (size_t t = lo; t < hi; ++t) {
        arr[t] = buffer[t];
    }
}

} // namespace
void sortArray(vector<int>& arr) {
    if (arr.size() < 2) {
        return;
    }
    vector<int> buffer(arr.size());
    mergeSortRec(arr, buffer, 0, arr.size());
}