/*
 * m x n matrix class
 */
#include <assert.h>

template <class DataType>  
class Matrix
{
private:
	DataType * data;
	unsigned int size[2];
	size_t mxn;

public:
	~Matrix()
	{
		delete [] data;
	}
	
	Matrix(unsigned int m, unsigned int n):
		data(NULL),
		mxn(0)
		{
			size[0] = m;
			size[1] = n;
			
			mxn = m*n;
			
			data = new DataType[mxn];
			
			assert(data != NULL);
		}
	
	// assign same data from another Matrix of the same datatype and same size
	Matrix& operator=(const Matrix& m)
	{
		assert(data[0] == m.size[0]);
		assert(data[1] == m.size[1]);
		
		const int n = mxn;
		for(int i=0; i<n; i++)
			data[i] = m.data[i];
		
		return *this;
	}

    unsigned int rows() const {
        return size[0];
    }

    unsigned int cols() const {
        return size[1];
    }
	
	// swap content with another Matrix of the same datatype and same size
	inline void swap(Matrix<DataType>& m)
	{
		assert(data[0] == m.size[0]);
		assert(data[1] == m.size[1]);
		
		std::swap(data, m.data);
	}
	
	// Access operator
	inline DataType& operator()(unsigned int ix, unsigned int iy) const
	{
		assert(ix<size[0]);
		assert(iy<size[1]);
		
		return data[iy*size[0] + ix];
	}
	
	// Access operator - read only mode
	inline const DataType& Read(unsigned int ix, unsigned int iy) const
	{
		assert(ix<size[0]);
		assert(iy<size[1]);
		
		return data[iy*size[0] + ix];
	}
	
	inline unsigned int getNumberOfElements() const
	{
		return mxn;
	}	
};

// A nice operator for the output (to write std::cout << matrix )
template <class DataType>
std::ostream& operator << (std::ostream& os, Matrix<DataType> const& m) {
    std::cout << "[";
    for(unsigned int i=0; i < m.rows(); ++i) {
        if(i > 0)
            std::cout << "," << std::endl;
        std::cout << "[";
        for(unsigned int j=0; j < m.cols(); ++j) {
            if(j > 0)
                std::cout << ", ";
            std::cout << m(i,j);
        }
        std::cout << "]";
    }
    std::cout << "]" << std::endl;
    return os;
}