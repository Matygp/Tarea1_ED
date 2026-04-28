#ifndef MOVING_IMG_H
#define MOVING_IMG_H

#include <stack>
#include <vector>
#include <string>
#include "basics.h"

//Enumeración para identificar los tipos de operaciones permitidas en el sistema
enum class OpType { LEFT, RIGHT, UP, DOWN, ROTATE };

//Estructura que almacena una acción específica: su tipo y la magnitud 
struct Action {
    OpType type;
    int dist; //se usa para píxeles de movimiento o como marcador especial para rotaciones Undo
};

class moving_image {
private:
    //Punteros para las matrices dinámicas de los tres canales de color RGB
    unsigned char **red_layer; 
    unsigned char **green_layer; 
    unsigned char **blue_layer; 

    //Estructuras de la STL para gestionar el historial y estados
    std::stack<Action> undo_stack;     //Pila LIFO para retroceder estados
    std::stack<Action> redo_stack;     //Pila LIFO para avanzar estados tras un undo
    std::vector<Action> full_history;  //Vector para registrar toda la secuencia cronológica (repeat_all)

    /**
     * execute_action: Ejecuta una acción almacenada sin registrarla nuevamente en las pilas.
     * Es vital para evitar bucles infinitos al llamar métodos desde undo, redo o repeat
     */
    void execute_action(Action act) {
        if (act.type == OpType::LEFT) move_left(act.dist, false);
        else if (act.type == OpType::RIGHT) move_right(act.dist, false);
        else if (act.type == OpType::UP) move_up(act.dist, false);
        else if (act.type == OpType::DOWN) move_down(act.dist, false);
        else if (act.type == OpType::ROTATE) {
            // Si el marcador es -1 significa que es un deshacer de rotación (requiere 3 giros)
            if (act.dist == -1) { 
                rotate(false); rotate(false); rotate(false);
            } else {
                rotate(false);
            }
        }
    }

    /**
     * _draw: Procesa las capas RGB y genera un archivo físico .png utilizando svpng
     */
    void _draw(const char* nb) {
        // Buffer temporal plano para almacenar los bytes en orden secuencial R-G-B
        unsigned char *rgb = new unsigned char[H_IMG * W_IMG * 3];
        unsigned char *p = rgb;
        unsigned x, y;

        FILE *fp = fopen(nb, "wb");

        //Bucle anidado para recorrer Filas (y) y Columnas (x) y extraer cada color
        for (y = 0; y < H_IMG; y++)
            for (x = 0; x < W_IMG; x++) {
                *p++ = red_layer[y][x];   //byte Rojo
                *p++ = green_layer[y][x]; //byte Verde
                *p++ = blue_layer[y][x];  //byte Azul
            }
        
        svpng(fp, W_IMG, H_IMG, rgb, 0); // Generación del archivo PNG
        fclose(fp);
        delete[] rgb; //Liberación del buffer temporal para evitar fugas de memoria
    }

    /**
     * reset: Limpia las matrices al color de fondo y redibuja a Mario en su posición inicial
     */
    void reset() {
        //Llena las matrices con el color crema por defecto definido en basics.h
        for(int i=0; i < H_IMG; i++)
            for(int j=0; j < W_IMG; j++) {
                red_layer[i][j] = DEFAULT_R;
                green_layer[i][j] = DEFAULT_G;
                blue_layer[i][j] = DEFAULT_B;
            }

        //Dibuja el sprite inicial leyendo las constantes s_R, s_G, s_B
        for(int i=0; i < 322; i++)
            for(int j=0; j < 256; j++) {
                // Solo dibuja si el píxel tiene información cromática (no transparente)
                if(s_R[i][j] || s_G[i][j] || s_B[i][j]) {
                    red_layer[INIT_Y+i][INIT_X+j] = s_R[i][j];
                    green_layer[INIT_Y+i][INIT_X+j] = s_G[i][j];
                    blue_layer[INIT_Y+i][INIT_X+j] = s_B[i][j];
                }
            }
    }

public:
    /**
     * Constructor: Reserva memoria dinámica para las matrices e inicializa la imagen.
     */
    moving_image() {
        red_layer = new unsigned char*[H_IMG];
        green_layer = new unsigned char*[H_IMG];
        blue_layer = new unsigned char*[H_IMG];
        
        for(int i=0; i < H_IMG; i++) {
            red_layer[i] = new unsigned char[W_IMG];
            green_layer[i] = new unsigned char[W_IMG];
            blue_layer[i] = new unsigned char[W_IMG];
        }
        reset(); //carga el estado por defecto
    }

    /**
     * Destructor: Libera sistemáticamente la memoria dinámica de cada fila y capa.
     */
    ~moving_image() {
        for(int i=0; i < H_IMG; i++) {
            delete[] red_layer[i];
            delete[] green_layer[i];
            delete[] blue_layer[i];
        }
        delete[] red_layer;
        delete[] green_layer;
        delete[] blue_layer;
    }

    //Interfaz pública para invocar el dibujado del PNG
    void draw(const char* nb) {
        _draw(nb);
    }

    /**
     * move_left: Desplaza d píxeles a la izquierda. Utiliza una capa temporal para 
     * gestionar la circularidad de la imagen.
     */
    void move_left(int d, bool save = true) {
        unsigned char **tmp_layer = new unsigned char*[H_IMG];
        for(int i=0; i < H_IMG; i++) tmp_layer[i] = new unsigned char[W_IMG];
        
        auto shift = [&](unsigned char** layer) {
            for(int i=0; i < H_IMG; i++) {
                //Desplazamiento lineal
                for(int j=0; j < W_IMG-d; j++) tmp_layer[i][j] = layer[i][j+d]; 
                //Cierre de la circularidad (lo que sale por la izquierda entra por la derecha)
                for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++) tmp_layer[i][j] = layer[i][k]; 
                //Actualización de la capa original
                for(int j=0; j < W_IMG; j++) layer[i][j] = tmp_layer[i][j];
            }
        };

        shift(red_layer); shift(green_layer); shift(blue_layer);

        //Liberación de memoria temporal usada para el cálculo
        for(int i=0; i < H_IMG; i++) delete[] tmp_layer[i];
        delete[] tmp_layer;

        //Registro de la acción si save es true 
        if (save) {
            undo_stack.push({OpType::LEFT, d});
            full_history.push_back({OpType::LEFT, d});
            while(!redo_stack.empty()) redo_stack.pop(); // Invalida el redo tras acción nueva
        }
    }

    /**
     * move_right: Desplaza d píxeles a la derecha optimizando con buffer por fila.
     */
    void move_right(int d, bool save = true) {
        d = d % W_IMG;
        if (d == 0) return;
        unsigned char* tmp_row = new unsigned char[W_IMG];
        
        auto shift_right = [&](unsigned char** layer) {
            for(int i = 0; i < H_IMG; i++) {
                for(int j = 0; j < d; j++) tmp_row[j] = layer[i][W_IMG - d + j];
                for(int j = d, k = 0; j < W_IMG; j++, k++) tmp_row[j] = layer[i][k];
                for(int j = 0; j < W_IMG; j++) layer[i][j] = tmp_row[j];
            }
        };
        shift_right(red_layer); shift_right(green_layer); shift_right(blue_layer);
        delete[] tmp_row;

        if (save) {
            undo_stack.push({OpType::RIGHT, d});
            full_history.push_back({OpType::RIGHT, d});
            while(!redo_stack.empty()) redo_stack.pop();
        }
    }

    /**
     * move_up: Desplaza hacia arriba. Optimización mediante reordenamiento de punteros de fila.
     */
    void move_up(int d, bool save = true) {
        d = d % H_IMG;
        if (d == 0) return;
        unsigned char** tmp_layer = new unsigned char*[H_IMG];
        
        auto s_up = [&](unsigned char** layer) {
            for(int i = 0; i < H_IMG - d; i++) tmp_layer[i] = layer[i + d];
            for(int i = H_IMG - d, k = 0; i < H_IMG; i++, k++) tmp_layer[i] = layer[k];
            for(int i = 0; i < H_IMG; i++) layer[i] = tmp_layer[i];
        };
        s_up(red_layer); s_up(green_layer); s_up(blue_layer);
        delete[] tmp_layer;

        if (save) {
            undo_stack.push({OpType::UP, d});
            full_history.push_back({OpType::UP, d});
            while(!redo_stack.empty()) redo_stack.pop();
        }
    }

    /**
     * move_down: Desplaza hacia abajo mediante reordenamiento de punteros de fila.
     */
    void move_down(int d, bool save = true) {
        d = d % H_IMG;
        if (d == 0) return;
        unsigned char** tmp_layer = new unsigned char*[H_IMG];
        
        auto s_down = [&](unsigned char** layer) {
            for(int i = 0; i < d; i++) tmp_layer[i] = layer[H_IMG - d + i];
            for(int i = d, k = 0; i < H_IMG; i++, k++) tmp_layer[i] = layer[k];
            for(int i = 0; i < H_IMG; i++) layer[i] = tmp_layer[i];
        };
        s_down(red_layer); s_down(green_layer); s_down(blue_layer);
        delete[] tmp_layer;

        if (save) {
            undo_stack.push({OpType::DOWN, d});
            full_history.push_back({OpType::DOWN, d});
            while(!redo_stack.empty()) redo_stack.pop();
        }
    }

    /**
     * rotate: Rota 90° anti-horario mapeando las coordenadas (i, j) a su nueva posición.
     */
    void rotate(bool save = true) {
        unsigned char** tmp_layer = new unsigned char*[H_IMG];
        for(int i = 0; i < H_IMG; i++) tmp_layer[i] = new unsigned char[W_IMG];
        
        auto rotate_c = [&](unsigned char** layer){
            for(int i = 0; i < H_IMG; i++)
                for(int j = 0; j < W_IMG; j++) tmp_layer[W_IMG - 1 - j][i] = layer[i][j];
            
            for(int i = 0; i < H_IMG; i++)
                for(int j = 0; j < W_IMG; j++) layer[i][j] = tmp_layer[i][j];
        };
        rotate_c(red_layer); rotate_c(green_layer); rotate_c(blue_layer);
        
        for(int i = 0; i < H_IMG; i++) delete[] tmp_layer[i];
        delete[] tmp_layer;

        if (save) {
            undo_stack.push({OpType::ROTATE, 0});
            full_history.push_back({OpType::ROTATE, 0});
            while(!redo_stack.empty()) redo_stack.pop();
        }
    }

    /**
     * undo: Retrocede la última acción. Registra el movimiento inverso en el historial global
     * para que la "película" de repeat_all muestre el retroceso.
     */
    void undo() {
        if (undo_stack.empty()) return;
        Action last = undo_stack.top(); 
        undo_stack.pop(); 
        redo_stack.push(last); 
        
        // Ejecución física del inverso sin guardar en la pila de deshacer (save=false)
        if (last.type == OpType::LEFT) {
            move_right(last.dist, false);
            full_history.push_back({OpType::RIGHT, last.dist});
        } 
        else if (last.type == OpType::RIGHT) {
            move_left(last.dist, false);
            full_history.push_back({OpType::LEFT, last.dist});
        } 
        else if (last.type == OpType::UP) {
            move_down(last.dist, false);
            full_history.push_back({OpType::DOWN, last.dist});
        } 
        else if (last.type == OpType::DOWN) {
            move_up(last.dist, false);
            full_history.push_back({OpType::UP, last.dist});
        } 
        else if (last.type == OpType::ROTATE) { 
            // 3 rotaciones anti-horarias para deshacer una única rotación (equiva a 90° horario)
            rotate(false); rotate(false); rotate(false); 
            //Se registra marcador -1 para que repeat_all genere solo UNA imagen tras el undo
            full_history.push_back({OpType::ROTATE, -1}); 
        }
    }

    /**
     * redo: Rehace una acción previamente deshecha por un undo.
     */
    void redo() {
        if (redo_stack.empty()) return;
        Action act = redo_stack.top(); 
        redo_stack.pop();
        execute_action(act); 
        undo_stack.push(act);
        full_history.push_back(act);
    }

    /**
     * repeat: Duplica la última acción del historial de deshacer.
     */
    void repeat() {
        if (undo_stack.empty()) return;
        Action last = undo_stack.top();
        execute_action(last); undo_stack.push(last);
        full_history.push_back(last);
    }

    /**
     * repeat_all: Resetea la imagen y aplica cada acción registrada en el historial global,
     * generando un archivo .png por cada paso.
     */
    void repeat_all() {
        reset(); 
        draw("historial_0.png");
        for(size_t i = 0; i < full_history.size(); i++) {
            execute_action(full_history[i]);
            std::string name = "historial_" + std::to_string(i+1) + ".png";
            draw(name.c_str());
        }
    }
};

#endif
