#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <future>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <memory>
#include <queue>

using namespace std;

// Estructura base
struct Pelicula {

    // ID único: lo usamos como llave en la hash table para ubicar rápido la película.
    string id;

    // Título: sirve para match perfecto (titulo == query) y match parcial (cuando escriben solo una parte).
    string titulo;

    // Sinopsis: se muestra al usuario y también suma puntos si la query aparece aquí.
    string sinopsis;

    // Tags crudos del CSV
    string tagsRaw;

    // Metadatos del dataset
    string split;
    string source;
};