#pragma once
#include "Pelicula.h"
#include "SuffixTrie.h"

// Patrón Strategy
// Nos permite cambiar el tipo de búsqueda solo títulos, global, etc
// rapido sin modificar el código central del motor.


// Interfaz base para cualquier tipo de búsqueda
class IBusquedaStrategy {
public:
    virtual unordered_set<string> buscar(const string& query) = 0;
    virtual ~IBusquedaStrategy() = default;
};


// Estrategia 1: Busca únicamente en los títulos
class BusquedaPorTituloStrategy : public IBusquedaStrategy {
private:
    SuffixTrie& indice;
public:
    BusquedaPorTituloStrategy(SuffixTrie& trie) : indice(trie) {}
    unordered_set<string> buscar(const string& query) override { return indice.buscar(query); }
};


// Estrategia 2: Busca en títulos y tags  a la vez
class BusquedaGlobalStrategy : public IBusquedaStrategy {
private:
    SuffixTrie& indTitulos;
    SuffixTrie& indTags;
public:
    BusquedaGlobalStrategy(SuffixTrie& titulos, SuffixTrie& tags) : indTitulos(titulos), indTags(tags) {}
    unordered_set<string> buscar(const string& query) override {
        unordered_set<string> res = indTitulos.buscar(query);
        unordered_set<string> tags = indTags.buscar(query);
        
        // Juntamos ambos resultados. El unordered_set ignora los duplicados automáticamente.
        res.insert(tags.begin(), tags.end());
        return res;
    }
};

class MotorBusqueda {
private:
    SuffixTrie indiceTitulos;
    SuffixTrie indiceTags;

    
    unique_ptr<IBusquedaStrategy> estrategiaActual;

    // Limpia el texto, quita puntuacion y guarda palabra por palabra en el arbol
    void procesarYAgregar(const string& texto, const string& id, SuffixTrie& trie) {
        // Limpiamos puntuación básica reemplazándola por espacios
        string textoLimpio = texto;
        replace_if(textoLimpio.begin(), textoLimpio.end(), ::ispunct, ' ');
        
        stringstream ss(textoLimpio);
        string palabra;
        while (ss >> palabra) {
            trie.insertarPalabra(palabra, id);
        }
    }

public:
    // Arranca con la búsqueda global por defecto
    MotorBusqueda() {
        estrategiaActual = make_unique<BusquedaGlobalStrategy>(indiceTitulos, indiceTags);
    }

    // Permite cambiar la estrategia de búsqueda en tiempo de ejecución
    void setEstrategia(unique_ptr<IBusquedaStrategy> nueva) {
        estrategiaActual = std::move(nueva);
    }

    // Prepara los datos de la película y los indexa en sus árboles correspondientes
    void indexarPelicula(const Pelicula& p) {
        procesarYAgregar(p.titulo, p.id, indiceTitulos);
        procesarYAgregar(p.tagsRaw, p.id, indiceTags);
    }

    // Pasa el texto a minúsculas y ejecuta la estrategia actual
    unordered_set<string> realizarBusqueda(string query) {
        transform(query.begin(), query.end(), query.begin(), ::tolower);
        return estrategiaActual->buscar(query);
    }
    
    SuffixTrie& getIndiceTitulos() { return indiceTitulos; }
};