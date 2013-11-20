#pragma once
template<typename T>
struct lightweight_vector
{
private:
	T * at;
	unsigned capacity;
public:
	unsigned length;
	unsigned getCapacity() const
	{
		return capacity;
	}
	unsigned size() const
	{
		return length;
	}
	lightweight_vector() : at( 0 ), length( 0 ), capacity( 0 )
	{
	}
	lightweight_vector( lightweight_vector const & source )
	{
		length = source.length;
		capacity = source.capacity;
		if( source.capacity )
		{
			at = new T[ source.capacity ];
			for( unsigned int i = 0; i < length; ++i )
				at[i] = source.at[i];
			return;
		}
		at = 0;
	}
	lightweight_vector & operator = ( lightweight_vector const & source )
	{
		if( at ) delete[] at;
		length = source.length;
		capacity = source.capacity;
		if( source.capacity > 0 )
		{
			at = new T[ source.capacity ];
			for( unsigned int i = 0; i <length; ++i )
				at[i] = source.at[i];
		}
		else
			at = 0;
		return *this;
	}
	~lightweight_vector()
	{
		if( at )
			delete[] at;
	}
	T & operator [] ( unsigned index )
	{
		return at[index];
	}
	const T & operator [] ( unsigned index ) const
	{
		return at[index];
	}
	void expand()
	{
		if( !capacity )
		{
			capacity = 4;
			at = new T[4];
		}
		else
		{
			capacity *= 2;
			T * n = new T[capacity];
			for(int i = length - 1; i >= 0; --i)
				n[i] = at[i];
			delete[] at;
			at = n;
		}
	}
	void resize( unsigned newSize )
	{
		while( capacity < newSize )
			expand();
		length = newSize;
	}
	void resize( unsigned newSize, T & newVal )
	{
		if( length < newSize )
			length = newSize;
		else
		{
			while( capacity < newSize )
				expand();
			while( length < newSize )
				at[ length++ ] = newVal;
		}
	}
	void resize( unsigned newSize, T newVal )
	{
		if( length > newSize )
			length = newSize;
		else
		{
			while( capacity < newSize )
				expand();
			while( length < newSize )
				at[ length++ ] = newVal;
		}
	}
	void clear_heavy() //heavy function to empty the vector and deallocate memory in the array
	{
		if( at )
			delete[] at;
		at = 0;
		length = 0;
		capacity = 0;
	}
	void clear_light() //light function for clearing vector
	{
		length=0;
	}
	void push_back( T const & source )
	{
		if(!at || length==capacity)
			expand();
		at[length++]= source;
	}
	void pop_back()
	{
		if(length)
			--length;
	}
	T * insert( T * pos, T & value ) //moves (all the elements past the index) forward by one, *where=source
	{
		if(length == capacity)
		{
			unsigned pos_index = pos - at;
			expand();
			pos = at + pos_index;
		}
		for( T * i = at + length; i > pos; --i )
			*i = *(i - 1);
		*pos = value;
		++length;
		return pos;
	}
	void erase( T * pos )
	{
		--length;
		for( T * i = pos; i < at + length; ++i )
			*i = *(i + 1);
	}
	void erase( T * posbeg, T * posend )
	{
		unsigned count = end() - posend;
		for( unsigned u = 0; u < count; ++u )
			posbeg[ u ] = posend[ u ];
		length = length - (posend - posbeg );
	}
	T * begin() const
	{
		return at;
	}
	T * end() const
	{
		return at + length;
	}
	T & back() const
	{
		return at[ length - 1 ];
	}
	T & front() const
	{
		return *at;
	}
};