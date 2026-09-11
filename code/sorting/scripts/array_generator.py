import numpy as np
import os
from itertools import product

def generar_arreglo(n, tipo, dominio):
    """Genera un arreglo de largo n segun el tipo y dominio especificados
    - tipo: 'ascendente', 'descendente', 'aleatorio'
    - dominio: 'D1' (valores en {0,...,9}) o 'D7' (valores en {0,...,10^7})
    """
    if dominio == 'D1':
        low, high = 0, 9
    elif dominio == 'D7':
        low, high = 0, 10**7
    else:
        raise ValueError("Dominio no valido. Usa 'D1' o 'D7'.")

    base = np.random.randint(low, high + 1, size=n)

    if tipo == 'aleatorio':
        arr = base
    elif tipo == 'ascendente':
        arr = np.sort(base)
    elif tipo == 'descendente':
        arr = np.sort(base)[::-1]
    else:
        raise ValueError("Tipo no valido. Usa 'ascendente', 'descendente' o 'aleatorio'.")

    return arr

def guardar_arreglo(arr, nombre_archivo):
    """Guarda un arreglo en un archivo de texto, en una sola línea
    separada por espacios (mismo formato de fila usado para las
    matrices en la parte de multiplicación)"""
    with open(nombre_archivo, 'w') as f:
        f.write(' '.join(map(str, arr)) + '\n')

def generar_y_guardar(n, t, d, m, carpeta="../data/array_input"):
    """Genera un arreglo y lo guarda con el nombre formateado
    {n}_{t}_{d}_{m}.txt"""
    arr = generar_arreglo(n, t, d)

    base = f"{n}_{t}_{d}_{m}"
    archivo = os.path.join(carpeta, f"{base}.txt")

    guardar_arreglo(arr, archivo)

    print(f"Archivo guardado: {archivo}")

def generar_todos():
    Ns = [10**1, 10**3, 10**5, 10**7]
    Ts = ["ascendente", "descendente", "aleatorio"]
    Ds = ["D1", "D7"]
    Ms = ["a", "b", "c"]

    total = len(Ns) * len(Ts) * len(Ds) * len(Ms)
    print(f"Generando {total} archivos de arreglos...")

    for n, t, d, m in product(Ns, Ts, Ds, Ms):
        generar_y_guardar(n, t, d, m)

    print("Todos los arreglos han sido generados.")

if __name__ == "__main__":
    generar_todos()