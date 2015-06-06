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
float Object::GetSpeed()
{
	if( FlagMotion( -1 ) )
		return D3DXVec3Length( &velocity );
	return 0.f;
}
float Object::GetAngle()
{
	float Theta;
	D3DXVECTOR3 v;
	D3DXQuaternionToAxisAngle( &orient, &v, &Theta );
	Theta *= ((v.z < 0.f) ? -1.f : 1.f);
	return Theta - (FlagBullet( -1 ) ? D3DX_PI/2 : 0);
}
void Object::Advance()
{
	if( FlagMotion( -1 ) )
		position += velocity += accel;
	orient = FlagPixelPerfect( - 1 )? D3DXQUATERNION( 0, 0, 0, 1 ) : orient * orientvel;
}
void Object::ShotInit()
{
	accel = D3DXVECTOR3( 0, 0, 0 );
	velocity = D3DXVECTOR3( 1, 1, 1 );
	scale = D3DXVECTOR3( 1, 1, 1 );
	orientvel = D3DXQUATERNION( 0, 0, 0, 1 );
	FlagMotion( 1 );
	FlagCollidable( 1 );
	FlagCollision( 0 );
	FlagScreenDeletable( 1 );
	FlagGraze( 1 );
	FlagBullet( 1 );
}
void Object::SetShotDataParams( ShotData const & Shot_Data, ULONG Buffer_Offset )
{
	BufferOffset = Buffer_Offset;
	VertexOffset = Shot_Data.VtxOffset;
	Time = Shot_Data.AnimationTime;
	Radius = Shot_Data.Radius;
	FlagPixelPerfect( Shot_Data.Flags & 0x10 ? 1 : 0 );
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
bool Object::FlagCollision( int flag )
{
	switch( flag )
	{
	case -1:
		return (flags & 0x40) != 0;
	case 0:
		flags = flags & ~0x40;
		return false;
	case 1:
	default:
		flags = flags | 0x40;
		return true;
	}
}