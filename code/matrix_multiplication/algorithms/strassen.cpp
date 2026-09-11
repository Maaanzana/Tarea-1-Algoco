// strassen.cpp
//
// Multiplicación de matrices cuadradas, algoritmo de STRASSEN.
// Complejidad teórica: O(n^log2(7)) ≈ O(n^2.807) en tiempo (7
// multiplicaciones de submatrices en vez de 8, a costa de más sumas),
// O(n^2) en espacio auxiliar (submatrices en cada nivel de recursión).
//
// Caso base: bajo STRASSEN_THRESHOLD se cambia a multiplicación
// directa O(n^3) (como en implementaciones de referencia, ej. Eigen,
// BLAS), evitando el alto overhead de recursionar hasta 1x1.
//
// Tamaños no potencia de 2: se rellena  con ceros hasta la
// siguiente potencia de 2 y se recorta el resultado al final. Los
// tamaños de esta tarea (2^4, 2^6, 2^8, 2^10) ya son potencias de 2,
// así que el padding no debería activarse en las mediciones.
//
// Referencias / bibliografía:
// - Strassen, V. (1969). Gaussian elimination is not optimal. Numerische
//   Mathematik, 13(4), 354-356.
// - Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
//   Introduction to Algorithms (3rd ed.), Cap. 4.2 "Strassen's algorithm
//   for matrix multiplication". MIT Press.
// - GeeksforGeeks. Matrix Multiplication, Approach 3 (Strassen's Method).
//   Consultado en: https://www.geeksforgeeks.org/dsa/strassens-matrix-multiplication/
//   (mismas 7 formulas y mismo esquema de padding a potencia de 2).
//   Nota: la version de GfG recursiona hasta el caso base 1x1; aqui se
//   usa STRASSEN_THRESHOLD para cambiar antes a multiplicacion directa,
//   ya que GfG mismo señala que "Strassen's Method is often not
//   preferred in practice due to its high constant factors".

#include <vector>
#include <cstddef>
#include <algorithm>
using namespace std;

using Matrix = vector<vector<int>>;

namespace {

#ifdef STRASSEN_THRESHOLD_OVERRIDE
constexpr size_t STRASSEN_THRESHOLD = STRASSEN_THRESHOLD_OVERRIDE;
#else
constexpr size_t STRASSEN_THRESHOLD = 64;
#endif

Matrix makeMatrix(size_t n) {
    return Matrix(n, vector<int>(n, 0));
}

Matrix add(const Matrix& A, const Matrix& B) {
    size_t n = A.size();
    Matrix C = makeMatrix(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
}

Matrix subtract(const Matrix& A, const Matrix& B) {
    size_t n = A.size();
    Matrix C = makeMatrix(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
}

// Multiplicacion directa O(n^3), usada como caso base de la recursion
Matrix multiplyNaive(const Matrix& A, const Matrix& B) {
    size_t n = A.size();
    Matrix C = makeMatrix(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t k = 0; k < n; ++k) {
            int aik = A[i][k];
            for (size_t j = 0; j < n; ++j) {
                C[i][j] += aik * B[k][j];
            }
        }
    }
    return C;
}

void split(const Matrix& M, Matrix& M11, Matrix& M12, Matrix& M21, Matrix& M22) {
    size_t half = M.size() / 2;
    M11 = makeMatrix(half);
    M12 = makeMatrix(half);
    M21 = makeMatrix(half);
    M22 = makeMatrix(half);
    for (size_t i = 0; i < half; ++i) {
        for (size_t j = 0; j < half; ++j) {
            M11[i][j] = M[i][j];
            M12[i][j] = M[i][j + half];
            M21[i][j] = M[i + half][j];
            M22[i][j] = M[i + half][j + half];
        }
    }
}

Matrix join(const Matrix& C11, const Matrix& C12, const Matrix& C21, const Matrix& C22) {
    size_t half = C11.size();
    size_t n = half * 2;
    Matrix C = makeMatrix(n);
    for (size_t i = 0; i < half; ++i) {
        for (size_t j = 0; j < half; ++j) {
            C[i][j] = C11[i][j];
            C[i][j + half] = C12[i][j];
            C[i + half][j] = C21[i][j];
            C[i + half][j + half] = C22[i][j];
        }
    }
    return C;
}

Matrix strassenRec(const Matrix& A, const Matrix& B) {
    size_t n = A.size();

    if (n <= STRASSEN_THRESHOLD) {
        return multiplyNaive(A, B);
    }

    Matrix A11, A12, A21, A22, B11, B12, B21, B22;
    split(A, A11, A12, A21, A22);
    split(B, B11, B12, B21, B22);

    Matrix M1 = strassenRec(add(A11, A22), add(B11, B22));
    Matrix M2 = strassenRec(add(A21, A22), B11);
    Matrix M3 = strassenRec(A11, subtract(B12, B22));
    Matrix M4 = strassenRec(A22, subtract(B21, B11));
    Matrix M5 = strassenRec(add(A11, A12), B22);
    Matrix M6 = strassenRec(subtract(A21, A11), add(B11, B12));
    Matrix M7 = strassenRec(subtract(A12, A22), add(B21, B22));

    Matrix C11 = add(subtract(add(M1, M4), M5), M7);
    Matrix C12 = add(M3, M5);
    Matrix C21 = add(M2, M4);
    Matrix C22 = add(subtract(add(M1, M3), M2), M6);

    return join(C11, C12, C21, C22);
}

// Rellena A y B con ceros hasta la siguiente potencia de 2. No-op si n ya es potencia de 2 (caso de todos los tamaños usados en esta tarea)
size_t nextPowerOfTwo(size_t n) {
    size_t p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

Matrix pad(const Matrix& M, size_t newSize) {
    Matrix P = makeMatrix(newSize);
    for (size_t i = 0; i < M.size(); ++i) {
        for (size_t j = 0; j < M.size(); ++j) {
            P[i][j] = M[i][j];
        }
    }
    return P;
}

} // namespace

vector<vector<int>> multiplyMatrices(
    const vector<vector<int>>& A,
    const vector<vector<int>>& B) {

    size_t n = A.size();
    size_t paddedSize = nextPowerOfTwo(n);

    if (paddedSize == n) {
        return strassenRec(A, B);
    }

    Matrix result = strassenRec(pad(A, paddedSize), pad(B, paddedSize));

    Matrix trimmed = makeMatrix(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            trimmed[i][j] = result[i][j];
        }
    }
    return trimmed;
}