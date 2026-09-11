"""
plot_generator.py (matrix_multiplication)

Lee todos los archivos data/measurements/<algoritmo>.csv generados por
matrix_multiplication.cpp (uno por algoritmo: naive, strassen) y genera
gráficos comparativos en data/plots/, en formato PNG.

Graficos generados:
  -resumen_tiempo.png / resumen_memoria.png: tiempo (y memoria) promedio vs n, promediando sobre tipo/dominio/ muestra, un color por algoritmo, escala log-log.
  -tiempo_{tipo}_{dominio}.png: un grafico por cada combinacion de tipo (dispersa/diagonal/densa) y dominio (D0/D10), tiempo vs n, un color por algoritmo.

Uso: python3 plot_generator.py
(se asume que se corre desde code/matrix_multiplication/scripts/, como
hace el makefile)
"""

import os
import glob
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

MEASUREMENTS_DIR = "../data/measurements"
PLOTS_DIR = "../data/plots"

ALGO_ORDER = ["naive", "strassen"]
ALGO_LABELS = {
    "naive": "Naive",
    "strassen": "Strassen",
}


def formatear_eje_x_potencias_de_dos(ax, valores_n):
    """
    Fija la escala del eje x en base 2 y pone un tick exactamente en
    cada valor de n usado en esta tarea (2^4, 2^6, 2^8, 2^10), en vez
    de dejar que matplotlib use su base 10 por defecto (que no cae
    sobre esos puntos y deja los ticks desperdigados).

    Esto NO cambia la pendiente de las curvas en el gráfico log-log
    (O(n^3) sigue viéndose con pendiente 3 sin importar la base del
    logaritmo) — solo mejora la legibilidad de los ticks.
    """
    ax.set_xscale("log", base=2)
    ticks = sorted(valores_n)
    ax.set_xticks(ticks)
    ax.set_xticklabels([f"$2^{{{int(np.log2(n))}}}$" for n in ticks])


def cargar_datos():
    archivos = glob.glob(os.path.join(MEASUREMENTS_DIR, "*.csv"))
    if not archivos:
        raise SystemExit(
            f"No se encontraron archivos .csv en {MEASUREMENTS_DIR}. "
            "Corre primero 'make run' para generar las mediciones."
        )
    dfs = [pd.read_csv(f) for f in archivos]
    return pd.concat(dfs, ignore_index=True)


def plot_resumen(df):
    resumen = (
        df.groupby(["algoritmo", "n"])[["tiempo_ms", "memoria_bytes"]]
        .mean()
        .reset_index()
    )

    for metrica, ylabel, nombre_archivo in [
        ("tiempo_ms", "Tiempo promedio (ms)", "resumen_tiempo.png"),
        ("memoria_bytes", "Memoria auxiliar promedio (bytes)", "resumen_memoria.png"),
    ]:
        fig, ax = plt.subplots(figsize=(8, 5.5))
        for algo in ALGO_ORDER:
            sub = resumen[resumen["algoritmo"] == algo].sort_values("n")
            if sub.empty:
                continue
            ax.plot(sub["n"], sub[metrica], marker="o", label=ALGO_LABELS.get(algo, algo))
        formatear_eje_x_potencias_de_dos(ax, df["n"].unique())
        ax.set_yscale("log")
        ax.set_xlabel("n (dimensión de la matriz)")
        ax.set_ylabel(ylabel)
        ax.set_title(f"{ylabel} vs n\n(promedio sobre tipo/dominio/muestra)")
        ax.legend()
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        fig.tight_layout()
        fig.savefig(os.path.join(PLOTS_DIR, nombre_archivo), dpi=150)
        plt.close(fig)
        print(f"Generado: {nombre_archivo}")


def plot_por_tipo_dominio(df):
    combinaciones = df[["tipo", "dominio"]].drop_duplicates().values
    for tipo, dominio in combinaciones:
        sub_td = df[(df["tipo"] == tipo) & (df["dominio"] == dominio)]
        resumen = sub_td.groupby(["algoritmo", "n"])["tiempo_ms"].mean().reset_index()

        fig, ax = plt.subplots(figsize=(8, 5.5))
        for algo in ALGO_ORDER:
            sub = resumen[resumen["algoritmo"] == algo].sort_values("n")
            if sub.empty:
                continue
            ax.plot(sub["n"], sub["tiempo_ms"], marker="o", label=ALGO_LABELS.get(algo, algo))
        formatear_eje_x_potencias_de_dos(ax, df["n"].unique())
        ax.set_yscale("log")
        ax.set_xlabel("n (dimensión de la matriz)")
        ax.set_ylabel("Tiempo promedio (ms)")
        ax.set_title(f"Tiempo vs n — tipo={tipo}, dominio={dominio}")
        ax.legend()
        ax.grid(True, which="both", linestyle="--", alpha=0.4)
        fig.tight_layout()
        nombre_archivo = f"tiempo_{tipo}_{dominio}.png"
        fig.savefig(os.path.join(PLOTS_DIR, nombre_archivo), dpi=150)
        plt.close(fig)
        print(f"Generado: {nombre_archivo}")


def main():
    os.makedirs(PLOTS_DIR, exist_ok=True)
    df = cargar_datos()
    plot_resumen(df)
    plot_por_tipo_dominio(df)
    print("Listo.")


if __name__ == "__main__":
    main()