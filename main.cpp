//programa:juego de memoria con matrices de 4x4,6x6 y 8x8
//autores:brayan camilo lopez peña-julian andres salazar belilla
//fecha:7 de julio del 2025
#include <windows.h>//Incluye la API de Windows, proporcionando funciones para interactuar con el sistema (ventanas, mensajes, registros)
#include <vector>//Incluye la biblioteca estándar vector para usar arreglos dinámicos (contenedores de la STL).
#include <string>//Permite el manejo de cadenas de texto (std::string).
#include <cstdlib>//Proporciona funciones generales como rand(), srand(), exit(), etc.
#include <ctime>//Ofrece funciones relacionadas con el tiempo, como time() (usado comúnmente para inicializar generadores de números aleatorios).
#include <fstream>//Permite leer y escribir archivos en el sistema (ifstream, ofstream).
#include <algorithm>//Incluye funciones genéricas de algoritmos (ordenamiento, búsqueda, etc.), como std::sort() o std::find()
#include <sstream>//Facilita el manejo de streams de strings (std::stringstream), útil para convertir entre tipos o formatear texto.
#include <mmsystem.h>//Biblioteca multimedia de Windows para reproducir sonidos (PlaySound()), manejar timers, etc.
#pragma comment(lib, "winmm.lib")//Directiva del compilador para enlazar automáticamente con la librería winmm.lib (necesaria para las funciones de mmsystem.h).

using namespace std;

// Estados del juego
enum GameState { //El enum asigna un número a cada nombre (empezando desde 0)
    PORTADA_INICIO, 
    MENU_PRINCIPAL,// Pantalla inicial/portada del juego.
    SELECCION_DIFICULTAD,
    EN_JUEGO,
    TABLA_POSICIONES,
    INGRESO_NOMBRE,
    TABLA_POSICIONES_SELECCION,
    CONFIRMAR_RESET// Pantalla de confirmación para resetear los puntajes.
};

struct Puntaje {//estructura de puntaje
    string nombre;
    int intentos;
};

// Variables globales
vector<vector<int>> matrizReal, matrizVisible;// Matriz que almacena la solución oculta del juego (pares de cartas ordenadas aleatoriamente)
vector<Puntaje> puntajes;// Almacena los puntajes históricos del juego (nombre e intentos de cada jugador)
string nombreJugador, dificultad;
int dimension, intentos, paresEncontrados;
bool primeraCartaSeleccionada = false;// Bandera que indica si el jugador ya seleccionó la primera carta de un par
bool bloqueoInteraccion = false;// Bandera que bloquea la interacción durante animaciones o delays
int fila1, col1, fila2, col2;// Coordenadas (fila, columna) de la primera y segunda  carta seleccionada
HWND hwndMain;// Identificador de la ventana principal del juego (manejo de interfaz gráfica en Windows)permite manejarla
GameState estadoActual = PORTADA_INICIO;  // Define y establece el estado inicial del juego en PORTADA_INICIO (la primera pantalla que verá el jugador)
char nombreInput[50] = "";

void ReproducirSonido(const char* sonido) {
    PlaySound(NULL, NULL, 0); // Detener cualquier sonido previo
    PlaySound(TEXT(sonido), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
    //   - SND_ASYNC: No bloquea la ejecución del programa
//   - SND_FILENAME: El parámetro es una ruta de archivo
//   - SND_NODEFAULT: No reproduce sonido por defecto si falla
}
//hdc:Un lienzo virtual donde dibujas (botones, textos, imágenes).

void DibujarBoton(HDC hdc, int x, int y, int ancho, int alto, const char* texto, bool esSalir = false) {//// Función para dibujar un botón personalizado en una ventana de Windows
    RECT rect = {x, y, x + ancho, y + alto};
    HBRUSH hBrush = CreateSolidBrush(esSalir ? RGB(229, 143, 143) : RGB(187, 143, 229));//Crea un pincel con color personalizado:
    FillRect(hdc, &rect, hBrush);  //  Rellena el rectángulo con el color del pincel
    DeleteObject(hBrush);
    FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));//  Dibuja un borde negro alrededor del botón
    
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);//	Evita que el texto tenga un rectángulo de fondo.
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFont);
    DrawText(hdc, texto, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//  Dibuja el texto centrado en el botón
    DeleteObject(hFont);
}

void MostrarPortada(HDC hdc) {
	// Obtiene las dimensiones del área cliente de la ventana principal
    RECT rect;
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    int alto = rect.bottom - rect.top;

    HBRUSH hBrush = CreateSolidBrush(RGB(161, 23, 23));
    RECT rectFondo = {0, 0, ancho, alto};// Define área completa
    FillRect(hdc, &rectFondo, hBrush); // Rellena con el color
    DeleteObject(hBrush);
    
    // Texto "Bienvenidos"
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hFontTitulo = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, // Crea fuente para el título
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFontTitulo);
    
    RECT rectTitulo = {0, 50, ancho, 150};
    DrawText(hdc, "Bienvenidos al Juego de Memoria", -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
    
    // Dibujo ASCII 
    HFONT hFontAscii = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
    SelectObject(hdc, hFontAscii);
    
    SetTextColor(hdc, RGB(0,0,0));
    int asciiX = (ancho - 500) / 2;// Posiciona y dibuja cada línea del arte ASCII
    TextOut(hdc, asciiX, 140, "¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 152, "¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_______¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 164, "¶¶¶¶¶¶¶¶¶¶¶¶_________________________¶¶¶¶¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 176, "¶¶¶¶¶¶¶¶¶¶____¶¶______¶¶__¶¶_______¶¶____¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 188, "¶¶¶¶¶¶¶____¶¶¶¶_______¶¶¶¶¶¶________¶¶¶¶____¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 200, "¶¶¶¶¶___¶¶¶¶¶¶________¶¶¶¶¶¶¶_______¶¶¶¶¶¶¶___¶¶¶¶", 50);
    TextOut(hdc, asciiX, 212, "¶¶¶¶__¶¶¶¶¶¶¶¶¶______¶¶¶¶¶¶¶¶_______¶¶¶¶¶¶¶¶¶__¶¶¶", 50);
    TextOut(hdc, asciiX, 224, "¶¶¶_¶¶¶¶¶¶¶¶¶¶¶¶¶___¶¶¶¶¶¶¶¶¶¶¶___¶¶¶¶¶¶¶¶¶¶¶¶___¶", 50);
    TextOut(hdc, asciiX, 236, "¶¶_¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶__¶", 50);
    TextOut(hdc, asciiX, 248, "¶__¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_¶", 50);
    TextOut(hdc, asciiX, 260, "¶__¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_¶", 50);
    TextOut(hdc, asciiX, 272, "¶¶_¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_¶", 50);
    TextOut(hdc, asciiX, 284, "¶¶__¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶__¶", 50);
    TextOut(hdc, asciiX, 296, "¶¶¶__¶¶¶¶¶¶_____¶¶____¶¶¶¶¶¶¶____¶¶_____¶¶¶¶¶¶__¶¶", 50);
    TextOut(hdc, asciiX, 308, "¶¶¶¶___¶¶¶¶____________¶¶¶¶_____________¶¶¶¶___¶¶¶", 50);
    TextOut(hdc, asciiX, 320, "¶¶¶¶¶¶___¶¶¶____________¶¶_____________¶¶¶___¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 332, "¶¶¶¶¶¶¶¶___________________________________¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 344, "¶¶¶¶¶¶¶¶¶¶¶¶___________________________¶¶¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 356, "¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶_______________¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶", 50);
    TextOut(hdc, asciiX, 368, "¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶", 50);

    // Autores
    HFONT hFontAutores = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFontAutores);
    
    RECT rectAutores = {0, 400, ancho, 450};
    DrawText(hdc, "Autores: Brayan Lopez - Julian Belilla", -1, &rectAutores, DT_CENTER | DT_SINGLELINE);
    
    // Botones
    int btnWidth = 200;
    int btnHeight = 50;
    int espacioEntreBotones = 20;
    
    // Botón Empezar
    DibujarBoton(hdc, (ancho - btnWidth)/2, alto - 150 - btnHeight - espacioEntreBotones, 
                btnWidth, btnHeight, "EMPEZAR");
    
    // Botón Salir
    DibujarBoton(hdc, (ancho - btnWidth)/2, alto - 150, 
                btnWidth, btnHeight, "SALIR", true);
    
    // Limpieza
    DeleteObject(hFontTitulo);
    DeleteObject(hFontAscii);
    DeleteObject(hFontAutores);
}
//EstaSobreBoton verifica si un clic ocurrió dentro de un área rectangular
bool EstaSobreBoton(int x, int y, int btnX, int btnY, int ancho, int alto) {// Verifica si las coordenadas (x,y) están dentro del área de un botón
    return x >= btnX && x <= btnX + ancho && y >= btnY && y <= btnY + alto;
}
//IntToStr convierte números a texto (para mostrar puntajes o valores).
string IntToStr(int value) {//Convierte un entero a string
    stringstream ss;
    ss << value;
    return ss.str();
}

void ResetearJuego() {
    matrizReal.clear();// Limpia la matriz solución
    matrizVisible.clear();
    intentos = 0; // Reinicia contador de intentos
    paresEncontrados = 0;
    primeraCartaSeleccionada = false;
    bloqueoInteraccion = false;
}

void RevelarCarta(int fila, int col) {// Muestra el valor de una carta en la matriz visible
    matrizVisible[fila][col] = matrizReal[fila][col];
}

void OcultarCartasNoPareadas() {// Vuelve a ocultar cartas si no son un par coincidente
    if (matrizReal[fila1][col1] != matrizReal[fila2][col2]) {
        matrizVisible[fila1][col1] = -1;
        matrizVisible[fila2][col2] = -1;
    }
}
//
bool juegoTerminado() {// Verifica si todas las cartas han sido descubiertas (juego terminado)
    for (const auto& fila : matrizVisible) {// Recorre cada fila de la matrizVisible
        for (int c : fila) {
            if (c == -1) return false; // Si encuentra una carta oculta (-1), el juego no ha terminado
        }
    }
    return true;
}

void GuardarPuntaje() {//Guarda el puntaje del jugador en un archivo según la dificultad
    ofstream archivo(dificultad + ".txt", ios::app);// Abre el archivo en modo append"Un modo de apertura para archivos en C++ que añade contenido al final del archivo sin borrar lo existente." (agrega al final sin borrar contenido)
    if (archivo.is_open()) {
        archivo << nombreJugador << " " << intentos << endl;// Escribe el nombre y los intentos en el archivo
        archivo.close();
    }
    ReproducirSonido("victoria.wav");
}

void CrearMatrizAleatoria() {// Genera una matriz de pares de cartas con valores aleatorios
    int totalCartas = dimension * dimension;// Calcula el total de cartas
    int paresNecesarios = totalCartas / 2;
    vector<int> valores;// Almacena los valores de las cartas

    for (int i = 0; i < paresNecesarios; i++) {
        int valor = 1 + (rand() % 70);
        valores.push_back(valor);// Añade el valor dos veces (par)
        valores.push_back(valor);//push_back:Añade un elemento al final de un vector (arreglo dinámico)
    }

    random_shuffle(valores.begin(), valores.end());// Baraja los valores para aleatorizar su posición

    matrizReal.resize(dimension, vector<int>(dimension));// Redimensiona las matrices para la dificultad actual
    matrizVisible.resize(dimension, vector<int>(dimension, -1));

    int index = 0;// Llena la matrizReal con los valores barajados
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            if (index < valores.size()) {
                matrizReal[i][j] = valores[index++];// Asigna valor y avanza el índice
            }
        }
    }
}
// Reinicia las variables del juego y genera una nueva partida
void IniciarJuego() {
    intentos = 0;
    paresEncontrados = 0;
    primeraCartaSeleccionada = false;
    bloqueoInteraccion = false;// Habilita interacciones
    CrearMatrizAleatoria();
    //hwndmain: Windows  redibuja mi ventana específica.
    InvalidateRect(hwndMain, NULL, TRUE); // Genera nueva matriz de cartas
}
//
void ProcesarClic(int x, int y) {// Función que procesa el clic del mouse en el área de juego
    if (bloqueoInteraccion) return;// Si hay bloqueo de interacción  no hacer nada

    RECT rect;// Obtener dimensiones del área cliente de la ventana(para cuando se expande o achica la pestaña)
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    
    // Tamaño de carta 
    int tamanoCarta = 60; // Ancho/alto de cada carta en píxeles
    int margenX = (ancho - dimension * tamanoCarta) / 2;// Margen horizontal para centrar
    int margenY = 100;
    
    int col = (x - margenX) / tamanoCarta; // Calcular fila y columna de la carta clickeada
    int fila = (y - margenY) / tamanoCarta;
    
    if (fila >= 0 && fila < dimension && col >= 0 && col < dimension) { // Verificar si el clic está dentro del tablero
        if (matrizVisible[fila][col] == -1) {// Verificar si la carta está oculta (-1)
            RevelarCarta(fila, col); // Mostrar la carta clickeada
            ReproducirSonido("escoger.wav"); // Sonido al seleccionar
            InvalidateRect(hwndMain, NULL, TRUE);
            // Lógica de selección de pares
            if (!primeraCartaSeleccionada) {
                primeraCartaSeleccionada = true;
                fila1 = fila; // Guardar posición
                col1 = col;
            } else {// Segunda carta seleccionada
                bloqueoInteraccion = true;
                
                fila2 = fila; // Guardar posición
                col2 = col;
                intentos++;
                
                if (matrizReal[fila1][col1] == matrizReal[fila2][col2]) { // Verificar si las cartas coinciden
                    paresEncontrados++;
                    ReproducirSonido("par.wav");// Sonido de acierto
                    if (juegoTerminado()) {// Verificar si el juego terminó
                        GuardarPuntaje();
                       // - WM_USER+1 = Notificar que el jugador ganó.
                        PostMessage(hwndMain, WM_USER+1, 0, 0); //WM_USER es una constante de Windows que marca el inicio  de mensajes personalizados (desde 0x0400 en adelante).
                    }
                    bloqueoInteraccion = false;
                } else {
                    ReproducirSonido("error.wav");
                    SetTimer(hwndMain, 1, 500, NULL); // Temporizador para ocultar cartas
                }
                primeraCartaSeleccionada = false; // Reiniciar selección
            }
        }
    }
}
//
void MostrarTablero(HDC hdc) {// Función que dibuja el tablero de juego y la interfaz de usuario
    RECT rect;// Obtener las dimensiones del área cliente de la ventana principal
    GetClientRect(hwndMain, &rect);// Función que calcula el espacio disponible para dibujar:
    int ancho = rect.right - rect.left;// Escribe las medidas en 'rect'
 // Ancho usable (ej: 800 píxeles)
 // Alto usable (ej: 600 píxeles)
    int alto = rect.bottom - rect.top;
    
    // Tamaño de carta 60px
    int tamanoCarta = 60;
    int margenX = (ancho - dimension * tamanoCarta) / 2;
    int margenY = 100;
    
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    string info = "Jugador: " + nombreJugador + " - Dificultad: " + dificultad + // Crear cadena con información del juego
                 " - Intentos: " + to_string(intentos) + " - Pares: " + to_string(paresEncontrados) + "/" + to_string((dimension * dimension)/2);
    TextOut(hdc, 10, 10, info.c_str(), (int)info.length());
    
    DibujarBoton(hdc, ancho - 150, 10, 120, 30, "Volver"); // Dibujar botón "Volver" en la esquina superior derecha
    
    for (int i = 0; i < dimension; i++) {// Dibujar cada carta del tablero
        for (int j = 0; j < dimension; j++) {
            RECT rectCarta = {
                margenX + j * tamanoCarta,
                margenY + i * tamanoCarta,
                margenX + (j + 1) * tamanoCarta,
                margenY + (i + 1) * tamanoCarta
            };
            
            HBRUSH hBrush;// Seleccionar color de la carta según su estado
            if (matrizVisible[i][j] == -1) {
                hBrush = CreateSolidBrush(RGB(255, 255, 0));// Amarillo para cartas ocultas
            } else {
                hBrush = CreateSolidBrush(RGB(0, 255, 0));// Verde para cartas descubiertas
            }
            FillRect(hdc, &rectCarta, hBrush);// Rellenar la carta con el color seleccionado
            DeleteObject(hBrush);
            
            FrameRect(hdc, &rectCarta, (HBRUSH)GetStockObject(BLACK_BRUSH));
            
            if (matrizVisible[i][j] != -1) {// Si la carta está descubierta, mostrar su valor
                SetTextColor(hdc, RGB(0, 0, 0));
                SetBkMode(hdc, TRANSPARENT);
                string numero = IntToStr(matrizVisible[i][j]);
                DrawText(hdc, numero.c_str(), -1, &rectCarta, DT_CENTER | DT_VCENTER | DT_SINGLELINE); // Dibujar el número centrado en la carta
            }
        }
    }
}
//
void ResetearArchivosPuntajes() {// Función para borrar todos los puntajes guardados
    ofstream facil("FACIL.txt", ios::trunc);// Abre el archivo de dificultad FÁCIL en modo truncamiento (borra contenido)
    ofstream normal("NORMAL.txt", ios::trunc);
    ofstream dificil("DIFICIL.txt", ios::trunc);
    facil.close(); normal.close(); dificil.close(); // Cierra todos los archivos
}
// Función para mostrar la pantalla de confirmación de borrado de puntajes
void MostrarConfirmacionReset(HDC hdc) {
    RECT rect;
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    RECT rectTitulo = {0, 150, ancho, 200};
    HFONT hFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");// Crea y selecciona una fuente grande en negrita
    SelectObject(hdc, hFont);
    DrawText(hdc, "¿Borrar todos los puntajes?", -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFont);

    DibujarBoton(hdc, ancho/2 - 170, 250, 150, 50, "Sí");// Dibuja botones de confirmación (Sí/No) centrados horizontalmente
    DibujarBoton(hdc, ancho/2 + 20, 250, 150, 50, "No");
}

void MostrarIngresoNombre(HDC hdc) {// Función para mostrar la pantalla de ingreso de nombre
    RECT rect;
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    RECT rectTitulo = {0, 50, ancho, 100};
    HFONT hFontTitulo = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFontTitulo);
    string titulo = "INGRESE SU NOMBRE (Dificultad: " + dificultad + ")"; // Crea y dibuja el título con la dificultad actual
    DrawText(hdc, titulo.c_str(), -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFontTitulo);
    
    RECT rectCampo = {ancho/2 - 150, 150, ancho/2 + 150, 200};// Define y dibuja el campo de entrada de texto (rectángulo blanco con borde)
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rectCampo, hBrush);
    DeleteObject(hBrush);
    FrameRect(hdc, &rectCampo, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    SetTextColor(hdc, RGB(0, 0, 0));
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial"); // Crea y selecciona una fuente normal para el texto del nombre
    SelectObject(hdc, hFont);
    DrawText(hdc, nombreInput, -1, &rectCampo, DT_LEFT | DT_VCENTER | DT_SINGLELINE);// Dibuja el texto ingresado (nombreInput) en el campo
    DeleteObject(hFont);
    
    DibujarBoton(hdc, ancho/2 - 100, 250, 200, 50, "Comenzar Juego");// Dibuja botones de acción
    DibujarBoton(hdc, 50, 400, 150, 50, "Volver");
}
//
void MostrarTablaPosiciones(HDC hdc, const string& dificultadSeleccionada = "") {// Función para mostrar la tabla de puntajes o selección de dificultad
    RECT rect;// Obtener dimensiones del área cliente
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    
    if (dificultadSeleccionada.empty()) { // Si no se ha seleccionado dificultad, mostrar menú de selección
        SetTextColor(hdc, RGB(255, 255, 0));
        SetBkMode(hdc, TRANSPARENT);
        RECT rectTitulo = {0, 50, ancho, 100};
        HFONT hFontTitulo = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        SelectObject(hdc, hFontTitulo);
        DrawText(hdc, "SELECCIONE DIFICULTAD", -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);// Dibujar título centrado
        DeleteObject(hFontTitulo);

        DibujarBoton(hdc, ancho/2 - 100, 150, 200, 50, "Fácil (4x4)"); // Dibujar botones de dificultad (centrados)
        DibujarBoton(hdc, ancho/2 - 100, 220, 200, 50, "Normal (6x6)");
        DibujarBoton(hdc, ancho/2 - 100, 290, 200, 50, "Difícil (8x8)");
    } else {
        ifstream archivo(dificultadSeleccionada + ".txt");// Si hay dificultad seleccionada, mostrar puntajes
        vector<Puntaje> puntajes;// Leer puntajes desde archivo
        string nombre;
        int intentos;
        
        while (archivo >> nombre >> intentos) {// Cargar datos del archivo
            puntajes.push_back({nombre, intentos});
        }
        
        sort(puntajes.begin(), puntajes.end(), [](Puntaje a, Puntaje b) {// Ordenar puntajes por intentos (de menor a mayor)
            return a.intentos < b.intentos;
        });

        SetTextColor(hdc, RGB(255, 255, 0));
        SetBkMode(hdc, TRANSPARENT);
        RECT rectTitulo = {0, 50, ancho, 100};
        HFONT hFontTitulo = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        SelectObject(hdc, hFontTitulo);
        string titulo = "TOP 10 - " + dificultadSeleccionada;
        DrawText(hdc, titulo.c_str(), -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
        DeleteObject(hFontTitulo);

        SetTextColor(hdc, RGB(255, 255, 255));
        HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        SelectObject(hdc, hFont);

        if (puntajes.empty()) {
            RECT rectMsg = {0, 150, ancho, 200};
            DrawText(hdc, "No hay puntajes registrados", -1, &rectMsg, DT_CENTER);// Mostrar mensaje si no hay puntajes
        } else {
            int limite = min(10, (int)puntajes.size());// Mostrar top 10 puntajes
            for (int i = 0; i < limite; i++) {
                RECT rectPuntaje = {ancho/2 - 150, 150 + i * 30, ancho/2 + 150, 180 + i * 30};
                string texto = to_string(i + 1) + ". " + puntajes[i].nombre + " - " + 
                              to_string(puntajes[i].intentos) + " intentos";
                DrawText(hdc, texto.c_str(), -1, &rectPuntaje, DT_LEFT);
            }
        }
        DeleteObject(hFont);
    }
    DibujarBoton(hdc, 50, 400, 150, 50, "Volver");
}

void MostrarMenuPrincipal(HDC hdc) {// Función para mostrar el menú principal del juego
    RECT rect;
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left;
    
    DibujarBoton(hdc, ancho/2 - 100, 150, 200, 50, "Nuevo Juego"); // Dibujar botones del menú (centrados)
    DibujarBoton(hdc, ancho/2 - 100, 220, 200, 50, "Tabla de Posiciones");
    DibujarBoton(hdc, ancho/2 - 100, 290, 200, 50, "Resetear Puntajes");
    DibujarBoton(hdc, ancho/2 - 100, 360, 200, 50, "Salir");
    
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    RECT rectTitulo = {0, 50, ancho, 100};
    HFONT hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFont);
    DrawText(hdc, "JUEGO DE MEMORIA", -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFont);
}
//
void MostrarDificultad(HDC hdc) {// Función para mostrar la pantalla de selección de dificultad
    RECT rect;
    GetClientRect(hwndMain, &rect);
    int ancho = rect.right - rect.left; // Calcular ancho disponible
    
    DibujarBoton(hdc, ancho/2 - 100, 150, 200, 50, "Facil (4x4)");// Botónes para dificultad 
    DibujarBoton(hdc, ancho/2 - 100, 220, 200, 50, "Normal (6x6)");
    DibujarBoton(hdc, ancho/2 - 100, 290, 200, 50, "Dificil (8x8)");
    DibujarBoton(hdc, 50, 400, 150, 50, "Volver");
    
    SetTextColor(hdc, RGB(255, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    RECT rectTitulo = {0, 50, ancho, 100};
    HFONT hFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    SelectObject(hdc, hFont);
    DrawText(hdc, "SELECCIONE DIFICULTAD", -1, &rectTitulo, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFont);//se elimina la fuente para prevenir lentitud del programa
}
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            InvalidateRect(hwnd, NULL, TRUE);// Forzar repintado de toda la ventana
            break;
        }
        
        case WM_PAINT: {// Cuando se necesita pintar la ventana
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH hBrush = CreateSolidBrush(RGB(128, 0, 128));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
            
            switch (estadoActual) {// Mostrar la pantalla correspondiente al estado actual del juego
                case PORTADA_INICIO: MostrarPortada(hdc); break;
                case MENU_PRINCIPAL: MostrarMenuPrincipal(hdc); break;
                case SELECCION_DIFICULTAD: MostrarDificultad(hdc); break;
                case EN_JUEGO: MostrarTablero(hdc); break;
                case TABLA_POSICIONES: MostrarTablaPosiciones(hdc); break;
                case INGRESO_NOMBRE: MostrarIngresoNombre(hdc); break;
                case TABLA_POSICIONES_SELECCION: MostrarTablaPosiciones(hdc, dificultad); break;
                case CONFIRMAR_RESET: MostrarConfirmacionReset(hdc); break;
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_LBUTTONDOWN: {// Cuando se hace clic con el botón izquierdo del mouse
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            int ancho = rect.right - rect.left;
            int alto = rect.bottom - rect.top;
            
            int btnWidth = 200; // Dimensiones estándar para botones
            int btnHeight = 50;
            int espacioEntreBotones = 20;
            
            ReproducirSonido("escoger.wav"); // Sonido al hacer clic
            
            switch (estadoActual) {// Manejar clic según el estado actual del juego
                case PORTADA_INICIO:
                    // Botón Empezar
                    if (EstaSobreBoton(x, y, 
                                      (ancho - btnWidth)/2, 
                                      alto - 150 - btnHeight - espacioEntreBotones, 
                                      btnWidth, btnHeight)) {
                        estadoActual = MENU_PRINCIPAL;
                        InvalidateRect(hwnd, NULL, TRUE);// Actualizar pantalla
                    }
                    // Botón Salir
                    else if (EstaSobreBoton(x, y, 
                                          (ancho - btnWidth)/2, 
                                          alto - 150, 
                                          btnWidth, btnHeight)) {
                        ReproducirSonido("salir.wav");
                        if (MessageBox(hwnd, "¿Está seguro que desea salir?", "Confirmar salida", 
                                      MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            PostQuitMessage(0);// Cerrar aplicación
                        }
                    }
                    break;
                    
                case MENU_PRINCIPAL:
                    if (EstaSobreBoton(x, y, ancho/2 - 100, 150, 200, 50)) {
                        estadoActual = SELECCION_DIFICULTAD;
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 220, 200, 50)) {
                        estadoActual = TABLA_POSICIONES;
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 290, 200, 50)) {
                        estadoActual = CONFIRMAR_RESET;
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 360, 200, 50)) {
                        ReproducirSonido("salir.wav");
                        if (MessageBox(hwnd, "¿Está seguro que desea salir?", "Confirmar salida", 
                                      MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            PostQuitMessage(0);
                        }
                    }
                    break;
                    
                case CONFIRMAR_RESET:
                    if (EstaSobreBoton(x, y, ancho/2 - 170, 250, 150, 50)) {
                        ResetearArchivosPuntajes();
                        estadoActual = MENU_PRINCIPAL;
                    } else if (EstaSobreBoton(x, y, ancho/2 + 20, 250, 150, 50)) {
                        estadoActual = MENU_PRINCIPAL;
                    }
                    break;
                    
                case SELECCION_DIFICULTAD:
                    if (EstaSobreBoton(x, y, ancho/2 - 100, 150, 200, 50)) {
                        dificultad = "FACIL"; dimension = 4;
                        estadoActual = INGRESO_NOMBRE;
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 220, 200, 50)) {
                        dificultad = "NORMAL"; dimension = 6;
                        estadoActual = INGRESO_NOMBRE;
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 290, 200, 50)) {
                        dificultad = "DIFICIL"; dimension = 8;
                        estadoActual = INGRESO_NOMBRE;
                    } else if (EstaSobreBoton(x, y, 50, 400, 150, 50)) {
                        estadoActual = MENU_PRINCIPAL;
                    }
                    break;
                    
                case EN_JUEGO:
                    if (EstaSobreBoton(x, y, ancho - 150, 10, 120, 30)) {
                        estadoActual = MENU_PRINCIPAL;
                        ResetearJuego();
                    } else {
                        ProcesarClic(x, y);
                    }
                    break;
                    
                case TABLA_POSICIONES:
                    if (EstaSobreBoton(x, y, ancho/2 - 100, 150, 200, 50)) {
                        estadoActual = TABLA_POSICIONES_SELECCION;
                        dificultad = "FACIL";
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 220, 200, 50)) {
                        estadoActual = TABLA_POSICIONES_SELECCION;
                        dificultad = "NORMAL";
                    } else if (EstaSobreBoton(x, y, ancho/2 - 100, 290, 200, 50)) {
                        estadoActual = TABLA_POSICIONES_SELECCION;
                        dificultad = "DIFICIL";
                    } else if (EstaSobreBoton(x, y, 50, 400, 150, 50)) {
                        estadoActual = MENU_PRINCIPAL;
                    }
                    break;
                    
                case TABLA_POSICIONES_SELECCION:
                    if (EstaSobreBoton(x, y, 50, 400, 150, 50)) {
                        estadoActual = TABLA_POSICIONES;
                    }
                    break;
                    
                case INGRESO_NOMBRE:
                    if (EstaSobreBoton(x, y, ancho/2 - 100, 250, 200, 50)) {
                        nombreJugador = nombreInput;
                        if (nombreJugador.empty()) nombreJugador = "Jugador";
                        IniciarJuego();
                        estadoActual = EN_JUEGO;
                    } else if (EstaSobreBoton(x, y, 50, 400, 150, 50)) {
                        estadoActual = SELECCION_DIFICULTAD;
                        memset(nombreInput, 0, sizeof(nombreInput));
                    }
                    break;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        
        case WM_CHAR:
            if (estadoActual == INGRESO_NOMBRE) {// Cuando se presiona una tecla
                ReproducirSonido("teclado.wav");
                if (wParam == VK_BACK) { // Tecla Backspace
                    if (strlen(nombreInput) > 0) {
                        nombreInput[strlen(nombreInput) - 1] = '\0';
                    }
                } else if (wParam >= 32 && wParam <= 126 && strlen(nombreInput) < 49) {
                    char c = (char)wParam;
                    strncat(nombreInput, &c, 1);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
            
        case WM_TIMER: // Cuando expira un temporizador
            if (wParam == 1) {// Temporizador para ocultar cartas no pareadas
                KillTimer(hwnd, 1);
                OcultarCartasNoPareadas();
                bloqueoInteraccion = false;
                PostMessage(hwnd, WM_USER+2, 0, 0);// Notificar que no son pareja
                InvalidateRect(hwnd, NULL, TRUE);// Actualizar pantalla
            }
            break;
            
        case WM_USER+1: // Mensaje de victoria
            MessageBox(hwnd, "¡Felicidades! Has completado el juego.", "Juego Terminado", MB_OK);
            estadoActual = MENU_PRINCIPAL;
            ResetearJuego();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
            
        case WM_USER+2: // Mensaje de no pareja
            MessageBox(hwnd, "¡No es un par! Intenta de nuevo.", "Información", MB_OK | MB_ICONINFORMATION);
            break;
            
        case WM_DESTROY: // Cuando se cierra la ventana
            PostQuitMessage(0);// Salir de la aplicación
            break;
            
        default:// Para cualquier otro mensaje no manejado
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {// Función principal de una aplicación Windows (equivalente a main() en programas de consola)
    srand((unsigned int)time(NULL));
    
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };// Configurar la clase de ventana (plantilla para crear ventanas)
    wc.style = CS_HREDRAW | CS_VREDRAW;// Redibujar ventana al cambiar tamaño
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "MemoryGame";
     // Registrar la clase de ventana en Windows
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Error al registrar la clase de ventana", "Error", MB_ICONERROR);
        return 0;
    }
    // Crear la ventana principal usando la clase registrada
    hwndMain = CreateWindow("MemoryGame", "Juego de Memoria", WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_MAXIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hwndMain) {
        MessageBox(NULL, "Error al crear la ventana", "Error", MB_ICONERROR);// Verificar si la ventana se creó correctamente
        return 0;
    }
    
    ShowWindow(hwndMain, nCmdShow);// Mostrar y actualizar la ventana
    UpdateWindow(hwndMain);
    
    MSG msg;// Bucle principal de mensajes 
    while (GetMessage(&msg, NULL, 0, 0)) {// Obtener mensajes de Windows
        TranslateMessage(&msg);// Procesar teclas (convertir WM_KEYDOWN a WM_CHAR)
        DispatchMessage(&msg);// Enviar mensaje a WndProc para manejo
    }
    return (int)msg.wParam; // Devolver código de salida al terminar
}
