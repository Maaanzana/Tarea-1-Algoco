// sorting.cpp
//
// Programa principal para realizar las mediciones de tiempo y memoria
// de los algoritmos de ordenamiento
//
// Algoritmos: Merge Sort, Quick Sort, Patience Sort, Sort

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
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
// Esta es la funcion implementada en cada algoritmo de los sort
void sortArray(vector<int>& arr);

// Tiempo maximo permitido por txt
constexpr unsigned long TIMEOUT_SECONDS = 3; //en realidad son minutos 
constexpr unsigned long TIMEOUT_MILLISECONDS =
    TIMEOUT_SECONDS * 60 * 1000;

// Informacion de cada caso
namespace {
struct CaseInfo {
    long long n = 0;
    string tipo;
    string dominio;
    string muestra;
    string baseName;
};

// Leer informacion del nombre del archivo
// Formato segun la tarea:{n}_{tipo}_{dominio}_{muestra}.txt

bool parseFileName(
    const string& stem,
    CaseInfo& info
) {
    vector<string> parts;
    stringstream ss(stem);
    string token;

    while (getline(ss, token, '_')) {
        parts.push_back(token);
    }
    if (parts.size() != 4) {
        return false;
    }
    try {
        info.n = stoll(parts[0]);
    }
    catch (...) {
        return false;
    }
    if (info.n < 0) {
        return false;
    }
    info.tipo = parts[1];
    info.dominio = parts[2];
    info.muestra = parts[3];
    info.baseName = stem;

    return true;
}

// Leer arreglo desde archivo
vector<int> readArray(
    const fs::path& path,
    size_t n
) {
    vector<int> arr(n);
    ifstream in(path);

    if (!in) {
        throw runtime_error("No se pudo abrir el archivo: " + path.string());
    }

    for (size_t i = 0; i < n; ++i) {
        if (!(in >> arr[i])) {
            throw runtime_error("Error leyendo el elemento " + to_string(i) + " desde " + path.string());
        }
    }
    return arr;
}

// Guardar arreglo ordenado
void writeArray(
    const fs::path& path,
    const vector<int>& arr
) {
    ofstream out(path);
    if (!out) {
        throw runtime_error("No se pudo crear el archivo: " + path.string());
    }
    for (size_t i = 0; i < arr.size(); ++i) {
        out << arr[i];
        if (i + 1 < arr.size()) {
            out << ' ';
        }
    }
    out << '\n';
}

// Cantidad de repeticiones
int repetitionsFor(long long n) {

    if (n <= 100) {
        return 20;
    }
    if (n <= 10'000) {
        return 10;
    }
    if (n <= 1'000'000) {
        return 3;
    }
    return 1;
}

// Verificar si el arreglo quedo ordenado
bool isSorted(
    const vector<int>& arr
) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }

    return true;
}

// Medición de memoria
// En Windows se utiliza el PICO historico del Working Set
// leer el pico una sola vez al final de processSingleCase captura el
// maximo de memoria física usada durante ese caso

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

// Convertir duracion a milisegundos
double nanosToMillis(
    chrono::steady_clock::duration duration
) {
    return chrono::duration<
        double,
        milli
    >(duration).count();
}

// Procesar un solo caso
// Esta función es ejecutada por el proceso hijo
// El proceso padre es el encargado de controlar el limite de 3 minutos
bool processSingleCase(
    const string& algName,
    const fs::path& inputPath,
    const fs::path& outputDir,
    const fs::path& resultPath
) {
    try {
        // Obtener informacion del caso
        CaseInfo info;
        if (!parseFileName(
                inputPath.stem().string(),
                info
            )) {
            cerr << "Nombre de archivo invalido: " << inputPath.filename() << '\n';
            return false;
        }
        // Leer arreglo original
        const vector<int> original = readArray(inputPath, static_cast<std::size_t>(info.n));
        // Cantidad de repeticiones
        const int repetitions = repetitionsFor(info.n);
        double totalTimeMs = 0.0;
        vector<int> finalArray;
        // Repeticiones
        for (int repetition = 0; repetition < repetitions; ++repetition) {
            // Copiar antes de medir
            vector<int> working = original;
            // Inicio cronometro
            const auto start = chrono::steady_clock::now();
            // Ejecutar algoritmo
            sortArray(working);
            // Fin cronometro
            const auto end = chrono::steady_clock::now();
            // Tiempo
            const double elapsedMs = nanosToMillis(end - start);
            totalTimeMs += elapsedMs;
            // Verificar ssorting
            if (!isSorted(working)) {
                cerr << "Error: " << algName << " no ordeno correctamente " << inputPath.filename() << '\n';
                return false;
            }
            // Guardar ultimo resultado
            if (repetition == repetitions - 1) {
                finalArray = move(working);
            }
        }
        // Promedio
        const double averageTimeMs = totalTimeMs / static_cast<double>(repetitions);
        // Pico de memoria de todo el caso 
        const size_t maxMemory = getPeakProcessMemory();

        // Archivo de salida
        const fs::path outputPath = outputDir / (algName + "_" + info.baseName + ".txt");
        writeArray(outputPath, finalArray);

        // Guardar resultados temporales
        // El proceso padre leera este archivo cuando el hijo termine
        ofstream result(resultPath);

        if (!result) {
            throw std::runtime_error("No se pudo crear archivo temporal de resultados.");
        }

        result << repetitions << ' ' << fixed << setprecision(6) << averageTimeMs << ' '<< maxMemory << '\n';
        result.close();

        return true;
    }
    catch (const exception& e) {
        cerr << "Error procesando " << inputPath.filename() << ": " << e.what() << '\n';
        return false;
    }
}
// Permite pasar correctamente rutas con espacios a CreateProcess
#ifdef _WIN32
string quoteWindowsArgument(
    const string& arg
) {
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

#endif

// ejecutar un caso en un proceso separado con límite de 3 minutos

#ifdef _WIN32

bool runCaseWithTimeout(
    const string& algName,
    const fs::path& inputPath,
    const fs::path& outputDir,
    const fs::path& resultPath,
    const string& executablePath
) {
    // Comando del proceso hijo
    string commandLine = quoteWindowsArgument(executablePath) + " " + quoteWindowsArgument(algName) + " --single-case " + quoteWindowsArgument(inputPath.string()) + " " + quoteWindowsArgument(outputDir.string()) + " " + quoteWindowsArgument(resultPath.string());
    // CreateProcess necesita un buffer modificable

    vector<char> commandBuffer(commandLine.begin(), commandLine.end());
    commandBuffer.push_back('\0');
    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};

    // Crear proceso hijo
    BOOL created = CreateProcessA(nullptr,commandBuffer.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startupInfo,&processInfo);
    if (!created) {
        cerr << "Error: no se pudo iniciar proceso para " << inputPath.filename() << ". Codigo: " << GetLastError() << '\n';
        return false;
    }
    // Esperar como maximo 3 minutos
    const DWORD waitResult = WaitForSingleObject(processInfo.hProcess,TIMEOUT_MILLISECONDS);

    // Caso normal: termino antes de 3 minutos
    if (waitResult == WAIT_OBJECT_0) {
        DWORD exitCode = 1;
        GetExitCodeProcess(processInfo.hProcess,&exitCode);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return exitCode == 0;
    }
    // Timeout
    if (waitResult == WAIT_TIMEOUT) {
        cout << " -> TIMEOUT (mas de " << TIMEOUT_SECONDS << " minutos)\n";
        // Matar proceso hijo
        TerminateProcess(
            processInfo.hProcess,
            124
        );

        // Esperar que realmente termine
        WaitForSingleObject(processInfo.hProcess,INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        // Eliminar resultado temporal
        error_code ec;
        fs::remove(resultPath,ec);

        return false;
    }
    // Error esperando
    cerr << "Error esperando proceso para "<< inputPath.filename() << '\n';

    TerminateProcess(processInfo.hProcess, 1);
    WaitForSingleObject(processInfo.hProcess,INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return false;
}

#endif
// Leer resultado del proceso hijo
bool readResult(
    const fs::path& resultPath,
    int& repetitions,
    double& averageTimeMs,
    size_t& maxMemory
) {
    ifstream result(resultPath);
    if (!result) {
        return false;
    }
    result >> repetitions >> averageTimeMs >> maxMemory;
    return !result.fail();
}

} // namespace

int main(int argc, char** argv) {

    // modo hijo
    // Este modo procesa solamente un archivo
    if (
        argc >= 6 &&
        string(argv[2]) == "--single-case"
    ) {
        const string algName = argv[1];
        const fs::path inputPath = argv[3];
        const fs::path outputDir = argv[4];
        const fs::path resultPath = argv[5];
        const bool success =processSingleCase(algName, inputPath, outputDir, resultPath);
        return success ? 0 : 1;
    }
    // modo normal
    if (argc < 2) {
        cerr << "Uso:\n" << "  ./bin/<algoritmo> <algoritmo>\n";
        return 1;
    }

    const string algName = argv[1];
    // Directorios
    const fs::path inputDir = "data/array_input";
    const fs::path outputDir = "data/array_output";
    const fs::path measurementsDir = "data/measurements";

    // Verificar entrada
    if (!fs::exists(inputDir)) {
        cerr << "Error: no existe el directorio de entrada: " << inputDir << '\n';
        return 1;
    }
    // Crear directorios
    try {
        fs::create_directories(outputDir);
        fs::create_directories(measurementsDir);
    }
    catch (const exception& e) {
        cerr << "Error creando directorios: " << e.what() << '\n';
        return 1;
    }
    // CSV (excel) de resultados
    const fs::path csvPath = measurementsDir / (algName + ".csv");
    ofstream csv(csvPath);
    if (!csv) {
        cerr << "Error: no se pudo crear " << csvPath << '\n';
        return 1;
    }
    csv << "algoritmo," << "n," << "tipo," << "dominio," << "muestra," << "repeticiones," << "tiempo_ms," << "memoria_bytes\n";

    // Archivo de omitidos
    const fs::path omittedPath = measurementsDir / (algName + "_omitidos.txt");
    ofstream omitted(omittedPath);
    int measuredCases = 0;
    int omittedCases = 0;
    // Obtener ruta del ejecutable actual
#ifdef _WIN32

    char executableBuffer[MAX_PATH];

    DWORD executableLength = GetModuleFileNameA(nullptr,executableBuffer,MAX_PATH);
    if (executableLength == 0) {
        cerr << "Error: no se pudo obtener la ruta del ejecutable.\n";
        return 1;
    }

    const string executablePath(executableBuffer,executableLength);

#else

    const string executablePath = argv[0];

#endif

    // Recorrer archivos
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        // Solo archivos regulares
        if (!entry.is_regular_file()) {
            continue;
        }
        // Solo .txt
        if (entry.path().extension() != ".txt") {
            continue;
        }
        // Informacion del caso
        CaseInfo info;
        if (!parseFileName(entry.path().stem().string(),info)) {
            cerr << "Advertencia: nombre ignorado: " << entry.path().filename() << '\n';
            continue;
        }
        // Mostrar caso
        cout << "[" << algName << "] " << info.baseName << " -> ejecutando";
        cout.flush();
        // Archivo temporal para recibir los resultados
        const fs::path resultPath = measurementsDir / (algName + "_temp_" + info.baseName + ".txt");

        // Eliminar resultado anterior por seguridad
        {
            error_code ec;
            fs::remove(resultPath, ec);
        }
        // Ejecutar proceso separado con limite de 3 minutos
#ifdef _WIN32

        const bool success = runCaseWithTimeout(algName, fs::absolute(entry.path()), fs::absolute(outputDir), fs::absolute(resultPath), executablePath);

#else
        // En Windows/MSYS64 se utiliza el codigo anterior
        // Para otros sistemas se ejecutaria directamente
        const bool success = processSingleCase(algName, entry.path(), outputDir, resultPath);

#endif

        // Si supero los 3 minutos
        if (!success) {
            // determinar si fue timeout
            // lo consideramos omitido para permitir continuar con el siguiente caso
            if (!fs::exists(resultPath)) {
                ++omittedCases;
                if (omitted) {
                    omitted << info.baseName << ".txt -> omitido: " << "supero el limite de " << TIMEOUT_SECONDS << " minutos.\n";
                }
                cout << "[" << algName << "] " << info.baseName << " -> Omitido\n";
                continue;
            }
            // Si existia resultado pero el proceso fallo, entonces fue un error real
            cerr << "\nError procesando " << info.baseName << '\n';
            error_code ec;
            fs::remove(resultPath, ec);
            return 1;
        }
        // Leer resultados
        int repetitions = 0;
        double averageTimeMs = 0.0;
        size_t maxMemory = 0;
        if (!readResult(resultPath,repetitions,averageTimeMs,maxMemory)) {
            cerr << "\nError: no se pudieron leer los resultados de " << info.baseName << '\n';
            error_code ec;
            fs::remove(resultPath, ec);
            return 1;
        }
        // Guardar CSV
        csv << algName << ',' << info.n << ',' << info.tipo << ',' << info.dominio << ',' << info.muestra << ',' << repetitions << ',' << fixed << setprecision(6) << averageTimeMs << ',' << maxMemory << '\n';
        ++measuredCases;
        // Eliminar resultado temporal
        {
            error_code ec;
            fs::remove(resultPath,ec);
        }
        // Mostrar resultado
        cout << " -> " << fixed << setprecision(6) << averageTimeMs << " ms, " << maxMemory << " bytes de memoria\n";
    }
    // Cerrar archivos
    csv.close();
    if (omitted) {
        omitted.close();
    }

    // Resumen para ver cuantoss se midieron y cuantos se omitieron
    cout << "\nListo (" << measuredCases << " casos medidos, " << omittedCases << " omitidos).\n";
    cout << "Resultados en: " << csvPath << '\n';
    return 0;
}