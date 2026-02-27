#pragma once
#include "Pelicula.h"
#include "DatabaseManager.h"



// Estructura auxiliar para guardar en el Max-Heap
struct ResultadoPuntuado {
    string id;
    int score;

    // Sobrecargamos el operador < para que la Priority Queue sepa cómo ordenar.
    // Al retornar 'score < otro.score', le decimos a C++ que haga un Max-Heap 
    // es decir ,el puntaje mayor va a la cima.
    bool operator<(const ResultadoPuntuado& otro) const {
        return score < otro.score; 
    }
};

class SistemaRanking {
public:
    // Calcula la "Importancia" de una película respecto a la búsqueda
    int calcularImportancia(const string& query, const Pelicula& p) {
        int score = 0;
        
        // Asumimos que query, p.titulo y p.tagsRaw ya están en minúsculas 
        // gracias a nuestro pre-procesamiento anterior.
        
        // 1. Coincidencia exacta en título (Mayor importancia)
        if (p.titulo == query) {
            score += 100;
        } 
        // 2. Coincidencia parcial en título
        else if (p.titulo.find(query) != string::npos) {
            score += 50;
        }

        // 3. Frecuencia en los Tags (Term Frequency simple)
        size_t pos = p.tagsRaw.find(query, 0);
        while (pos != string::npos) {
            score += 30;
            pos = p.tagsRaw.find(query, pos + query.length());
        }

        // 4. Coincidencia en la SINOPSIS (menor peso)
        size_t posSinopsis = p.sinopsis.find(query, 0);
        while (posSinopsis != string::npos) {
            score += 5;
            posSinopsis = p.sinopsis.find(query, posSinopsis + query.length());
        }

        return score;
    }
};




// Paginador con Heap

class PaginadorResultados {
private:
    // priority_queue implementa internamente un Max-Heap en C++
    priority_queue<ResultadoPuntuado> maxHeap;
    DatabaseManager& db;

public:
    // Construir el Heap toma O(K * L) donde K es el número de IDs 
    // encontrados y L es la longitud del texto al calcular el score.
    PaginadorResultados(const unordered_set<string>& idsEncontrados, const string& query) 
        : db(DatabaseManager::getInstance()) 
    {
        SistemaRanking ranking;
        
        for (const string& id : idsEncontrados) {
            Pelicula p = db.obtenerPelicula(id); // O(1) gracias al Hash Table
            int score = ranking.calcularImportancia(query, p);
            
            // Insertamos en el Max-Heap: O(log K) por inserción
            maxHeap.push({id, score}); 
        }
    }

    // Extraer N elementos toma O(N log K)
    // Retorna los IDs mostrados para poder seleccionarlos
    vector<string> mostrarSiguientePagina(int cantidadElementos) {
        vector<string> idsMostrados;
        if (maxHeap.empty()) {
            cout << "--- No hay mas resultados para mostrar ---" << endl;
            return idsMostrados;
        }

        cout << "\n----- RESULTADOS ---" << endl;
        for (int i = 0; i < cantidadElementos && !maxHeap.empty(); i++) {
            ResultadoPuntuado top = maxHeap.top();
            maxHeap.pop(); 
            idsMostrados.push_back(top.id);

            Pelicula p = db.obtenerPelicula(top.id);
            cout << i + 1 << ". " << p.titulo << " (Score: " << top.score << ")" << endl;
        }
        cout << "------------------------------------------" << endl;
        return idsMostrados;
    }
    
    bool hayMasResultados() { return !maxHeap.empty(); }
};