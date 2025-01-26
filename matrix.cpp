#include <cstdio>

#include "matrix.hpp"

#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

template<int Rows, int Cols>
static void
set_matrix(Matrix<Rows, Cols> &A, int &acc)
{
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            A(i, j) = acc;
            acc += 1;
        }
    }
}

static void
add_test()
{
    Square_Matrix<2> a, b;
    int acc = 1;
    set_matrix(a, acc);
    set_matrix(b, acc);

    println("=== ADD ===");
    a.print();
    b.print();
    
    println("a + b");
    (a + b).print();
    
    println("b + a");
    (b + a).print();
}

static void
mul_test1()
{
    Matrix<3, 2> a;
    Matrix<2, 3> b;
    int acc = 1;
    set_matrix(a, acc);
    set_matrix(b, acc);

    println("=== MUL ===");
    a.print();
    b.print();
    
    println("a * b");
    (a * b).print();
    
    println("b * a");
    (b * a).print();
}

static void
mul_test2()
{
    Column_Vector<4> a;
    Row_Vector<4> b;
    int acc = 1;
    set_matrix(a, acc);
    set_matrix(b, acc);

    println("=== MUL ===");
    a.print();
    b.print();
    
    println("a * b");
    (a * b).print();
    
    println("b * a");
    (b * a).print();
}

int
main()
{
    add_test();
    mul_test1();
    mul_test2();
    return 0;
}

void
matrix_print(const int *data, int rows, int cols)
{
    printfln("%ix%i matrix:", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%i", data[matrix_index(i, j, cols)]);
            if (j < cols - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void
matrix_add(int *dst, const int *a, const int *b, int rows, int cols)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = matrix_index(i, j, cols);
            dst[idx] = a[idx] + b[idx];
        }
    }
}

// a[i, k], dim: m x n (rows x cols)
// b[k, j], dim: n x p (cols x rows)
// c[i, j]; dim: m x p (rows x rows)
void
matrix_mul(int *dst, const int *a, const int *b, int rows, int cols)
{
    int dots = cols;
    // Now referring to C's dimensions, not A's
    cols = rows;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int dot = 0;
            for (int k = 0; k < dots; k++) {
                dot += a[matrix_index(i, k, dots)] * b[matrix_index(k, j, rows)];
            }
            dst[matrix_index(i, j, cols)] = dot;
        }
    }
}

int
matrix_index(int i, int j, int cols)
{
    return i*cols + j;
}
