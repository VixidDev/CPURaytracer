#include <iostream>
#include <iomanip>
#include "Matrix4.h"
#include "Quaternion.h"
#include <limits>
#include <math.h>

// constructor - default to the zero matrix
Matrix4::Matrix4()
    { // default constructor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
    } // default constructor

Matrix4::Matrix4(const Matrix4 &other)
    { // copy constructor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = other.coordinates[row][col];
    } // copy constructor

// equality operator
bool Matrix4::operator ==(const Matrix4 &other) const
    { // operator ==()
    // loop through, testing for mismatches
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (abs(coordinates[row][col] - other.coordinates[row][col]) > std::numeric_limits<float>::epsilon())
                return false;
    // if no mismatches, matrices are the same
    return true;
    } // operator ==()


// indexing - retrieves the beginning of a line
// array indexing will then retrieve an element
float * Matrix4::operator [](const int rowIndex)
    { // operator *()
    // return the corresponding row
    return coordinates[rowIndex];
    } // operator *()

// similar routine for const pointers
const float * Matrix4::operator [](const int rowIndex) const
    { // operator *()
    // return the corresponding row
    return coordinates[rowIndex];
    } // operator *()

// scalar operations
// multiplication operator (no division operator)
Matrix4 Matrix4::operator *(float factor) const
    { // operator *()
    // start with a zero matrix
    Matrix4 returnMatrix;
    // multiply by the factor
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnMatrix.coordinates[row][col] = coordinates[row][col] * factor;
    // and return it
    return returnMatrix;
    } // operator *()

// vector operations on homogeneous coordinates
// multiplication is the only operator we use
Homogeneous4 Matrix4::operator *(const Homogeneous4 &vector) const
    { // operator *()
    // get a zero-initialised vector
    Homogeneous4 productVector;
    productVector.x += coordinates[0][0] * vector.x;
    productVector.x += coordinates[0][1] * vector.y;
    productVector.x += coordinates[0][2] * vector.z;
    productVector.x += coordinates[0][3] * vector.w;

    productVector.y += coordinates[1][0] * vector.x;
    productVector.y += coordinates[1][1] * vector.y;
    productVector.y += coordinates[1][2] * vector.z;
    productVector.y += coordinates[1][3] * vector.w;

    productVector.z += coordinates[2][0] * vector.x;
    productVector.z += coordinates[2][1] * vector.y;
    productVector.z += coordinates[2][2] * vector.z;
    productVector.z += coordinates[2][3] * vector.w;

    productVector.w += coordinates[3][0] * vector.x;
    productVector.w += coordinates[3][1] * vector.y;
    productVector.w += coordinates[3][2] * vector.z;
    productVector.w += coordinates[3][3] * vector.w;
    
    // return the result
    return productVector;
    } // operator *()

// and on Cartesian coordinates
Cartesian3 Matrix4::operator *(const Cartesian3 &vector) const
    { // cartesian multiplication
    // convert to Homogeneous coords and multiply
    Homogeneous4 productVector = (*this) * Homogeneous4(vector);

    // then divide back through
    return productVector.Point();
    } // cartesian multiplication

// matrix operations
// addition operator
Matrix4 Matrix4::operator +(const Matrix4 &other) const
    { // operator +()
    // start with a zero matrix
    Matrix4 sumMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            sumMatrix.coordinates[row][col] = coordinates[row][col] + other.coordinates[row][col];

    // return the result
    return sumMatrix;
    } // operator +()

// subtraction operator
Matrix4 Matrix4::operator -(const Matrix4 &other) const
    { // operator -()
    // start with a zero matrix
    Matrix4 differenceMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            differenceMatrix.coordinates[row][col] = coordinates[row][col] + other.coordinates[row][col];

    // return the result
    return differenceMatrix;
    } // operator -()

// multiplication operator
Matrix4 Matrix4::operator *(const Matrix4 &other) const
    { // operator *()
    // start with a zero matrix
    Matrix4 productMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            for (int entry = 0; entry < 4; entry++)
                productMatrix.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];

    // return the result
    return productMatrix;
    } // operator *()

// matrix transpose
Matrix4 Matrix4::transpose() const
    { // transpose()
    // start with a zero matrix
    Matrix4 transposeMatrix;
    
    // now loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            transposeMatrix.coordinates[row][col] = coordinates[col][row];

    // return the result
    return transposeMatrix;
    } // transpose()

// returns a column-major array of 16 values
// for use with OpenGL
columnMajorMatrix Matrix4::columnMajor() const
    { // columnMajor()
    // start off with an unitialised array
    columnMajorMatrix returnArray;
    // loop to fill in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnArray.coordinates[4 * col + row] = coordinates[row][col];
    // now return the array
    return returnArray;
    } // columnMajor()

// returns a column-major array of 16 values
// for use with OpenGL
rowMajorMatrix Matrix4::rowMajor() const
{ // rowMajor()
// start off with an unitialised array
    rowMajorMatrix returnArray;
    // loop to fill in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnArray.coordinates[4 * col + row] = coordinates[col][row];
    // now return the array
    return returnArray;
} // columnMajor()

// factory methods that create specific matrices
// the zero matrix
void Matrix4::SetZero()
    { // SetZero()
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
    } // SetZero()

// the identity matrix
void Matrix4::SetIdentity()
    { // SetIdentity()
    // start with a zero matrix
    SetZero();
    // fill in the diagonal with 1's
    for (int row = 0; row < 4; row++)
            coordinates[row][row] = 1.0;
    } // SetIdentity()

void Matrix4::SetTranslation(const Cartesian3 &vector)
    { // SetTranslation()
    // start with an identity matrix
    SetIdentity();

    coordinates[0][3] = vector.x;
    coordinates[1][3] = vector.y;
    coordinates[2][3] = vector.z;

    } // SetTranslation()

void Matrix4::SetRotation(const Cartesian3 &axis, float theta)
    { // SetRotation()
    // This is derived from quaternions, so we invoke them
    Quaternion rotationQuaternion(axis.unit(), theta * 0.5f);
    (*this) = rotationQuaternion.GetMatrix();
    } // SetRotation()

void Matrix4::SetScale(float xScale, float yScale, float zScale)
    { // SetScale()
    // start off with a zero matrix
    SetZero();

    // set the scale factors
    coordinates[0][0] = xScale;
    coordinates[1][1] = yScale;
    coordinates[2][2] = zScale;
    coordinates[3][3] = 1.0f;

    } // SetScale()

// scalar operations
// additional scalar multiplication operator
Matrix4 operator *(float factor, const Matrix4 &matrix)
    { // operator *()
    // since this is commutative, call the other version
    return matrix * factor;
    } // operator *()

// stream input
std::istream & operator >> (std::istream &inStream, Matrix4 &matrix)
    { // operator >>()
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            inStream >> matrix.coordinates[row][col];   
    // and return the stream
    return inStream;
    } // operator >>()

// stream output
std::ostream & operator << (std::ostream &outStream, const Matrix4 &matrix)
    { // operator <<()
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            outStream << std::setprecision(4) << std::setw(8) << matrix.coordinates[row][col] << ((col == 3) ? "\n" : " "); 
    // and return the stream
    return outStream;
    } // operator <<()
