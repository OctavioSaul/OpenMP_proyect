#include "common.h"
#ifndef VECTORIZED_MATRIX_VECTORIZED_H
#define VECTORIZED_MATRIX_VECTORIZED_H
#endif
typedef struct cell{
    int x;
    int y;
}
        cell;
class IDW{
public:

	void reset_Matrix(float **mat,  int ROWS,  int COLS, float val);
    void IDW_cost_dist(float req, int val_null, float **cost_dist, float **&final,float **&suma,int x_init,  int y_init,  int ROWS,  int COLS, float exp);
    int movimientos(int x, int y, float **mat, float **&final, map <int, cell> &map_opciones , int ROWS, int COLS, int val_null, int explorado);
private:
};