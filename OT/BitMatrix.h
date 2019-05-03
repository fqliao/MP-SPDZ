/*
 * BitMatrix.h
 *
 */

#ifndef OT_BITMATRIX_H_
#define OT_BITMATRIX_H_

#include <vector>
#include <emmintrin.h>
#include <immintrin.h>

#include "BitVector.h"
#include "Tools/random.h"
#include "Tools/MMO.h"
#include "Math/gf2nlong.h"
#include "Math/gf2nshortsquare.h"

using namespace std;

union square128 {
    typedef int128 RowType;

    const static int N_ROWS = 128;
    const static int N_ROWS_ALLOCATED = 128;
    const static int N_COLUMNS = 128;
    const static int N_ROW_BYTES = 128 / 8;

    static size_t size() { return N_ROWS * sizeof(__m128i); }

#ifdef __AVX2__
    __m256i doublerows[64];
#endif
    __m128i rows[128];
    octet bytes[128][16];
    int16_t doublebytes[128][8];
    int32_t words[128][4];

    bool get_bit(int x, int y)
    { return (bytes[x][y/8] >> (y % 8)) & 1; }

    void set_zero();

    square128& operator^=(square128& other);
    square128& operator^=(const __m128i* other);
    square128& operator^=(BitVector& other);
    bool operator==(square128& other);

    template <class T>
    square128& add(square128& other);
    template <class T>
    square128& sub(square128& other);
    template <class T>
    square128& rsub(square128& other);
    template <class T>
    square128& sub(const __m128i* other);
    template <class T>
    square128& sub(const void* other) { return sub<T>((__m128i*)other); }

    void randomize(PRNG& G);
    template <class T>
    void randomize(int row, PRNG& G);
    template <class T>
    void conditional_add(BitVector& conditions, square128& other, int offset);
    void transpose();
    template <class T>
    void hash_row_wise(MMO& mmo, square128& input);
    template <class T>
    void to(T& result);

    void check_transpose(square128& dual, int i, int k);
    void check_transpose(square128& dual);
    void print(int i, int k);
    void print();
    void print_octets();
    void print_doublerows();

    // Pack and unpack in native format
    //   i.e. Dont care about conversion to human readable form
    void pack(octetStream& o) const;
    void unpack(octetStream& o);
};

// allocator to counter GCC bug
template <typename _Tp, int ALIGN>
class aligned_allocator : public std::allocator<_Tp>
{
public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef _Tp*       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef _Tp        value_type;

    template<typename _Tp1>
    struct rebind
    { typedef aligned_allocator<_Tp1, ALIGN> other; };

    _Tp*
    allocate(size_t __n, const void* = 0)
    {
        if (__n > this->max_size())
            std::__throw_bad_alloc();

        _Tp* res = 0;
        int err = posix_memalign((void**)&res, ALIGN, __n * sizeof(_Tp));
        if (err != 0 or res == 0)
            std::__throw_bad_alloc();
        return res;
    }

    void
    deallocate(pointer __p, size_type)
    {
        free(__p);
    }
};

template <class U>
class Slice;

template <class U>
class Matrix
{
public:
    typedef U PartType;

    vector< U, aligned_allocator<U, 32> > squares;

    void resize_vertical(int length) { squares.resize(DIV_CEIL(length, U::N_ROWS)); }

    template <class V>
    Matrix<U>& operator=(const Matrix<V>& other);

    bool operator==(Matrix<U>& other);
    bool operator!=(Matrix<U>& other);

    void randomize(PRNG& G);
    void randomize(int row, PRNG& G);
    void print_side_by_side(Matrix<U>& other);
    void print_conditional(BitVector& conditions);

    // Pack and unpack in native format
    //   i.e. Dont care about conversion to human readable form
    void pack(octetStream& o) const;
    void unpack(octetStream& o);

    void to(vector<BitVector>& output);
};

class BitMatrix : public Matrix<square128>
{
public:
    BitMatrix() {}
    BitMatrix(int length);

    __m128i operator[](int i) { return squares[i / 128].rows[i % 128]; }

    void resize(int length);
    int size();

    void transpose();
    void check_transpose(BitMatrix& dual);
};

template <class U>
class Slice
{
    friend U;

    U& bm;
    size_t start, end;

public:
    Slice(U& bm, size_t start, size_t size);

    template <class T>
    Slice<U>& rsub(Slice<U>& other);
    template <class T>
    Slice<U>& sub(BitVector& other, int repeat = 1);

    template <class T>
    void randomize(int row, PRNG& G);
    template <class T>
    void conditional_add(BitVector& conditions, U& other, bool useOffset = false);
    void transpose();

    template <class T>
    void print();

    // Pack and unpack in native format
    //   i.e. Dont care about conversion to human readable form
    void pack(octetStream& o) const;
    void unpack(octetStream& o);
};

typedef Slice<BitMatrix> BitMatrixSlice;

#endif /* OT_BITMATRIX_H_ */
