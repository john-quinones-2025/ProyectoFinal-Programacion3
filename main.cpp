#include "Pelicula.h"
#include "SuffixTrie.h"
#include "MotorBusqueda.h"
#include "DatabaseManager.h"
#include "RankingPaginacion.h"
#include "UsuarioRecomendaciones.h"

int main() {
    ios_base::sync_with_stdio(false); cin.tie(NULL);

    cout << "======================================================" << endl;
    cout << "> Iniciando sistema..." << endl;
    
    // Cargando la base de datos
    DatabaseManager& db = DatabaseManager::getInstance();
    db.cargarDesdeCSV("mpst_full_data.csv");

    // Se construye el árbol
    cout << "> Construyendo Arbol de Busqueda..." << endl;
    MotorBusqueda motor;
    for (const auto& par : db.getBaseDeDatos()) {
        motor.indexarPelicula(par.second);
    }
    cout << "  -> Arbol construido con exito.\n" << endl;

    // Se construye el grafo
    GrafoRecomendaciones grafo;
    grafo.construirGrafo(db.getBaseDeDatos());
    cout << "======================================================\n" << endl;

    // Se crea el usuario
    Usuario miUsuario("Cinefilo");
    
    // Se ejecuta el bucle princiál de la interfaz
    bool ejecutando = true;
    while (ejecutando) {
        cout << "\n¡BIENVENIDO A LA PLATAFORMA DE STREAMING!" << endl;
        
        // Mostrar lista de películas guardadas en "Ver mas tarde"
        cout << "\n TU LISTA: VER MAS TARDE" << endl;
        vector<string> listaPendientes = miUsuario.obtenerListaVerMasTarde();
        if (listaPendientes.empty()) {
            cout << "   (No tienes peliculas pendientes)" << endl;
        } else {
            for (size_t i = 0; i < listaPendientes.size(); i++) {
                cout << "   " << i+1 << ". " << db.obtenerPelicula(listaPendientes[i]).titulo << endl;
            }
        }

        // Mostrar las películas recomendadas
        cout << "\n RECOMENDADOS PARA TI" << endl;
        vector<pair<string, int>> recomendaciones = grafo.recomendar(miUsuario.getGustadas(), 3);
        if (recomendaciones.empty()) {
            cout << "   (Dale Like a algunas peliculas para recibir recomendaciones)" << endl;
        } else {
            for (const auto& rec : recomendaciones) {
                cout << "   - " << db.obtenerPelicula(rec.first).titulo << endl;
            }
        }

        // Inicia el Menú principal dandote a elegir si buscar una película o terminar el programa
        cout << "\n========================================================" << endl;
        cout << "                    MENU PRINCIPAL" << endl;
        cout << "======================================================" << endl;
        cout << "1. Buscar pelicula (Palabra, Frase, Etiqueta o Fragmento)" << endl;
        cout << "2. Salir" << endl;
        cout << "------------------------------------------------------" << endl;
        cout << "> Ingrese una opcion: " << endl;
        
        string opcionMenu;
        cin >> opcionMenu;
        cin.ignore();

        // Si eliges salir, termina todo
        if (opcionMenu == "2") {
            ejecutando = false;
            cout << "\n¡Gracias por usar la plataforma! Hasta pronto." << endl;
            break;
        } 
        else if (opcionMenu == "1") {
            cout << "> Ingrese el texto a buscar: "<< endl;
            string query;
            getline(cin, query);

            auto resultadosBusqueda = motor.realizarBusqueda(query);
            cout << "\nBuscando '" << query << "'... ¡Encontradas " << resultadosBusqueda.size() << " coincidencias!" << endl;

            if (resultadosBusqueda.empty()) continue;

            PaginadorResultados paginador(resultadosBusqueda, query);
            bool viendoResultados = true;

            // Muestra las 5 peliculas que coincidan mejor con lo que buscaste
            while (viendoResultados) {
                vector<string> paginaActual = paginador.mostrarSiguientePagina(5);
                
                bool enMenuPaginacion = true;
                while (enMenuPaginacion) {
                    cout << "\n> Opciones: [1-" << paginaActual.size() << "] Ver detalle | ";
                    if (paginador.hayMasResultados()) cout << "[S] Siguientes 5 | ";
                    cout << "[V] Volver al menu principal" << endl;
                    cout << "> Ingrese opcion: "<< endl;
                    
                    string optResultados;
                    cin >> optResultados;
                    cin.ignore();

                    if (optResultados == "V" || optResultados == "v") {
                        viendoResultados = false;
                        enMenuPaginacion = false;
                    } 
                    else if ((optResultados == "S" || optResultados == "s") && paginador.hayMasResultados()) {
                        enMenuPaginacion = false;
                    }
                    else if (isdigit(optResultados[0])) {
                        int index = stoi(optResultados) - 1;
                        if (index >= 0 && index < paginaActual.size()) {

                            // En caso escojas ver alguna película, puedes ver los detalles de esta
                            Pelicula pSeleccionada = db.obtenerPelicula(paginaActual[index]);
                            
                            bool viendoDetalle = true;
                            while (viendoDetalle) {
                                cout << "\n======================================================" << endl;
                                cout << "                 DETALLE DE PELICULA" << endl;
                                cout << "======================================================" << endl;
                                cout << "TITULO: " << pSeleccionada.titulo << endl;
                                cout << "TAGS: " << pSeleccionada.tagsRaw << endl;
                                cout << "\nSINOPSIS:\n" << pSeleccionada.sinopsis << endl;
                                cout << "------------------------------------------------------" << endl;
                                cout << "¿Que deseas hacer?" << endl;
                                cout << "[L] Dar Like" << endl;
                                cout << "[T] Anadir a 'Ver mas tarde'" << endl;
                                cout << "[V] Volver a los resultados" << endl;
                                cout << "> Ingrese opcion: " << endl;
                                
                                string optDetalle;
                                cin >> optDetalle;
                                cin.ignore();

                                if (optDetalle == "L" || optDetalle == "l") {
                                    miUsuario.agregarLike(pSeleccionada.id);
                                } else if (optDetalle == "T" || optDetalle == "t") {
                                    miUsuario.agregarVerMasTarde(pSeleccionada.id);
                                } else if (optDetalle == "V" || optDetalle == "v") {
                                    viendoDetalle = false;
                                } else {
                                    cout << "Opcion invalida." << endl;
                                }
                            }
                            // Al salir del detalle, volvemos a imprimir las opciones de la página actual

                        // Finalmente lo de abajo son los casos en que se digite una respuesta inválida
                        } else {
                            cout << "Numero fuera de rango." << endl;
                        }
                    } else {
                        cout << "Opcion no reconocida." << endl;
                    }
                }
            }
        } else {
            cout << "Opcion no valida. Intente de nuevo." << endl;
        }
    }

    return 0;
}