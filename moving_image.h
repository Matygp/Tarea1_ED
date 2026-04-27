#ifndef MOVING_IMG_H
#define MOVING_IMG_H

#include "basics.h"

// Clase que representa una imagen como una colección de 3 matrices siguiendo el
// esquema de colores RGB

class moving_image {
private:
  unsigned char **red_layer; // Capa de tonalidades rojas
  unsigned char **green_layer; // Capa de tonalidades verdes
  unsigned char **blue_layer; // Capa de tonalidades azules

public:
  // Constructor de la imagen. Se crea una imagen por defecto
  moving_image() {
    // Reserva de memoria para las 3 matrices RGB
    red_layer = new unsigned char*[H_IMG];
    green_layer = new unsigned char*[H_IMG];
    blue_layer = new unsigned char*[H_IMG];
    
    for(int i=0; i < H_IMG; i++) {
      red_layer[i] = new unsigned char[W_IMG];
      green_layer[i] = new unsigned char[W_IMG];
      blue_layer[i] = new unsigned char[W_IMG];
    }

    // Llenamos la imagen con su color de fondo
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++) {
	red_layer[i][j] = DEFAULT_R;
	green_layer[i][j] = DEFAULT_G;
	blue_layer[i][j] = DEFAULT_B;
      }

    // Dibujamos el objeto en su posición inicial
    for(int i=0; i < 322; i++)
      for(int j=0; j < 256; j++) {
	if(!s_R[i][j] && !s_G[i][j] && !s_B[i][j]) {
	  red_layer[INIT_Y+i][INIT_X+j] = DEFAULT_R;
	  green_layer[INIT_Y+i][INIT_X+j] = DEFAULT_G;
	  blue_layer[INIT_Y+i][INIT_X+j] = DEFAULT_B;
	} else {
	  red_layer[INIT_Y+i][INIT_X+j] = s_R[i][j];
	  green_layer[INIT_Y+i][INIT_X+j] = s_G[i][j];
	  blue_layer[INIT_Y+i][INIT_X+j] = s_B[i][j];
	}
      }   
  }

  // Destructor de la clase
  ~moving_image() {
    for(int i=0; i < H_IMG; i++) {
      delete red_layer[i];
      delete green_layer[i];
      delete blue_layer[i];
    }

    delete red_layer;
    delete green_layer;
    delete blue_layer;
  }

  // Función utilizada para guardar la imagen en formato .png
  void draw(const char* nb) {
    _draw(nb);
  }

  // Función que similar desplazar la imagen, de manera circular, d pixeles a la izquierda
  void move_left(int d) {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];
    
    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
	tmp_layer[i][j] = red_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = red_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j] = green_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = green_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j] = blue_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = blue_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	blue_layer[i][j] = tmp_layer[i][j];
  }

  void move_right(int d) {
    d = d % W_IMG;
    if (d == 0)
      return;

    unsigned char* tmp_row = new unsigned char[W_IMG];

    //Función para evitar repetir code en cada capa
    auto shift_right = [&](unsigned char** layer) {
      for(int i = 0; i<H_IMG; i++) {
        for(int j = 0; j < d; j++)
          tmp_row[j] = layer[i][W_IMG - d + j]; //copia últimos d elementos al inicio del buffer temp
        for(int j = d,k = 0; j < W_IMG; j++,k++)
          tmp_row[j] = layer[i][k]; //se copia el resto de los elementos
        for(int j=0; j < W_IMG; j++)
          layer[i][j] = tmp_row[j];
      }
    };
    shift_right(red_layer);
    shift_right(green_layer);
    shift_right(blue_layer);

    delete[] tmp_row;
  }

  void move_up(int d) {
    d = d % H_IMG;
    if (d == 0) return;

    unsigned char** tmp_layer = new unsigned char*[H_IMG];

    auto s_up = [&](unsigned char** layer) {
      //Reordena los punteros de las filas en vez de mover todos los pixeles
      for(int i = 0; i < H_IMG - d; i++)
        tmp_layer[i] = layer[i + d];
      for(int i = H_IMG - d, k = 0; i < H_IMG; i++,k++)
        tmp_layer[i] = layer[k];

      //Actualizar punteros originales
      for(int i = 0; i < H_IMG; i++)
        layer[i] = tmp_layer[i];
    };

    s_up(red_layer);
    s_up(green_layer);
    s_up(blue_layer);

    delete[] tmp_layer;
  }

  void move_down(int d) { //funcionamiento similar a move_up()
    d = d % H_IMG;
    if (d == 0) return;

    unsigned char** tmp_layer = new unsigned char*[H_IMG];

    auto s_down = [&](unsigned char** layer) {
      for(int i = 0; i < d; i++) 
        tmp_layer[i] = layer[H_IMG - d + i];
      for(int i = d, k = 0; i < H_IMG; i++, k++) 
        tmp_layer[i] = layer[k];

      for(int i = 0; i < H_IMG; i++) 
        layer[i] = tmp_layer[i];
    };

    s_down(red_layer);
    s_down(green_layer);
    s_down(blue_layer);

    delete[] tmp_layer;
  }

  void rotate() {
    //Reserva memoria para una capa temporak
    unsigned char** tmp_layer = new unsigned char*[H_IMG];
    for(int i = 0; i < H_IMG; i++) tmp_layer[i] = new unsigned char[W_IMG];

    auto rotate_c = [&](unsigned char** layer){
      //Mapeo matricial para 90° anti horario
      for(int i = 0; i < H_IMG; i++){
        for(int j = 0; j < W_IMG; j++){
          tmp_layer[W_IMG - 1 - j][i] = layer[i][j];
        }
      }
      //Volvemos la rotacion a la capa original
      for(int i = 0; i < H_IMG; i++){
        for(int j = 0; j < W_IMG; j++){
          layer[i][j] = tmp_layer[i][j];
        }
      }
    };

    rotate_c(red_layer);
    rotate_c(green_layer);
    rotate_c(blue_layer);

    //Liberamos la matriz temporal
    for(int i = 0; i < H_IMG; i++) delete[] tmp_layer[i];
    delete[] tmp_layer;
  }

private:
  // Función privada que guarda la imagen en formato .png
  void _draw(const char* nb) {
    //    unsigned char rgb[H_IMG * W_IMG * 3], *p = rgb;
    unsigned char *rgb = new unsigned char[H_IMG * W_IMG * 3];
    unsigned char *p = rgb;
    unsigned x, y;

    // La imagen resultante tendrá el nombre dado por la variable 'nb'
    FILE *fp = fopen(nb, "wb");

    // Transformamos las 3 matrices en una arreglo unidimensional
    for (y = 0; y < H_IMG; y++)
        for (x = 0; x < W_IMG; x++) {
            *p++ = red_layer[y][x];    /* R */
            *p++ = green_layer[y][x];    /* G */
            *p++ = blue_layer[y][x];    /* B */
        }
    // La función svpng() transforma las 3 matrices RGB en una imagen PNG 
    svpng(fp, W_IMG, H_IMG, rgb, 0);
    fclose(fp);
}

  
};

#endif
