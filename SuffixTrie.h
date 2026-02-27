#pragma once
#include "Pelicula.h"

// Utilizamos SuffixTrie ,porque permite buscar subcadenas muy rapido, en tiempo O(M). 


// Estructura del Nodo del arbol
struct TrieNode {

    // Usamos un map para ahorrar memoria (en lugar de un array fijo) y acceder en O(1)
    unordered_map<char, TrieNode*> hijos; 


    // Set para guardar los IDs de las películas sin repetirlos
    unordered_set<string> idsPeliculas;   
};




// Clase principal del motor de busqueda en memoria
class SuffixTrie {
private:
    TrieNode* raiz;

public:
    SuffixTrie() { raiz = new TrieNode(); }
 
    // Guarda la palabra y todos sus sufijos. Tiene complejidad O(L^2) donde L es su longitud.
    // Los dos bucles anidados generan y guardan cada fragmento posible.
    void insertarPalabra(const string& palabra, const string& idPelicula) {
        for (size_t i = 0; i < palabra.length(); i++) {
            TrieNode* actual = raiz;

            // Construye la rama letra por letra
            for (size_t j = i; j < palabra.length(); j++) {
                char letra = palabra[j];

                // Si la letra no tiene nodo, lo creamos
                if (actual->hijos.find(letra) == actual->hijos.end()) {
                    actual->hijos[letra] = new TrieNode();
                }

                // Avanzamos y vinculamos la película a este fragmento
                actual = actual->hijos[letra];
                actual->idsPeliculas.insert(idPelicula);
            }
        }
    }

    // Busca un texto exacto y devuelve las películas que lo contienen. Complejidad: O(M)
    unordered_set<string> buscar(const string& query) {
        TrieNode* actual = raiz;

        // Si el camino se corta, la palabra no está registrada
        for (char letra : query) {
            if (actual->hijos.find(letra) == actual->hijos.end()) {
                return unordered_set<string>(); 
            }
            actual = actual->hijos[letra];
        }

        // Retorna los IDs almacenados en el último nodo que alcanzamos
        return actual->idsPeliculas;
    }
};