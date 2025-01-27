#ifndef MATRIX_HPP
#define MATRIX_HPP

// Row major index.
int
matrix_index(int i, int j, int cols);

// Low-level matrix printing function.
void
matrix_print(const int *a, int rows, int cols);

// Low-level matrix addition. Do NOT use manually!
void
matrix_add(int *dst, const int *a, const int *b, int rows, int cols);

// Low-level matrix multiplication. Do NOT use manually!
void
matrix_mul(int *dst, const int *a, const int *b, int rows, int cols);

// The Matrix class is a wrapper around a stack-allocated fixed-size array.
// It also provides a cleaner interface to the low-level `matrix_*` functions.
template<int Rows, int Cols>
struct Matrix {
    static constexpr int rows = Rows;
    static constexpr int cols = Cols;

    // 2D "subscripting" access. Read-only.
    int
    operator()(int i, int j) const
    {
        return this->data[i*this->cols + j];
    }

    // 2D "subscripting" access. Read-write.
    int &
    operator()(int i, int j)
    {
        return this->data[i*this->cols + j];
    }

    // Matrix addition. By virtue of template deduction, this will cause compile
    // errors if `other` is not of the same dimensions as `this`.
    Matrix<Rows, Cols>
    operator+(const Matrix<Rows, Cols> &other) const noexcept
    {
        Matrix<Rows, Cols> C;
        matrix_add(C.data, this->data, other.data, this->rows, this->cols);
        return C;
    }

    // Matrix multiplication. By virtue of template deduction, this will cause
    // compile errors if `other` does not have complementary dimensions to `this`.
    Matrix<Rows, Rows>
    operator*(const Matrix<Cols, Rows> &other) const noexcept
    {
        Matrix<Rows, Rows> C;
        matrix_mul(C.data, this->data, other.data, this->rows, this->cols);
        return C;
    }

    void
    print() const noexcept
    {
        matrix_print(this->data, this->rows, this->cols);
    }

    int data[Rows * Cols];
};

template<int Length>
using Square_Matrix = Matrix<Length, Length>;

template<int Length>
using Column_Vector = Matrix<Length, 1>;

template<int Length>
using Row_Vector = Matrix<1, Length>;

#endif // MATRIX_HPP
