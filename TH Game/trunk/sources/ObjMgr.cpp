#include <ObjMgr.hpp>
#include <Direct3DEngine.hpp>

ObjMgr::ObjMgr() : VertexCount( 0 ), PrimitiveType( D3DPT_TRIANGLELIST )
{
}

void Object::SetSpeed( float Speed )
{
	if( Speed != 0.f )
	{
		D3DXVec3Scale( &velocity, &velocity, Speed / D3DXVec3Length( &velocity ) );
		FlagMotion( 1 );
	}
	else FlagMotion( 0 );
}
void Object::SetVelocity( D3DXVECTOR3 Velocity )
{
	velocity = Velocity;
}
void Object::SetAccel( D3DXVECTOR3 Accel )
{
	accel = Accel;
}
void Object::SetPosition( D3DXVECTOR3 Position )
{
	position = Position;
}
void Object::SetScale( D3DXVECTOR3 Scaling )
{
	scale = Scaling;
}
void Object::SetAngle( float Theta )
{
	FLOAT speed = D3DXVec3Length( &velocity );
	velocity = D3DXVECTOR3( speed * cos( Theta ), speed * sin( Theta ), 0.f );
	D3DXQuaternionRotationAxis( &orient, &D3DXVECTOR3( 0, 0, 1 ), Theta + (FlagBullet( -1 ) ? D3DX_PI/2 : 0) );
}
void Object::SetAngleEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &orient, D3DXVec3Scale( &velocity, &Axis, D3DXVec3Length( &velocity ) / D3DXVec3Length( &Axis ) ), Theta );
}
void Object::SetRotation( float Theta )
{
	D3DXQuaternionRotationAxis( &orient, &D3DXVECTOR3( 0, 0, 1 ), Theta );
}
void Object::SetRotationEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &orient, &Axis, Theta );
}
void Object::SetRotationVelocity( float Theta )
{
	D3DXQuaternionRotationAxis( &orientvel, &D3DXVECTOR3( 0, 0, 1 ), Theta );
}
void Object::SetRotationVelocityEx( D3DXVECTOR3 Axis, float Theta )
{
	D3DXQuaternionRotationAxis( &orientvel, &Axis, Theta );
}
void Object::Advance()
{
	if( FlagMotion( -1 ) )
		position += velocity += accel;
	orient = FlagPixelPerfect( - 1 )? D3DXQUATERNION( 0, 0, 0, 1 ) : orient * orientvel;
}
bool Object::FlagMotion( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x1) != 0;
	case 0:
		flags = flags & ~0x1;
		return false;
	case 1:
	default:
		flags = flags | 0x1;
		return true;
	}
}
bool Object::FlagCollidable( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x2) != 0;
	case 0:
		flags = flags & ~0x2;
		return false;
	case 1:
	default:
		flags = flags | 0x2;
		return true;
	}
}
bool Object::FlagScreenDeletable( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x4) != 0;
	case 0:
		flags = flags & ~0x4;
		return false;
	case 1:
	default:
		flags = flags | 0x4;
		return true;
	}
}
bool Object::FlagGraze( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x8) != 0;
	case 0:
		flags = flags & ~0x8;
		return false;
	case 1:
	default:
		flags = flags | 0x8;
		return true;
	}
}
bool Object::FlagPixelPerfect( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x10) != 0;
	case 0:
		flags = flags & ~0x10;
		return false;
	case 1:
	default:
		flags = flags | 0x10;
		return true;
	}
}
bool Object::FlagBullet( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x20) != 0;
	case 0:
		flags = flags & ~0x20;
		return false;
	case 1:
	default:
		flags = flags | 0x20;
		return true;
	}
}