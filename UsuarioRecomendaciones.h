#pragma once
#include "Pelicula.h"

// Utilizando las colas y la lógica First In y First Out, se crea el Usuario que puede almacenar
// películas en "Ver mas Tarde" además de películas a las cuales les dio LIKE
class Usuario {
private:
    string nombre;
    // Cola FIFO para que la primera que agregaste sea la primera que veas
    queue<string> verMasTarde;
    // Vector solo para almacenar a las que les dio LIKE
    vector<string> peliculasGustadas;

public:
    // Constructor
    Usuario(string _nombre) : nombre(_nombre) {}

    // Función para agregar la película a "Ver mas tarde"
    void agregarVerMasTarde(const string& idPelicula) {
        verMasTarde.push(idPelicula);
        cout << "Pelicula " << idPelicula << " agregada a 'Ver mas tarde'." << endl;
    }

    // Función para agregar la película a las que le dio LIKE
    void agregarLike(const string& idPelicula) {
        peliculasGustadas.push_back(idPelicula);
        cout << "Te ha gustado la pelicula " << idPelicula << "!" << endl;
    }

    // Devuelve y elimina la siguiente película a ver
    string obtenerProximaVerMasTarde() {
        if (verMasTarde.empty()) return "";
        string id = verMasTarde.front();
        verMasTarde.pop(); // La sacamos de la cola
        return id;
    }

    // Nos devuelve la lista con las peliculas en "Ver mas tarde"
    vector<string> obtenerListaVerMasTarde() {
        vector<string> lista;
        queue<string> copia = verMasTarde; // Copiamos para no afectar la original
        while(!copia.empty()){
            lista.push_back(copia.front());
            copia.pop();
        }
        return lista;
    }

    const vector<string>& getGustadas() const { return peliculasGustadas; }
    string getNombre() const { return nombre; }
};

// Utilizando grado bipartido y algoritmo BFS, se crea una estrategia para recomendar películas
// en base a las que le hemos dado LIKE
class GrafoRecomendaciones {
private:
    // Tag -> Peliculas
    unordered_map<string, vector<string>> adyacenciaTagAPeliculas;

    // Pelicula -> Tags
    unordered_map<string, vector<string>> adyacenciaPeliculaATags;

public:
    // Construye el grado bipartito
    void construirGrafo(const unordered_map<string, Pelicula>& baseDeDatos) {
        cout << "Construyendo Grafo de Recomendaciones (Peliculas <-> Tags)..." << endl;
        for (const auto& par : baseDeDatos) {
            const Pelicula& p = par.second;
            string id = p.id;
            
            // Separar los tags crudos en palabras individuales reemplazando las comas por espacios
            string tagsLimpios = p.tagsRaw;
            replace(tagsLimpios.begin(), tagsLimpios.end(), ',', ' ');
            stringstream ss(tagsLimpios);
            string tag;
            
            while (ss >> tag) {
                // Pasamos a minúscula para uniformidad
                transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
                
                // Creamos las aristas bidireccionales del grafo bipartito
                adyacenciaTagAPeliculas[tag].push_back(id);
                adyacenciaPeliculaATags[id].push_back(tag);
            }
        }
    }

    // Se crea el sistema de recomendaciones utilizando el algoritmo BFS de 2 niveles
    vector<pair<string, int>> recomendar(const vector<string>& idsGustadas, int topN) {
        if (idsGustadas.empty()) return {};

        // Mapa para contar cuántos "caminos" (tags compartidos) llevan a cada película
        unordered_map<string, int> conteoSimilitud;
        unordered_set<string> setGustadas(idsGustadas.begin(), idsGustadas.end());

        // Recorrido BFS de 2 Niveles
        for (const string& idOrigen : idsGustadas) {

            // En el nivel 1 vemos los nodos tag de la pelicula
            for (const string& tag : adyacenciaPeliculaATags[idOrigen]) {
                
                // Y en el nivel 2 vemos todas las películas que comparten el tag
                for (const string& idDestino : adyacenciaTagAPeliculas[tag]) {

                    // Finalmente evitamos que aparezcan películas que el usuario ya le dio LIKE
                    if (setGustadas.find(idDestino) == setGustadas.end()) {
                        conteoSimilitud[idDestino]++;
                    }
                }
            }
        }

        // Utilizamos Max_Heap para obtener las mejores coincidencias
        priority_queue<pair<int, string>> maxHeap;
        for (const auto& par : conteoSimilitud) {
            maxHeap.push({par.second, par.first}); // Ordena por la similitud (par.second)
        }

        vector<pair<string, int>> recomendaciones;
        for (int i = 0; i < topN && !maxHeap.empty(); i++) {

            recomendaciones.push_back({maxHeap.top().second, maxHeap.top().first});
            maxHeap.pop();
        }

        return recomendaciones;
    }
};