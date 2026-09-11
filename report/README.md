# AlgoReportTemplate

## Descripción

Este repositorio contiene una plantilla en **LaTeX** diseñada específicamente para la creación de mini-informes en la asignatura **Algoritmos y Complejidad** del Departamento de Informática de la **Universidad Técnica Federico Santa María (UTFSM)**. La plantilla está estructurada para facilitar la redacción de tareas, reportes de laboratorio y proyectos relacionados con el análisis de algoritmos.

### Modificación de la plantilla

- Escriba sus datos en `author.tex`.
- `preamble.tex` y `report.tex` NO debe ser modificados.
- `references.bib` contiene las referencias bibliográficas. Agregue las suyas.
- `report.pdf` es el archivo de salida generado por `pdflatex`.
- Elimine las figuras existentes y utilice las que generará en su informe.

```bash
.
├── author.tex
├── preamble.tex
├── README.md
├── references.bib
├── report.pdf
├── report.tex
└── sections
    ├── conclusions.tex
    ├── experiment_intro.tex
    └── experiment_results.tex

2 directories, 9 files

```

### Entrega

La entrega se realiza vía **aula.usm.cl** en formato `.zip`.

Resumen de Entregables

author.tex: Datos del autor
introduccion.tex: Introducción del informe
experiment_intro.tex: Entorno experimental detallado
experiment_results.tex: Resultados y análisis con gráficos
conclusions.tex: Conclusiones respaldadas por datos
references.bib: Referencias bibliográficas completas
README.md: Documentación del código y uso |

Instrucciones Finales

1. Compila el informe: (2 opciones)
opcion a:
   cd report
   pdflatex report.tex
   pdflatex report.tex (compilar 2 veces antes de abrirlo por favor)

opcion b:
   cd report
   latexmk -pdf informe.tex