// naive.cpp
//
// Multiplicación de matrices cuadradas, método NAIVE (triple ciclo).
// Complejidad teórica: O(n^3) en tiempo, O(n^2) en espacio (resultado).
//
// Orden de ciclos i-k-j (no i-j-k): accede a las filas de B de forma
// secuencial en el ciclo interno, mejorando localidad de cache sin
// cambiar la complejidad asintotica.
//
// Referencias / bibliografía:
// - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
//   Introduction to Algorithms (3rd ed.), Cap. 4.2 "Strassen's algorithm
//   for matrix multiplication" (contexto del método naive como base de
//   comparación). MIT Press.
// - Aggarwal, A., & Vitter, J. S. (1988). The input/output complexity of
//   sorting and related problems. Communications of the ACM (motivación
//   del orden de ciclos i-k-j para localidad de caché).
// - GeeksforGeeks. Matrix Multiplication, Approach 1 (Nested Loops).
//   Consultado en: https://www.geeksforgeeks.org/dsa/strassens-matrix-multiplication/
//   Nota: la version de GfG usa orden de ciclos i-j-k; aqui se usa i-k-j
//   por la razon de localidad de cache explicada en la Nota (1).

#include <vector>
#include <cstddef>
using namespace std;

vector<vector<int>> multiplyMatrices(
    const vector<vector<int>>& A,
    const vector<vector<int>>& B) {

    size_t n = A.size();
    vector<vector<int>> C(n, vector<int>(n, 0));

    for (size_t i = 0; i < n; ++i) {
        for (size_t k = 0; k < n; ++k) {
            int aik = A[i][k];
            const vector<int>& bRow = B[k];
            vector<int>& cRow = C[i];
            for (size_t j = 0; j < n; ++j) {
                cRow[j] += aik * bRow[j];
            }
        }
    }

    return C;
}