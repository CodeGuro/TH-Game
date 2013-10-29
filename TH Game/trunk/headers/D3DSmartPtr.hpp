#pragma once
#include <d3d9types.h>

template< typename T >
class D3DSmartPtr
{
private:
	T member;
	IUnknown * GetIUnknown() const
	{
		return member;
	}
	IUnknown * GetIUnknown( T const Type ) const
	{
		return Type;
	}
public:
	D3DSmartPtr() : member( NULL )
	{
	}
	D3DSmartPtr( D3DSmartPtr const & Source ) : member( Source.member )
	{
		if( Source.GetIUnknown() )
			Source.GetIUnknown()->AddRef();
	}
	D3DSmartPtr( T const & Source ) : member( Source )
	{
		if( GetIUnknown( Source ) ) GetIUnknown( Source )->AddRef();
	}
	D3DSmartPtr & operator = ( D3DSmartPtr const & Source )
	{
		if( Source.GetIUnknown() )
			Source.GetIUnknown()->AddRef();
		if( GetIUnknown() )
			GetIUnknown()->Release();
		member = Source.member;
		return *this;
	}
	T & operator = ( T const & Source )
	{
		if( GetIUnknown( Source ) ) GetIUnknown( Source )->AddRef();
		member = Source;
		return member;
	}
	T & operator * ()
	{
		return member;
	}
	T & operator -> ()
	{
		return member;
	}
	T * operator & ()
	{
		return &member;
	}
	operator T & ()
	{
		return member;
	}
	operator T const & () const
	{
		return member;
	}
	~D3DSmartPtr()
	{
		if( GetIUnknown() )
			GetIUnknown()->Release();
	}
};
