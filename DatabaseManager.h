#pragma once
#include "Pelicula.h"

// 3. CONCURRENCIA Y SINGLETON (Gestor de Base de Datos)

class DatabaseManager {
private:
    // Hash table: id -> Pelicula (búsqueda rápida por clave)
    unordered_map<string, Pelicula> baseDeDatos;

    // Mutex: protege escrituras concurrentes sobre baseDeDatos (evita data races)
    mutex dbMutex;

    // Constructor privado: parte del patrón Singleton
    DatabaseManager() {}

    // Procesa un bloque de filas del CSV (se ejecuta en tareas async)
    void procesarLoteConcurrente(vector<vector<string>> loteFilas) {
        // Mapa local del lote: así trabajamos sin bloquear (más eficiente)
        unordered_map<string, Pelicula> loteProcesado;

        // Pre-procesamiento: todo a minúsculas para comparar/buscar sin importar mayúsculas
        auto aMinusculas = [](string& str) {
            transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
        };

        for (const auto& fila : loteFilas) {
            // Validamos: mínimo 6 columnas y saltamos el header
            if (fila.size() >= 6 && fila[0] != "imdb_id") {
                // Creamos la Pelicula con los campos del CSV
                Pelicula p = {fila[0], fila[1], fila[2], fila[3], fila[4], fila[5]};

                // Normalizamos texto para búsquedas/ranking
                aMinusculas(p.titulo);
                aMinusculas(p.sinopsis);
                aMinusculas(p.tagsRaw);

                // Guardamos en el mapa local
                loteProcesado[p.id] = p;
            }
        }

        // Zona crítica: aquí sí bloqueamos porque escribimos en la tabla global
        lock_guard<mutex> lock(dbMutex);
        for (const auto& par : loteProcesado) {
            baseDeDatos[par.first] = par.second;
        }
    }

public:
    // Singleton: evitamos copias (una sola instancia real)
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(const DatabaseManager&) = delete;

    // Punto de acceso global a la única instancia
    static DatabaseManager& getInstance() {
        static DatabaseManager instancia; // se crea una vez, de forma segura
        return instancia;
    }

    // Carga el CSV con parsing de comillas + procesamiento por lotes
    void cargarDesdeCSV(const string& ruta) {
        ifstream archivo(ruta);
        if (!archivo.is_open()) return;

        // Parsing manual: controlamos comas dentro de comillas
        char c;
        bool dentroComillas = false;
        string celda = "";
        vector<string> fila;

        // Lotes + tareas concurrentes
        vector<vector<string>> lote;
        vector<future<void>> hilos;

        cout << "1. Cargando CSV con hilos concurrentes ..." << endl;

        while (archivo.get(c)) {
            if (c == '"') {
                // Doble comilla dentro de un string: "" -> "
                if (dentroComillas && archivo.peek() == '"') { celda += '"'; archivo.get(c); }
                else { dentroComillas = !dentroComillas; }
            }
            else if (c == ',' && !dentroComillas) {
                // Fin de celda
                fila.push_back(celda);
                celda = "";
            }
            else if ((c == '\n' || c == '\r') && !dentroComillas) {
                // Fin de fila
                if (!celda.empty() || !fila.empty()) {
                    fila.push_back(celda);
                    lote.push_back(fila);
                    fila.clear();
                    celda = "";

                    // Cuando el lote llega a 1000 filas, lo mandamos a una tarea async
                    if (lote.size() >= 1000) {
                        hilos.push_back(async(launch::async,
                                              &DatabaseManager::procesarLoteConcurrente,
                                              this,
                                              lote));
                        lote.clear();
                    }
                }
            }
            else {
                // Si estamos dentro de comillas, un salto de línea se vuelve espacio
                if (dentroComillas && (c == '\n' || c == '\r')) celda += ' ';
                else if (c != '\r' && c != '\n') celda += c;
            }
        }

        // Última fila si quedó algo pendiente
        if (!celda.empty() || !fila.empty()) {
            fila.push_back(celda);
            lote.push_back(fila);
        }

        // Último lote
        if (!lote.empty()) {
            hilos.push_back(async(launch::async,&DatabaseManager::procesarLoteConcurrente,this,lote));
        }

        // Esperamos a que terminen todas las tareas
        for (auto& h : hilos) h.wait();

        archivo.close();
        cout << "   -> Carga completada. " << baseDeDatos.size()
             << " peliculas en Hash Table (O(1)).\n" << endl;
    }

    // Acceso de solo lectura a toda la base
    const unordered_map<string, Pelicula>& getBaseDeDatos() const { return baseDeDatos; }

    // Buscar por id (acceso rápido). Si no existe, retorna Pelicula vacía.
    Pelicula obtenerPelicula(const string& id) {
        if (baseDeDatos.find(id) != baseDeDatos.end()) return baseDeDatos[id];
        return Pelicula();
    }
};
