#include "a2.hpp"

#include <stdio.h>
#include <math.h>


// Return a matrix to represent a counterclockwise rotation of "angle"
// degrees around the axis "axis", where "axis" is one of the
// characters 'x', 'y', or 'z'.
Matrix4x4 rotation( double angle, char axis )
{
	Matrix4x4 r;

	if ( axis )
	{
		Vector4D row1, row2, row3, row4;
		double   co = cos( angle );
		double   si = sin( angle );

		if ( axis == 'x' )
		{
			row1 = Vector4D( co, -si, 0, 0 );
			row2 = Vector4D( si,  co, 0, 0 );
			row3 = Vector4D( 0,   0,  1, 0 );
			row4 = Vector4D( 0,   0,  0, 1 );
			r    = Matrix4x4( row1, row2, row3, row4 );
		}
		else if ( axis == 'y' )
		{
			row1 = Vector4D( 1, 0,   0,  0 );
			row2 = Vector4D( 0, co, -si, 0 );
			row3 = Vector4D( 0, si,  co, 0 );
			row4 = Vector4D( 0, 0,   0,  1 );
			r    = Matrix4x4( row1, row2, row3, row4 );
		}
		else if ( axis == 'z' )
		{
			row1 = Vector4D(  co, 0, si, 0 );
			row2 = Vector4D(  0,  1, 0,  0 );
			row3 = Vector4D( -si, 0, co, 0 );
			row4 = Vector4D(  0,  0, 0,  1 );
			r    = Matrix4x4( row1, row2, row3, row4 );
		}
	}

	return r;
}

// Return a matrix to represent a displacement of the given vector.
Matrix4x4 translation( const Vector3D& displacement )
{
	Matrix4x4 t;
	Vector4D  row1, row2, row3, row4;
	double    x = displacement[0];
	double    y = displacement[1];
	double    z = displacement[2];

	row1 = Vector4D( 1, 0, 0, x );
	row2 = Vector4D( 0, 1, 0, y );
	row3 = Vector4D( 0, 0, 1, z );
	row4 = Vector4D( 0, 0, 0, 1 );
	t    = Matrix4x4( row1, row2, row3, row4 );

	return t;
}

// Return a matrix to represent a nonuniform scale with the given factors.
Matrix4x4 scaling( const Vector3D& scale )
{
	Matrix4x4 s;
	Vector4D  row1, row2, row3, row4;
	double    x = scale[0];
	double    y = scale[1];
	double    z = scale[2];

	row1 = Vector4D( x, 0, 0, 0 );
	row2 = Vector4D( 0, y, 0, 0 );
	row3 = Vector4D( 0, 0, z, 0 );
	row4 = Vector4D( 0, 0, 0, 1 );
	s    = Matrix4x4( row1, row2, row3, row4 );

	return s;
}
