"""
plot_generator.py (sorting)

Lee todos los archivos data/measurements/<algoritmo>.csv generados por
sorting.cpp (uno por algoritmo: mergesort, quicksort, patiencesort,
sort) y genera graficos comparativos en data/plots/, en formato PNG.

Nota sobre quicksort: para n=10^7 omiten a veces por limite de tiempo (ver
data/measurements/quicksort_omitidos.txt), asi que sus curvas
simplemente no tendrán ese punto

Graficos generados:
  -resumen_tiempo.png / resumen_memoria.png: tiempo (y memoria) promedio vs n, promediando sobre tipo/dominio/muestra, un color por algoritmo, escala log-log.
  -tiempo_{tipo}_{dominio}.png: un grafico por cada combinacion de tipo (ascendente/descendente/aleatorio) y dominio (D1/D7), tiempo vs n, un color por algoritmo.

Uso: python3 plot_generator.py
(se asume que se corre desde code/sorting/scripts/, como hace el
makefile)
"""

import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

MEASUREMENTS_DIR = "../data/measurements"
PLOTS_DIR = "../data/plots"

ALGO_ORDER = ["mergesort", "quicksort", "patiencesort", "sort"]
ALGO_LABELS = {
    "mergesort": "Merge Sort",
    "quicksort": "Quick Sort",
    "patiencesort": "Patience Sort",
    "sort": "Sort",
}


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
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("n (largo del arreglo)")
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
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("n (largo del arreglo)")
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