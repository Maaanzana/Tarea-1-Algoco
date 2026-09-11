// quicksort.cpp
//
// Es una implementacion con mediana de tres + insertion sort para subarreglos chicos + tail-call elimination son las 3 optimizaciones 
// "de manual de texto avanzado" que se usan en quicksorts.
// Complejidad teórica: O(n log n) en promedio. El peor caso O(n²) no
// se activa por el simple orden del arreglo (la mediana de tres
// selecciona el pivote central en arreglos ya ordenados con valores
// distintos), sino por la combinación de orden y baja cardinalidad de
// valores (muchos elementos repetidos/iguales al pivote degeneran la
// partición de Lomuto). Ver dominio D1 vs D7 en las mediciones.
// Referencias / bibliografía:
// - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
//   Introduction to Algorithms (3rd ed.), Cap. 7 "Quicksort", incluyendo
//   el análisis del peor caso (7.2) y el esquema de partición de
//   Lomuto. MIT Press.
// - Sedgewick, R. (1978). Implementing Quicksort Programs.
//   Communications of the ACM, 21(10), 847-857 (técnica de recursionar
//   sólo sobre la partición más chica para acotar la profundidad de la
//   pila).
// quicksort.cpp

#include <vector>
#include <utility>
using namespace std;

namespace {

// Ordenamiento por insercion para subarreglos pequeños
void insertionSort(vector<int>& arr, long long lo, long long hi) {
    for (long long i = lo + 1; i <= hi; ++i) {
        int key = arr[static_cast<size_t>(i)];
        long long j = i - 1;
        while (j >= lo && arr[static_cast<size_t>(j)] > key) {
            arr[static_cast<size_t>(j + 1)] = arr[static_cast<size_t>(j)];
            --j;
        }
        arr[static_cast<size_t>(j + 1)] = key;
    }
}


// Obtiene la posicion de la mediana entre arr[lo], arr[mid] y arr[hi]
long long medianOfThree(
    vector<int>& arr,
    long long lo,
    long long hi
) {
    long long mid = lo + (hi - lo) / 2;
    if (arr[static_cast<size_t>(lo)] >
        arr[static_cast<size_t>(mid)]) {
        swap(
            arr[static_cast<size_t>(lo)],
            arr[static_cast<size_t>(mid)]
        );
    }
    if (arr[static_cast<size_t>(lo)] >
        arr[static_cast<size_t>(hi)]) {
        swap(
            arr[static_cast<size_t>(lo)],
            arr[static_cast<size_t>(hi)]
        );
    }
    if (arr[static_cast<size_t>(mid)] >
        arr[static_cast<size_t>(hi)]) {
        swap(
            arr[static_cast<size_t>(mid)],
            arr[static_cast<size_t>(hi)]
        );
    }

    return mid;
}


// Particion de Lomuto
// El pivote se obtiene mediante mediana de tres
long long partition(
    vector<int>& arr,
    long long lo,
    long long hi
) {
    long long pivotIndex = medianOfThree(arr, lo, hi);
    // Llevar el pivote al final
    swap(
        arr[static_cast<size_t>(pivotIndex)],
        arr[static_cast<size_t>(hi)]
    );
    int pivot = arr[static_cast<size_t>(hi)];
    long long i = lo;
    for (long long j = lo; j < hi; ++j) {
        if (arr[static_cast<size_t>(j)] < pivot) {
            swap(
                arr[static_cast<size_t>(i)],
                arr[static_cast<size_t>(j)]
            );
            ++i;
        }
    }
    swap(
        arr[static_cast<size_t>(i)],
        arr[static_cast<size_t>(hi)]
    );

    return i;
}


void quicksortRec(
    vector<int>& arr,
    long long lo,
    long long hi
) {
    constexpr long long INSERTION_LIMIT = 16;
    while (lo < hi) {
        // Para subarreglos pequeños, insertion sort
        if (hi - lo + 1 <= INSERTION_LIMIT) {

            insertionSort(arr, lo, hi);
            return;
        }

        long long p = partition(arr, lo, hi);

        long long leftSize = p - lo;
        long long rightSize = hi - p;

        // Recursionamos solamente sobre la particion mas pequeña y procesamos la grande mediante el while. Esto mantiene baja la profundidad de la pila
        if (leftSize < rightSize) {
            quicksortRec(arr, lo, p - 1);
            lo = p + 1;
        }
        else {
            quicksortRec(arr, p + 1, hi);
            hi = p - 1;
        }
    }
}

} // namespace


void sortArray(vector<int>& arr) {
    if (arr.size() < 2) {
        return;
    }
    quicksortRec(
        arr,
        0,
        static_cast<long long>(arr.size()) - 1
    );
}