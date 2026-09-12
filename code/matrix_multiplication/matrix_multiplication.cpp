// matrix_multiplication.cpp
//
// Programa principal de mediciones de tiempo y memoria para los 2
// algoritmos de multiplicacion de matrices. Este mismo archivo se
// compila y enlaza (naive.cpp, strassen.cpp)
//
// El programa:
// -Recorre data/matrix_input/ buscando pares de archivos {n}_{t}_{d}_{m}_1.txt y {n}_{t}_{d}_{m}_2.txt (M1 y M2).
// -Para cada par, corre el algoritmo enlazado, repitiendo varias veces y promediando el tiempo.
// -Escribe la matriz resultado en data/matrix_output/{base}_out.txt
// -Agrega una fila a data/measurements/{algoritmo}.csv con:
//      algoritmo,n,tipo,dominio,muestra,repeticiones,tiempo_ms,memoria_bytes
//
// ---------------------------------------------------------------------
// Medicion de memoria y limite de tiempo.
// mismo mecanismo ya usado en sorting.cpp: leer PeakWorkingSetSize (el pico
// historico de memoria fisica residente del proceso, via GetProcessMemoryInfo)

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#endif

namespace fs = filesystem;
using Matrix = vector<vector<int>>;
// Definida en exactamente uno de naive.cpp / strassen.cpp (el que se enlaza con este archivo)
Matrix multiplyMatrices(const Matrix& A, const Matrix& B);

// Tiempo maximo permitido por caso.
constexpr unsigned long TIMEOUT_SECONDS = 3; //en realidad minutos
constexpr unsigned long TIMEOUT_MILLISECONDS = TIMEOUT_SECONDS * 1000 * 60;

namespace {
struct CaseInfo {
    long long n = 0;
    string tipo;
    string dominio;
    string muestra;
    string baseName;
};

// Espera nombres del tipo "{n}_{t}_{d}_{m}_1.txt"
bool parseFileName(
    const string& stem,
    CaseInfo& info,
    string& side
) {
    vector<string> parts;
    stringstream ss(stem);
    string token;

    while (getline(ss, token, '_')) {
        parts.push_back(token);
    }
    if (parts.size() != 5) {
        return false;
    }
    side = parts[4]; 
    if (side != "1" && side != "2") {
        return false;
    }
    try {
        info.n = stoll(parts[0]);
    }
    catch (...) {
        return false;
    }

    info.tipo = parts[1];
    info.dominio = parts[2];
    info.muestra = parts[3];
    info.baseName = parts[0] + "_" + parts[1] + "_" + parts[2] + "_" + parts[3];

    return true;
}

Matrix readMatrix(const fs::path& path, size_t n) {
    Matrix M(n, vector<int>(n));
    ifstream in(path);

    if (!in) {
        throw runtime_error(
            "No se pudo abrir el archivo: " + path.string()
        );
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (!(in >> M[i][j])) {
                throw runtime_error(
                    "Error leyendo la matriz desde " + path.string()
                );
            }
        }
    }
    return M;
}

void writeMatrix(const fs::path& path, const Matrix& M) {
    ofstream out(path);

    if (!out) {
        throw runtime_error(
            "No se pudo crear el archivo: " + path.string()
        );
    }

    for (const auto& row : M) {
        for (size_t j = 0; j < row.size(); ++j) {
            out << row[j];
            if (j + 1 < row.size()) {
                out << ' ';
            }
        }
        out << '\n';
    }
}

// Mas repeticiones para matrices chicas, menos (o ninguna extra) para las grandes, donde una sola multiplicacion ya toma tiempo apreciable.
int repetitionsFor(long long n) {
    if (n <= 32) return 10;
    if (n <= 128) return 5;
    if (n <= 512) return 2;
    return 1;
}

// Medición de memoria: 
// A base del pico historico del working set.
size_t getPeakProcessMemory() {

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc{};
    if (!GetProcessMemoryInfo(
            GetCurrentProcess(),
            &pmc,
            sizeof(pmc)
        )) {
        return 0;
    }

    return static_cast<size_t>(
        pmc.PeakWorkingSetSize
    );

#else

    return 0;

#endif
}

// procesar un solo caso (ejecutado por el proceso hijo, o directamente si no hay timeout)

bool processSingleCase(
    const fs::path& path1,
    const fs::path& path2,
    const fs::path& outputDir,
    const fs::path& resultPath
) {
    try {
        CaseInfo info;
        string side;
        if (!parseFileName(path1.stem().string(), info, side)) {
            cerr << "Nombre de archivo inválido: " << path1.filename() << '\n';
            return false;
        }

        const Matrix A = readMatrix(path1, static_cast<size_t>(info.n));
        const Matrix B = readMatrix(path2, static_cast<size_t>(info.n));

        const int repetitions = repetitionsFor(info.n);
        double totalMs = 0.0;
        Matrix lastResult;

        for (int r = 0; r < repetitions; ++r) {
            const auto start = chrono::steady_clock::now();

            Matrix C = multiplyMatrices(A, B);

            const auto end = chrono::steady_clock::now();
            totalMs += chrono::duration<double, milli>(end - start).count();

            if (r == repetitions - 1) {
                lastResult = move(C);
            }
        }
        const double averageMs = totalMs / static_cast<double>(repetitions);

        // Guardar matriz resultado.
        const fs::path outputPath = outputDir / (info.baseName + "_out.txt");
        writeMatrix(outputPath, lastResult);
        // Pico de memoria de todo el caso
        const size_t maxMemory = getPeakProcessMemory();
        // Guardar resultados temporales.
        // El proceso padre leera este archivo cuando el proceso hijo termine.

        ofstream result(resultPath);
        if (!result) {
            throw runtime_error(
                "No se pudo crear archivo temporal de resultados."
            );
        }

        result << repetitions << ' ' << fixed << setprecision(6) << averageMs << ' ' << maxMemory << '\n';
        return true;
    }
    catch (const exception& e) {
        cerr << "Error procesando " << path1.filename() << ": " << e.what() << '\n';
        return false;
    }
}
// escapar argumento de windoes
#ifdef _WIN32

string quoteWindowsArgument(const string& arg) {
    string result = "\"";

    for (char c : arg) {
        if (c == '"') {
            result += "\\\"";
        }
        else {
            result += c;
        }
    }

    result += "\"";
    return result;
}

// ejecutar un caso con timeout

bool runCaseWithTimeout(
    const string& algName,
    const fs::path& path1,
    const fs::path& path2,
    const fs::path& outputDir,
    const fs::path& resultPath,
    const string& executablePath
) {
    string commandLine = quoteWindowsArgument(executablePath) + " " + quoteWindowsArgument(algName) + " --single-case " + quoteWindowsArgument(path1.string()) + " " + quoteWindowsArgument(path2.string()) + " " + quoteWindowsArgument(outputDir.string()) + " " + quoteWindowsArgument(resultPath.string());
    vector<char> commandBuffer(commandLine.begin(), commandLine.end());
    commandBuffer.push_back('\0');

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};

    BOOL created = CreateProcessA(nullptr, commandBuffer.data(), nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startupInfo,&processInfo);

    if (!created) {
        cerr << "Error: no se pudo iniciar proceso para " << path1.filename() << ". Codigo: " << GetLastError() << '\n';
        return false;
    }

    const DWORD waitResult =
        WaitForSingleObject(processInfo.hProcess, TIMEOUT_MILLISECONDS);

    // Caso normal: termino antes del limite
    if (waitResult == WAIT_OBJECT_0) {

        DWORD exitCode = 1;
        GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        return exitCode == 0;
    }

    // Timeout
    if (waitResult == WAIT_TIMEOUT) {
        cout << " -> TIMEOUT (mas de " << TIMEOUT_SECONDS << " minutos)\n";

        TerminateProcess(processInfo.hProcess, 124);
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        error_code ec;
        fs::remove(resultPath, ec);

        return false;
    }

    // Error esperando.
    cerr << "Error esperando proceso para " << path1.filename() << '\n';

    TerminateProcess(processInfo.hProcess, 1);
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return false;
}

#endif
// leer resultados del proceso hijo

bool readResult(
    const fs::path& resultPath,
    int& repetitions,
    double& averageMs,
    size_t& maxMemory
) {
    ifstream result(resultPath);
    if (!result) {
        return false;
    }

    result >> repetitions >> averageMs >> maxMemory;

    return !result.fail();
}

} // namespace

int main(int argc, char** argv) {
    // modo hijo
    // Este modo procesa solamente un caso 

    if (argc >= 7 && string(argv[2]) == "--single-case") {

        const string algName = argv[1];
        const fs::path path1 = argv[3];
        const fs::path path2 = argv[4];
        const fs::path outputDir = argv[5];
        const fs::path resultPath = argv[6];

        const bool success = processSingleCase(path1, path2, outputDir, resultPath);

        return success ? 0 : 1;
    }

    // modo normal

    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <nombre_algoritmo>\n" << "Ejemplo: " << argv[0] << " naive\n";
        return 1;
    }

    const string algName = argv[1];

    const fs::path inputDir = "data/matrix_input";
    const fs::path outputDir = "data/matrix_output";
    const fs::path measurementsDir = "data/measurements";

    if (!fs::exists(inputDir)) {
        cerr << "No existe el directorio " << inputDir << " (¿estas corriendo el programa desde code/matrix_multiplication/?)\n";
        return 1;
    }

    try {
        fs::create_directories(outputDir);
        fs::create_directories(measurementsDir);
    }
    catch (const std::exception& e) {
        cerr << "Error creando directorios: " << e.what() << '\n';
        return 1;
    }

    const fs::path csvPath = measurementsDir / (algName + ".csv");
    ofstream csv(csvPath);

    if (!csv) {
        cerr << "Error: no se pudo crear " << csvPath << '\n';
        return 1;
    }

    csv << "algoritmo,n,tipo,dominio,muestra,repeticiones,tiempo_ms,memoria_bytes\n";

    const fs::path omittedPath = measurementsDir / (algName + "_omitidos.txt");
    ofstream omitted(omittedPath);

    int measuredCases = 0;
    int omittedCases = 0;

    // Obtener ruta del ejecutable actual

#ifdef _WIN32

    char executableBuffer[MAX_PATH];
    DWORD executableLength =
        GetModuleFileNameA(nullptr, executableBuffer, MAX_PATH);

    if (executableLength == 0) {
        cerr << "ERROR: no se pudo obtener la ruta del ejecutable.\n";
        return 1;
    }

    const string executablePath(executableBuffer, executableLength);

#else

    const string executablePath = argv[0];

#endif

    // Identificar los base names
    set<string> baseNames;

    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".txt") {
            continue;
        }

        CaseInfo dummyInfo;
        string dummySide;
        if (parseFileName(entry.path().stem().string(), dummyInfo, dummySide)) {
            baseNames.insert(dummyInfo.baseName);
        }
    }

    // Recorrer casos.

    for (const auto& base : baseNames) {

        const fs::path path1 = inputDir / (base + "_1.txt");
        const fs::path path2 = inputDir / (base + "_2.txt");

        if (!fs::exists(path1) || !fs::exists(path2)) {
            cerr << "Falta un archivo del par para " << base << ", se omite\n";
            continue;
        }

        CaseInfo info;
        string side;
        parseFileName(path1.stem().string(), info, side);

        cout << "[" << algName << "] " << base << " -> ejecutando";
        cout.flush();

        const fs::path resultPath =
            measurementsDir / (algName + "_temp_" + base + ".txt");

        {
            error_code ec;
            fs::remove(resultPath, ec);
        }

#ifdef _WIN32

        const bool success = runCaseWithTimeout(
            algName,
            fs::absolute(path1),
            fs::absolute(path2),
            fs::absolute(outputDir),
            fs::absolute(resultPath),
            executablePath
        );

#else

        // Fuera de Windows se ejecuta directamente, sin limite de tiempo 

        const bool success = processSingleCase(path1, path2, outputDir, resultPath);

#endif

        if (!success) {
            if (!fs::exists(resultPath)) {
                ++omittedCases;
                if (omitted) {
                    omitted
                        << base
                        << " -> omitido: supero el límite de "
                        << TIMEOUT_SECONDS
                        << " minutos.\n";
                }

                cout << "[" << algName << "] " << base << " -> OMITIDO\n";
                continue;
            }

            cerr << "\nError procesando " << base << '\n';

            error_code ec;
            fs::remove(resultPath, ec);

            return 1;
        }

        int repetitions = 0;
        double averageMs = 0.0;
        size_t maxMemory = 0;

        if (!readResult(resultPath, repetitions, averageMs, maxMemory)) {

            cerr << "\nERROR: no se pudieron leer los resultados de " << base << '\n';
            error_code ec;
            fs::remove(resultPath, ec);

            return 1;
        }

        csv << algName << ',' << info.n << ',' << info.tipo << ',' << info.dominio << ',' << info.muestra << ',' << repetitions << ',' << fixed << setprecision(6) << averageMs << ',' << maxMemory << '\n';
        ++measuredCases;

        {
            error_code ec;
            fs::remove(resultPath, ec);
        }

        cout << " -> " << fixed << setprecision(6) << averageMs << " ms, " << maxMemory << " bytes de memoria\n";
    }

    csv.close();
    if (omitted) {
        omitted.close();
    }

    cout << "\nListo (" << measuredCases << " casos medidos, " << omittedCases << " omitidos).\n";
    cout << "Resultados en: " << csvPath << '\n';

    return 0;
}