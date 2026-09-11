Tarea 1 - INF221 Algoritmos y Complejidad

Descripción

Este repositorio contiene la implementación en C++ de los algoritmos de ordenamiento y multiplicación de matrices solicitados en la Tarea 1, junto con los scripts de generación de datos mediante python, medición y generación de gráficos.

Para tanto Sorting como Matrix Mulyiplication se aplico un limite de tiempo de ejecucion de 3 minutos para que el programa se ejecute en un tiempo respetable y no se demore años en terminar.

Estructura

code/
|── matrix_multiplication/
|── sorting/
|── README.md (este mismo archivo)

Requisitos

- C++: compilador g++ (se usó la verisión 16.1.0).
- Python: Python 3.8+ con las librerías numpy, pandas y matplotlib.
- Make: GNU Make.
(para compilar todo se uso el MSYS2 MINGW64)

Instrucciones de Uso
(se navega entre los dos archivos, se regresa a code/ usando cd ..)
1. Generar datos de entrada

# Para aorting
cd code/sorting
make generate-data

# Para multiplicación de matrices
cd code/matrix_multiplication
make generate-data

2. Ejecutar  mediciones 

# Sorting
cd code/sorting
make run

# Multiplicación de matrices
cd code/matrix_multiplication
make run

3. Generar gráficos

# Sorting
cd code/sorting
make plot

# Multiplicación de matrices
cd code/matrix_multiplication
make plot

4. Limpiar archivos generados
# En ambos lados 
make clean 


Mediciones de Memoria
La memoria se mide mediante el pico histórico del Working Set (PeakWorkingSetSize) en Windows, capturado con GetProcessMemoryInfo. Esto permite obtener el máximo de memoria física utilizada por el proceso durante la ejecución del algoritmo, sin interferir con la asignación dinámica de memoria.

Quick Sort
Para n=10^7 con tipo ascendente/descendente, estos casos son omitidos y documentados en quicksort_omitidos.txt, ya que su ejecución no es posible en un tiempo respetable.

Referencias
Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). Introduction to Algorithms (3rd ed.). MIT Press.

Knuth, D. E. (1998). The Art of Computer Programming, Vol. 3: Sorting and Searching (2nd ed.). Addison-Wesley.

Strassen, V. (1969). Gaussian elimination is not optimal. Numerische Mathematik, 13(4), 354-356.

