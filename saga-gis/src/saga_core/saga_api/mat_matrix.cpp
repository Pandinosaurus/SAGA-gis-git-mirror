
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                  Library: SAGA_API                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    mat_matrix.cpp                     //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'.                              //
//                                                       //
// This library is free software; you can redistribute   //
// it and/or modify it under the terms of the GNU Lesser //
// General Public License as published by the Free       //
// Software Foundation, either version 2.1 of the        //
// License, or (at your option) any later version.       //
//                                                       //
// This library is distributed in the hope that it will  //
// be useful, but WITHOUT ANY WARRANTY; without even the //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU Lesser General Public //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU Lesser     //
// General Public License along with this program; if    //
// not, see <http://www.gnu.org/licenses/>.              //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "mat_tools.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool		SG_Matrix_Triangular_Decomposition	(CSG_Matrix &A, CSG_Vector &d, CSG_Vector &e);
bool		SG_Matrix_Tridiagonal_QL			(CSG_Matrix &Q, CSG_Vector &d, CSG_Vector &e);


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Vector::CSG_Vector(void)
{
	m_Array.Create(sizeof(double), 0, SG_ARRAY_GROWTH_2);
}

//---------------------------------------------------------
CSG_Vector::CSG_Vector(const CSG_Vector &Vector)
{
	m_Array.Create(sizeof(double), 0, SG_ARRAY_GROWTH_2);

	Create(Vector);
}

bool CSG_Vector::Create(const CSG_Vector &Vector)
{
	return( Assign(Vector) );
}

//---------------------------------------------------------
CSG_Vector::CSG_Vector(int n, double *Data)
{
	m_Array.Create(sizeof(double), 0, SG_ARRAY_GROWTH_2);

	Create(n, Data);
}

bool CSG_Vector::Create(int n, double *Data)
{
	return( n > 0 && Create((size_t)n, Data) );
}

//---------------------------------------------------------
CSG_Vector::CSG_Vector(size_t n, double *Data)
{
	m_Array.Create(sizeof(double), 0, SG_ARRAY_GROWTH_2);

	Create(n, Data);
}

bool CSG_Vector::Create(size_t n, double *Data)
{
	if( n > 0 )
	{
		if( m_Array.Set_Array(n) )
		{
			if( Data )
			{
				memcpy(Get_Data(), Data, n * sizeof(double));
			}
			else
			{
				memset(Get_Data(),    0, n * sizeof(double));
			}

			return( true );
		}
	}

	Destroy();

	return( false );
}

//---------------------------------------------------------
CSG_Vector::~CSG_Vector(void)
{
	Destroy();
}

bool CSG_Vector::Destroy(void)
{
	return( m_Array.Set_Array(0) );
}

//---------------------------------------------------------
/**
  * Sets the number of rows to nRows. Values will be preserved.
  * Returns true if successful.
*/
bool CSG_Vector::Set_Rows(int nRows)
{
	return( Set_Rows((size_t)nRows) );
}

bool CSG_Vector::Set_Rows(size_t nRows)
{
	if( nRows > Get_Size() )
	{
		return( Add_Rows(nRows - Get_Size()) );
	}

	if( nRows < Get_Size() )
	{
		return( Del_Rows(Get_Size() - nRows) );
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_Vector::Add_Rows(int nRows)
{
	return( Set_Rows((int)nRows) );
}

bool CSG_Vector::Add_Rows(size_t nRows)
{
	if( nRows > 0 && m_Array.Set_Array(Get_Size() + nRows) )
	{
		for(size_t i=Get_Size()-nRows; i<Get_Size(); i++)
		{
			Get_Data()[i]	= 0.;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
/**
  * Deletes last nRows rows. Sets size to zero if nRows is greater
  * than current number of rows
  * Returns true if successful.
*/
bool CSG_Vector::Del_Rows(int nRows)
{
	return( nRows < 1 || Del_Rows((int)nRows) );
}

bool CSG_Vector::Del_Rows(size_t nRows)
{
	if( nRows >= Get_Size() )
	{
		return( Destroy() );
	}

	return( m_Array.Set_Array(Get_Size() - nRows) );
}

//---------------------------------------------------------
bool CSG_Vector::Add_Row(double Value)
{
	if( m_Array.Inc_Array() )
	{
		Get_Data()[Get_N() - 1]	= Value;

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Del_Row(int Row)
{
	if( Row < 0 )	// just remove last entry
	{
		return( m_Array.Dec_Array() );
	}

	return( Row < Get_N() && Del_Row((size_t)Row) );
}

bool CSG_Vector::Del_Row(size_t Row)
{
	if( Row < Get_Size() )
	{
		for(size_t i=Row, j=Row+1; j<Get_Size(); i++, j++)
		{
			Get_Data()[i]	= Get_Data()[j];
		}

		return( m_Array.Dec_Array() );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_String CSG_Vector::to_String(int Width, int Precision, bool bScientific, const SG_Char *Separator)	const
{
	CSG_String	s, sep(Separator && *Separator ? Separator : SG_T(" "));

	for(int i=0; i<Get_N(); i++)
	{
		s	+= sep + SG_Get_Double_asString(Get_Data(i), Width, Precision, bScientific);
	}

	return( s );
}

//---------------------------------------------------------
bool CSG_Vector::from_String(const CSG_String &String)
{
	Destroy();

	CSG_String_Tokenizer	Line(String);

	while( Line.Has_More_Tokens() )
	{
		double		d;
		CSG_String	s(Line.Get_Next_Token());

		if( s.asDouble(d) )
		{
			Add_Row(d);
		}
	}

	return( Get_N() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Vector::is_Equal(const CSG_Vector &Vector) const
{
	if( Get_N() == Vector.Get_N() )
	{
		for(int i=0; i<Get_N(); i++)
		{
			if( Get_Data(i) != Vector.Get_Data(i) )
			{
				return( false );
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Assign(double Scalar)
{
	if( Get_N() > 0 )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	= Scalar;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Assign(const CSG_Vector &Vector)
{
	if( Create(Vector.Get_N()) )
	{
		memcpy(Get_Data(), Vector.Get_Data(), Get_N() * sizeof(double));

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Add(double Scalar)
{
	if( Get_N() > 0 )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	+= Scalar;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Add(const CSG_Vector &Vector)
{
	if( Get_N() == Vector.Get_N() && Get_N() > 0 )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	+= Vector.Get_Data()[i];
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Subtract(const CSG_Vector &Vector)
{
	if( Get_N() == Vector.Get_N() && Get_N() > 0 )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	-= Vector.Get_Data()[i];
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Multiply(double Scalar)
{
	if( Get_N() > 0 )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	*= Scalar;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Vector::Multiply(const CSG_Vector &Vector)
{
	if( Get_N() == Vector.Get_N() && Get_N() == 3 )
	{
		CSG_Vector	v(*this);

		Get_Data()[0]	= v[1] * Vector[2] - v[2] * Vector[1];
		Get_Data()[1]	= v[2] * Vector[0] - v[0] * Vector[2];
		Get_Data()[2]	= v[0] * Vector[1] - v[1] * Vector[0];

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
double CSG_Vector::Multiply_Scalar(const CSG_Vector &Vector) const
{
	double	z	= 0.;

	if( Get_N() == Vector.Get_N() )
	{
		for(int i=0; i<Get_N(); i++)
		{
			z	+= Get_Data()[i] * Vector[i];
		}
	}

	return( z );
}

//---------------------------------------------------------
bool CSG_Vector::Multiply(const class CSG_Matrix &Matrix)
{
	return( Assign(Matrix.Multiply(*this)) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Vector::operator == (const CSG_Vector &Vector) const
{
	return( is_Equal(Vector) );
}

//---------------------------------------------------------
CSG_Vector & CSG_Vector::operator = (double Scalar)
{
	Assign(Scalar);

	return( *this );
}

CSG_Vector & CSG_Vector::operator = (const CSG_Vector &Vector)
{
	Assign(Vector);

	return( *this );
}

//---------------------------------------------------------
CSG_Vector & CSG_Vector::operator += (double Scalar)
{
	Add(Scalar);

	return( *this );
}

CSG_Vector & CSG_Vector::operator += (const CSG_Vector &Vector)
{
	Add(Vector);

	return( *this );
}

//---------------------------------------------------------
CSG_Vector & CSG_Vector::operator -= (double Scalar)
{
	Add(-Scalar);

	return( *this );
}

CSG_Vector & CSG_Vector::operator -= (const CSG_Vector &Vector)
{
	Subtract(Vector);

	return( *this );
}

//---------------------------------------------------------
CSG_Vector & CSG_Vector::operator *= (double Scalar)
{
	Multiply(Scalar);

	return( *this );
}

CSG_Vector & CSG_Vector::operator *= (const CSG_Vector &Vector)
{
	Multiply(Vector);

	return( *this );
}

CSG_Vector & CSG_Vector::operator *= (const class CSG_Matrix &Matrix)
{
	Multiply(Matrix);

	return( *this );
}

//---------------------------------------------------------
CSG_Vector CSG_Vector::operator + (double Scalar) const
{
	CSG_Vector	v(*this);

	v.Add(Scalar);

	return( v );
}

CSG_Vector CSG_Vector::operator + (const CSG_Vector &Vector) const
{
	CSG_Vector	v(*this);

	v.Add(Vector);

	return( v );
}

//---------------------------------------------------------
CSG_Vector CSG_Vector::operator - (double Scalar) const
{
	CSG_Vector	v(*this);

	v.Add(-Scalar);

	return( v );
}

CSG_Vector CSG_Vector::operator - (const CSG_Vector &Vector) const
{
	CSG_Vector	v(*this);

	v.Subtract(Vector);

	return( v );
}

//---------------------------------------------------------
CSG_Vector CSG_Vector::operator * (double Scalar) const
{
	CSG_Vector	v(*this);

	v.Multiply(Scalar);

	return( v );
}

double CSG_Vector::operator * (const CSG_Vector &Vector) const
{
	return( Multiply_Scalar(Vector) );
}

CSG_Vector operator * (double Scalar, const CSG_Vector &Vector)
{
	return( Vector * Scalar );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Vector::Set_Zero(void)
{
	return( Create(Get_N()) );
}

//---------------------------------------------------------
bool CSG_Vector::Set_Unity(void)
{
	double	Length;

	if( (Length = Get_Length()) > 0. )
	{
		for(int i=0; i<Get_N(); i++)
		{
			Get_Data()[i]	/= Length;
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Vector::Flip_Values(void)
{
	if( Get_Size() > 1 )
	{
		double	*v	= Get_Data();

		for(size_t i=0, j=Get_Size()-1; i<j; i++, j--)
		{
			double d = v[i]; v[i] = v[j]; v[j] = d;
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Vector::Sort(bool bAscending)
{
	if( Get_Size() > 0 )
	{
		qsort(Get_Data(), Get_Size(), sizeof(double), SG_Compare_Double);

		if( bAscending == false )
		{
			Flip_Values();
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
double CSG_Vector::Get_Length(void) const
{
	if( Get_N() > 0 )
	{
		double	z	= 0., *Z	= Get_Data();

		for(int i=0; i<Get_N(); i++)
		{
			z	+= Z[i] * Z[i];
		}

		return( sqrt(z) );
	}

	return( 0. );
}

//---------------------------------------------------------
double CSG_Vector::Get_Angle(const CSG_Vector &Vector) const
{
	if( Get_N() > Vector.Get_N() )
	{
		return( Vector.Get_Angle(*this) );
	}

	double	A, B;

	if( (A = Get_Length()) > 0. && (B = Vector.Get_Length()) > 0. )
	{
		double	z = 0., *Z = Get_Data();

		for(int i=0; i<Get_N(); i++)
		{
			z	+= Vector[i] * Z[i];
		}

		for(int i=Get_N(); i<Vector.Get_N(); i++)
		{
			z	+= Vector[i];
		}

		return( acos(z / (A * B)) );
	}

	return( 0. );
}

//---------------------------------------------------------
CSG_Vector CSG_Vector::Get_Unity(void) const
{
	CSG_Vector	v(*this);

	v.Set_Unity();

	return( v );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool	SG_VectorR2_Rotate(double &x, double &y, double Angle)
{
	double	sin_a	= sin(Angle);
	double	cos_a	= cos(Angle);

	double	t	= x;

	x	= t * cos_a - y * sin_a;
	y	= t * sin_a + y * cos_a;

	return( true );
}

//---------------------------------------------------------
bool	SG_VectorR2_Rotate(double     Vector[2], double Angle)
{
	return( SG_VectorR2_Rotate(Vector[0], Vector[1], Angle) );
}

//---------------------------------------------------------
bool	SG_VectorR2_Rotate(CSG_Vector &Vector  , double Angle)
{
	return( Vector.Get_N() >= 2 && SG_VectorR2_Rotate(Vector[0], Vector[1], Angle) );
}

//---------------------------------------------------------
bool	SG_VectorR3_Rotate(double     Vector[3], size_t Axis, double Angle)
{
	if( Axis <= 3 )
	{
		CSG_Vector	v(3, Vector);

		double	sin_a	= sin(Angle);
		double	cos_a	= cos(Angle);

		switch( Axis )
		{
		case 0:
			Vector[1] = v[1] * cos_a - v[2] * sin_a;
			Vector[2] = v[1] * sin_a + v[2] * cos_a;
			break;

		case 1:
			Vector[0] = v[0] * cos_a + v[2] * sin_a;
			Vector[2] =-v[0] * sin_a + v[2] * cos_a;
			break;

		case 2:
			Vector[0] = v[0] * cos_a - v[1] * sin_a;
			Vector[1] = v[0] * sin_a + v[1] * cos_a;
			break;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool	SG_VectorR3_Rotate(CSG_Vector &Vector  , size_t Axis, double Angle)
{
	return( Vector.Get_N() >= 3 && SG_VectorR3_Rotate(Vector.Get_Data(), Axis, Angle) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Matrix::CSG_Matrix(void)
{
	_On_Construction();
}

//---------------------------------------------------------
CSG_Matrix::CSG_Matrix(const CSG_Matrix &Matrix)
{
	_On_Construction();

	Create(Matrix);
}

bool CSG_Matrix::Create(const CSG_Matrix &Matrix)
{
	return( Create(Matrix.m_nx, Matrix.m_ny, Matrix.m_z[0]) );
}

//---------------------------------------------------------
CSG_Matrix::CSG_Matrix(int nCols, int nRows, double *Data)
{
	_On_Construction();

	Create(nCols, nRows, Data);
}

bool CSG_Matrix::Create(int nCols, int nRows, double *Data)
{
	if( nCols < 1 || nRows < 1 )
	{
		Destroy();

		return( false );
	}

	if( nCols != m_nx || nRows != m_ny )
	{
		Destroy();

		if( (m_z    = (double **)SG_Malloc(nRows         * sizeof(double *))) == NULL
		||  (m_z[0]	= (double  *)SG_Malloc(nRows * nCols * sizeof(double  ))) == NULL )
		{
			Destroy();

			return( false );
		}

		m_nx	= nCols;
		m_ny	= nRows;

		for(int y=1; y<m_ny; y++)
		{
			m_z[y]	= m_z[y - 1] + m_nx;
		}
	}

	if( Data )
	{
		memcpy(m_z[0], Data, m_ny * m_nx * sizeof(double));
	}
	else
	{
		memset(m_z[0],    0, m_ny * m_nx * sizeof(double));
	}

	return( true );
}

//---------------------------------------------------------
CSG_Matrix::CSG_Matrix(int nCols, int nRows, double **Data)
{
	_On_Construction();

	Create(nCols, nRows, Data);
}

bool CSG_Matrix::Create(int nCols, int nRows, double **Data)
{
	if( Create(nCols, nRows) )
	{
		if( Data )
		{
			for(int y=0; y<m_ny; y++)
			{
				memcpy(m_z[y], Data[y], m_nx * sizeof(double));
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
CSG_Matrix::~CSG_Matrix(void)
{
	Destroy();
}

bool CSG_Matrix::Destroy(void)
{
	if( m_z )
	{
		if( m_z[0] )
		{
			SG_Free(m_z[0]);
		}

		SG_Free(m_z);
	}

	_On_Construction();

	return( true );
}

//---------------------------------------------------------
void CSG_Matrix::_On_Construction(void)
{
	m_z		= NULL;
	m_nx	= 0;
	m_ny	= 0;
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Matrix::Set_Size(int nRows, int nCols)
{
	return( nRows > 0 && nCols > 0 && Set_Rows(nRows) && Set_Cols(nCols) );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Cols(int nCols)
{
	if( nCols > m_nx )
	{
		return( Add_Cols(nCols - m_nx) );
	}

	if( nCols < m_nx )
	{
		return( Del_Cols(m_nx - nCols) );
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Rows(int nRows)
{
	if( nRows > m_ny )
	{
		return( Add_Rows(nRows - m_ny) );
	}

	if( nRows < m_ny )
	{
		return( Del_Rows(m_ny - nRows) );
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_Matrix::Add_Cols(int nCols)
{
	if( nCols > 0 && m_ny > 0 )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx + nCols, Tmp.m_ny) )
		{
			for(int y=0; y<Tmp.m_ny; y++)
			{
				memcpy(m_z[y], Tmp.m_z[y], Tmp.m_nx * sizeof(double));
			}

			return( true );
		}
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Add_Rows(int nRows)
{
	if( nRows > 0 && m_nx > 0 )
	{
		m_ny	+= nRows;

		m_z		= (double **)SG_Realloc(m_z   , m_ny        * sizeof(double *));
		m_z[0]	= (double  *)SG_Realloc(m_z[0], m_ny * m_nx * sizeof(double  ));

		for(int y=1; y<m_ny; y++)
		{
			m_z[y]	= m_z[y - 1] + m_nx;
		}

		memset(m_z[m_ny - nRows], 0, nRows * m_nx * sizeof(double));

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
/**
  * Deletes the last nCols columns.
*/
bool CSG_Matrix::Del_Cols(int nCols)
{
	if( nCols > 0 && m_ny > 0 && nCols < m_nx )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx - nCols, Tmp.m_ny) )
		{
			for(int y=0; y<Tmp.m_ny; y++)
			{
				memcpy(m_z[y], Tmp.m_z[y], m_nx * sizeof(double));
			}

			return( true );
		}
	}

	return( false );
}

//---------------------------------------------------------
/**
  * Deletes the last nRows rows.
*/
bool CSG_Matrix::Del_Rows(int nRows)
{
	if( nRows > 0 && m_nx > 0 && nRows < m_ny )
	{
		m_ny	-= nRows;

		m_z		= (double **)SG_Realloc(m_z   , m_ny        * sizeof(double *));
		m_z[0]	= (double  *)SG_Realloc(m_z[0], m_ny * m_nx * sizeof(double  ));

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Add_Col(double *Data)
{
	if( Add_Cols(1) )
	{
		Set_Col(m_nx - 1, Data);

		return( true );
	}

	return( false );
}

bool CSG_Matrix::Add_Col(const CSG_Vector &Data)
{
	if( m_nx == 0 )
	{
		return( Create(1, Data.Get_N()) && Set_Col(0, Data.Get_Data()) );
	}

	return( m_ny <= Data.Get_N() && Add_Col(Data.Get_Data()) );
}

//---------------------------------------------------------
bool CSG_Matrix::Add_Row(double *Data)
{
	if( Add_Rows(1) )
	{
		Set_Row(m_ny - 1, Data);

		return( true );
	}

	return( false );
}

bool CSG_Matrix::Add_Row(const CSG_Vector &Data)
{
	if( m_ny == 0 )
	{
		return( Create(Data.Get_N(), 1) && Set_Row(0, Data.Get_Data()) );
	}

	return( m_nx <= Data.Get_N() && Add_Row(Data.Get_Data()) );
}

//---------------------------------------------------------
bool CSG_Matrix::Ins_Col(int Col, double *Data)
{
	if( Col >= 0 && Col <= m_nx )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx + 1, Tmp.m_ny) )
		{
			for(int y=0; y<m_ny; y++)
			{
				double	*pz	= m_z[y], *pz_tmp	= Tmp.m_z[y];

				for(int x=0; x<m_nx; x++, pz++)
				{
					if( x != Col )
					{
						*pz	= *pz_tmp;	pz_tmp++;
					}
					else if( Data )
					{
						*pz	= Data[y];
					}
				}
			}

			return( true );
		}
	}

	return( false );
}

bool CSG_Matrix::Ins_Col(int Col, const CSG_Vector &Data)
{
	return( m_nx == 0 ? Add_Col(Data) : m_ny <= Data.Get_N() ? Ins_Col(Col, Data.Get_Data()) : false );
}

//---------------------------------------------------------
bool CSG_Matrix::Ins_Row(int Row, double *Data)
{
	if( Row >= 0 && Row <= m_ny )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx, Tmp.m_ny + 1) )
		{
			for(int y=0, y_tmp=0; y<m_ny; y++)
			{
				if( y != Row )
				{
					memcpy(m_z[y], Tmp.m_z[y_tmp++], m_nx * sizeof(double));
				}
				else if( Data )
				{
					memcpy(m_z[y], Data, m_nx * sizeof(double));
				}
			}

			return( true );
		}
	}

	return( false );
}

bool CSG_Matrix::Ins_Row(int Row, const CSG_Vector &Data)
{
	return( m_ny == 0 ? Add_Row(Data) : m_nx <= Data.Get_N() ? Ins_Row(Row, Data.Get_Data()) : false );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Col(int Col, double *Data)
{
	if( Data && Col >= 0 && Col < m_nx )
	{
		for(int y=0; y<m_ny; y++)
		{
			m_z[y][Col]	= Data[y];
		}

		return( true );
	}

	return( false );
}

bool CSG_Matrix::Set_Col(int Col, const CSG_Vector &Data)
{
	return( m_ny <= Data.Get_N() ? Set_Col(Col, Data.Get_Data()) : false );
}

//---------------------------------------------------------
/**
* Sets matrix size to one column with vector's data as row.
*/
bool CSG_Matrix::Set_Col(const CSG_Vector &Data)
{
	return( Create(1, Data.Get_N()) && Set_Col(0, Data.Get_Data()) );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Row(int Row, double *Data)
{
	if( Data && Row >= 0 && Row < m_ny )
	{
		memcpy(m_z[Row], Data, m_nx * sizeof(double));

		return( true );
	}

	return( false );
}

bool CSG_Matrix::Set_Row(int Row, const CSG_Vector &Data)
{
	return( m_nx <= Data.Get_N() ? Set_Row(Row, Data.Get_Data()) : false );
}

//---------------------------------------------------------
/**
* Sets matrix size to one row with vector's data as column.
*/
bool CSG_Matrix::Set_Row(const CSG_Vector &Data)
{
	return( Create(Data.Get_N(), 1) && Set_Row(0, Data.Get_Data()) );
}

//---------------------------------------------------------
bool CSG_Matrix::Del_Col(int Col)
{
	if( m_nx == 1 )
	{
		return( Destroy() );
	}

	if( Col >= 0 && Col < m_nx )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx - 1, Tmp.m_ny) )
		{
			for(int y=0; y<m_ny; y++)
			{
				double	*pz	= m_z[y], *pz_tmp	= Tmp.m_z[y];

				for(int x_tmp=0; x_tmp<Tmp.m_nx; x_tmp++, pz_tmp++)
				{
					if( x_tmp != Col )
					{
						*pz	= *pz_tmp;	pz++;
					}
				}
			}

			return( true );
		}
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Del_Row(int Row)
{
	if( m_ny == 1 )
	{
		return( Destroy() );
	}

	if( Row >= 0 && Row < m_ny )
	{
		CSG_Matrix	Tmp(*this);

		if( Create(Tmp.m_nx, Tmp.m_ny - 1) )
		{
			for(int y=0, y_tmp=0; y_tmp<Tmp.m_ny; y_tmp++)
			{
				if( y_tmp != Row )
				{
					memcpy(m_z[y++], Tmp.m_z[y_tmp], m_nx * sizeof(double));
				}
			}

			return( true );
		}
	}

	return( false );
}

//---------------------------------------------------------
CSG_Vector CSG_Matrix::Get_Col(int Col)	const
{
	CSG_Vector	Vector;
	
	if( Col >= 0 && Col < m_nx )
	{
		Vector.Create(m_ny);

		for(int y=0; y<m_ny; y++)
		{
			Vector[y]	= m_z[y][Col];
		}
	}

	return( Vector );
}

//---------------------------------------------------------
CSG_Vector CSG_Matrix::Get_Row(int Row)	const
{
	CSG_Vector	Vector;
	
	if( Row >= 0 && Row < m_ny )
	{
		Vector.Create(m_nx, m_z[Row]);
	}

	return( Vector );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_String CSG_Matrix::to_String(int Width, int Precision, bool bScientific, const SG_Char *Separator)	const
{
	CSG_String	s, sep(Separator && *Separator ? Separator : SG_T(" "));

	for(int y=0, n=SG_Get_Digit_Count(m_ny + 1); y<m_ny; y++)
	{
		s	+= CSG_String::Format("\n%0*d:", n, y + 1);

		for(int x=0; x<m_nx; x++)
		{
			s	+= sep + SG_Get_Double_asString(m_z[y][x], Width, Precision, bScientific);
		}
	}

	s	+= "\n";

	return( s );
}

//---------------------------------------------------------
bool CSG_Matrix::from_String(const CSG_String &String)
{
	Destroy();

	CSG_String_Tokenizer	Lines(String, "\r\n");

	while( Lines.Has_More_Tokens() )
	{
		CSG_String_Tokenizer	Line(Lines.Get_Next_Token().AfterFirst(':'));

		CSG_Vector	z;

		while( Line.Has_More_Tokens() )
		{
			double		d;
			CSG_String	s(Line.Get_Next_Token());

			if( s.asDouble(d) )
			{
				z.Add_Row(d);
			}
		}

		Add_Row(z);
	}

	return( Get_NRows() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Matrix::is_Equal(const CSG_Matrix &Matrix) const
{
	if( m_nx < Matrix.m_nx || m_ny < Matrix.m_ny )
	{
		return( false );
	}

	for(int y=0; y<m_ny; y++) for(int x=0; x<m_nx; x++)
	{
		if( m_z[y][x] != Matrix.m_z[y][x] )
		{
			return( false );
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_Matrix::Assign(double Scalar)
{
	if( m_nx > 0 && m_ny > 0 )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	= Scalar;
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Assign(const CSG_Matrix &Matrix)
{
	return( Create(Matrix) );
}

//---------------------------------------------------------
bool CSG_Matrix::Add(double Scalar)
{
	if( m_nx > 0 && m_ny > 0 )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	+= Scalar;
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Add(const CSG_Matrix &Matrix)
{
	if( m_nx == Matrix.m_nx && m_ny == Matrix.m_ny )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	+= Matrix.m_z[y][x];
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Subtract(const CSG_Matrix &Matrix)
{
	if( m_nx == Matrix.m_nx && m_ny == Matrix.m_ny )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	-= Matrix.m_z[y][x];
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Multiply(double Scalar)
{
	if( m_nx > 0 && m_ny > 0 )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	*= Scalar;
			}
		}

		return( true );
	}

	return( false );
}

CSG_Vector CSG_Matrix::Multiply(const CSG_Vector &Vector) const
{
	CSG_Vector	v;

	if( m_nx == Vector.Get_N() && v.Create(m_ny) )
	{
		for(int y=0; y<m_ny; y++)
		{
			double	z	= 0.;

			for(int x=0; x<m_nx; x++)
			{
				z	+= m_z[y][x] * Vector(x);
			}

			v[y]	= z;
		}
	}

	return( v );
}

CSG_Matrix CSG_Matrix::Multiply(const CSG_Matrix &Matrix) const
{
	CSG_Matrix	m;

	if( m_nx == Matrix.m_ny && m.Create(Matrix.m_nx, m_ny) )
	{
		for(int y=0; y<m.m_ny; y++)
		{
			for(int x=0; x<m.m_nx; x++)
			{
				double	z	= 0.;

				for(int n=0; n<m_nx; n++)
				{
					z	+= m_z[y][n] * Matrix.m_z[n][x];
				}

				m.m_z[y][x]	= z;
			}
		}
	}

	return( m );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Matrix::operator == (const CSG_Matrix &Matrix) const
{
	return( is_Equal(Matrix) );
}

//---------------------------------------------------------
CSG_Matrix & CSG_Matrix::operator = (double Scalar)
{
	Assign(Scalar);

	return( *this );
}

CSG_Matrix & CSG_Matrix::operator = (const CSG_Matrix &Matrix)
{
	Assign(Matrix);

	return( *this );
}

//---------------------------------------------------------
CSG_Matrix & CSG_Matrix::operator += (double Scalar)
{
	Add(Scalar);

	return( *this );
}

CSG_Matrix & CSG_Matrix::operator += (const CSG_Matrix &Matrix)
{
	Add(Matrix);

	return( *this );
}

//---------------------------------------------------------
CSG_Matrix & CSG_Matrix::operator -= (double Scalar)
{
	Add(-Scalar);

	return( *this );
}

CSG_Matrix & CSG_Matrix::operator -= (const CSG_Matrix &Matrix)
{
	Subtract(Matrix);

	return( *this );
}

//---------------------------------------------------------
CSG_Matrix & CSG_Matrix::operator *= (double Scalar)
{
	Multiply(Scalar);

	return( *this );
}

CSG_Matrix & CSG_Matrix::operator *= (const CSG_Matrix &Matrix)
{
	Multiply(Matrix);

	return( *this );
}

//---------------------------------------------------------
CSG_Matrix CSG_Matrix::operator + (double Scalar) const
{
	CSG_Matrix	m(*this);

	m.Add(Scalar);

	return( m );
}

CSG_Matrix CSG_Matrix::operator + (const CSG_Matrix &Matrix) const
{
	CSG_Matrix	m(*this);

	m.Add(Matrix);

	return( m );
}

//---------------------------------------------------------
CSG_Matrix CSG_Matrix::operator - (double Scalar) const
{
	CSG_Matrix	m(*this);

	m.Add(-Scalar);

	return( m );
}

CSG_Matrix CSG_Matrix::operator - (const CSG_Matrix &Matrix) const
{
	CSG_Matrix	m(*this);

	m.Subtract(Matrix);

	return( m );
}

//---------------------------------------------------------
CSG_Matrix CSG_Matrix::operator * (double Scalar) const
{
	CSG_Matrix	m(*this);

	m.Multiply(Scalar);

	return( m );
}

CSG_Vector CSG_Matrix::operator * (const CSG_Vector &Vector) const
{
	return( Multiply(Vector) );
}

CSG_Matrix CSG_Matrix::operator * (const CSG_Matrix &Matrix) const
{
	return( Multiply(Matrix) );
}

CSG_Matrix	operator * (double Scalar, const CSG_Matrix &Matrix)
{
	return( Matrix * Scalar );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Matrix::Set_Zero(void)
{
	return( Create(m_nx, m_ny) );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Identity(void)
{
	if( m_nx > 0 && m_ny > 0 )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	= x == y ? 1. : 0.;
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Transpose(void)
{
	CSG_Matrix	m;
	
	if( m.Create(*this) && Create(m_ny, m_nx) )
	{
		for(int y=0; y<m_ny; y++)
		{
			for(int x=0; x<m_nx; x++)
			{
				m_z[y][x]	= m.m_z[x][y];
			}
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Matrix::Set_Inverse(bool bSilent, int nSubSquare)
{
	//-----------------------------------------------------
	int		n	= 0;

	if( nSubSquare > 0 )
	{
		if( nSubSquare <= m_nx && nSubSquare <= m_ny )
		{
			n	= nSubSquare;
		}
	}
	else if( is_Square() )
	{
		n	= m_nx;
	}

	//-----------------------------------------------------
	if( n > 0 )
	{
		CSG_Matrix	m(*this);
		CSG_Array	p(sizeof(int), n);

		if( SG_Matrix_LU_Decomposition(n, (int *)p.Get_Array(), m.Get_Data(), bSilent) )
		{
			CSG_Vector	v(n);

			for(int j=0; j<n && (bSilent || SG_UI_Process_Set_Progress(j, n)); j++)
			{
				v.Set_Zero();
				v[j]	= 1.;

				SG_Matrix_LU_Solve(n, (int *)p.Get_Array(), m, v.Get_Data(), true);

				for(int i=0; i<n; i++)
				{
					m_z[i][j]	= v[i];
				}
			}

			return( true );
		}
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
double CSG_Matrix::Get_Determinant(void) const
{
	double	d	= 0.;

	if( is_Square() )	// det is only defined for squared matrices !
	{
		int			n;
		CSG_Matrix	m(*this);
		CSG_Array	p(sizeof(int), m_nx);

		if( SG_Matrix_LU_Decomposition(m_nx, (int *)p.Get_Array(), m.Get_Data(), true, &n) )
		{
			d	= n % 2 ? -1. : 1.;

			for(int i=0; i<m_nx; i++)
			{
				d	*= m[i][i];
			}
		}
	}

	return( d );
}

//---------------------------------------------------------
CSG_Matrix CSG_Matrix::Get_Transpose(void) const
{
	CSG_Matrix	m(m_ny, m_nx);

	for(int y=0; y<m_ny; y++)
	{
		for(int x=0; x<m_nx; x++)
		{
			m.m_z[x][y]	= m_z[y][x];
		}
	}

	return( m );
}

//---------------------------------------------------------
CSG_Matrix CSG_Matrix::Get_Inverse(bool bSilent, int nSubSquare) const
{
	CSG_Matrix	m(*this);

	m.Set_Inverse(bSilent, nSubSquare);

	return( m );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool		SG_Matrix_Solve(CSG_Matrix &Matrix, CSG_Vector &Vector, bool bSilent)
{
	int	n	= Vector.Get_N();

	if( n > 0 && n == Matrix.Get_NX() && n == Matrix.Get_NY() )
	{
		CSG_Array	Permutation(sizeof(int), n);

		if( SG_Matrix_LU_Decomposition(n, (int *)Permutation.Get_Array(), Matrix.Get_Data(), bSilent) )
		{
			return( SG_Matrix_LU_Solve(n, (int *)Permutation.Get_Array(), Matrix, Vector.Get_Data(), bSilent) );
		}
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool		SG_Matrix_Eigen_Reduction(const CSG_Matrix &Matrix, CSG_Matrix &Eigen_Vectors, CSG_Vector &Eigen_Values, bool bSilent)
{
	CSG_Vector	Intermediate;

	Eigen_Vectors	= Matrix;

	return(	SG_Matrix_Triangular_Decomposition	(Eigen_Vectors, Eigen_Values, Intermediate)	// Triangular decomposition (Householder's method)
		&&	SG_Matrix_Tridiagonal_QL			(Eigen_Vectors, Eigen_Values, Intermediate)	// Reduction of symmetric tridiagonal matrix
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool		SG_Matrix_LU_Decomposition(int n, int *Permutation, double **Matrix, bool bSilent, int *nRowChanges)
{
	int			i, j, k, iMax;
	double		dMax, d, Sum;
	CSG_Vector	Vector;
	
	Vector.Create(n);

	if( nRowChanges )	(*nRowChanges)	= 0;

	for(i=0, iMax=0; i<n && (bSilent || SG_UI_Process_Set_Progress(i, n)); i++)
	{
		dMax	= 0.;

		for(j=0; j<n; j++)
		{
			if( (d = fabs(Matrix[i][j])) > dMax )
			{
				dMax	= d;
			}
		}

		if( dMax <= 0. )	// singular matrix !!!...
		{
			return( false );
		}

		Vector[i]	= 1. / dMax;
	}

	for(j=0; j<n && (bSilent || SG_UI_Process_Set_Progress(j, n)); j++)
	{
		for(i=0; i<j; i++)
		{
			Sum		= Matrix[i][j];

			for(k=0; k<i; k++)
			{
				Sum		-= Matrix[i][k] * Matrix[k][j];
			}

			Matrix[i][j]	= Sum;
		}

		for(i=j, dMax=0.; i<n; i++)
		{
			Sum		= Matrix[i][j];

			for(k=0; k<j; k++)
			{
				Sum		-= Matrix[i][k] * Matrix[k][j];
			}

			Matrix[i][j]	= Sum;

			if( (d = Vector[i] * fabs(Sum)) >= dMax )
			{
				dMax	= d;
				iMax	= i;
			}
		}

		if( j != iMax )
		{
			for(k=0; k<n; k++)
			{
				d				= Matrix[iMax][k];
				Matrix[iMax][k]	= Matrix[j   ][k];
				Matrix[j   ][k]	= d;
			}

			Vector[iMax]	= Vector[j];

			if( nRowChanges )	(*nRowChanges)++;
		}

		Permutation[j]	= iMax;

		if( Matrix[j][j] == 0. )
		{
			Matrix[j][j]	= M_TINY;
		}

		if( j != n )
		{
			d	= 1. / (Matrix[j][j]);

			for(i=j+1; i<n; i++)
			{
				Matrix[i][j]	*= d;
			}
		}
	}

	return( bSilent || SG_UI_Process_Get_Okay(false) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool		SG_Matrix_LU_Solve(int n, const int *Permutation, const double **Matrix, double *Vector, bool bSilent)
{
	int		i, j, k;
	double	Sum;

	for(i=0, k=-1; i<n && (bSilent || SG_UI_Process_Set_Progress(i, n)); i++)
	{
		Sum						= Vector[Permutation[i]];
		Vector[Permutation[i]]	= Vector[i];

		if( k >= 0 )
		{
			for(j=k; j<=i-1; j++)
			{
				Sum	-= Matrix[i][j] * Vector[j];
			}
		}
		else if( Sum )
		{
			k		= i;
		}

		Vector[i]	= Sum;
	}

	for(i=n-1; i>=0 && (bSilent || SG_UI_Process_Set_Progress(n-i, n)); i--)
	{
		Sum			= Vector[i];

		for(j=i+1; j<n; j++)
		{
			Sum		-= Matrix[i][j] * Vector[j];
		}

		Vector[i]	= Sum / Matrix[i][i];
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
// Householder reduction of matrix a to tridiagonal form.

bool SG_Matrix_Triangular_Decomposition(CSG_Matrix &A, CSG_Vector &d, CSG_Vector &e)
{
	if( A.Get_NX() != A.Get_NY() )
	{
		return( false );
	}

	int		l, k, j, i, n;
	double	scale, hh, h, g, f;

	n	= A.Get_NX();

	d.Create(n);
	e.Create(n);

	for(i=n-1; i>=1; i--)
	{
		l	= i - 1;
		h	= scale = 0.;

		if( l > 0 )
		{
			for(k=0; k<=l; k++)
			{
				scale	+= fabs(A[i][k]);
			}

			if( scale == 0. )
			{
				e[i]	= A[i][l];
			}
			else
			{
				for(k=0; k<=l; k++)
				{
					A[i][k]	/= scale;
					h		+= A[i][k] * A[i][k];
				}

				f		= A[i][l];
				g		= f > 0. ? -sqrt(h) : sqrt(h);
				e[i]	= scale * g;
				h		-= f * g;
				A[i][l]	= f - g;
				f		= 0.;

				for(j=0; j<=l; j++)
				{
					A[j][i]	= A[i][j]/h;
					g		= 0.;

					for(k=0; k<=j; k++)
					{
						g	+= A[j][k] * A[i][k];
					}

					for(k=j+1; k<=l; k++)
					{
						g	+= A[k][j] * A[i][k];
					}

					e[j]	= g / h;
					f		+= e[j] * A[i][j];
				}

				hh	= f / (h + h);

				for(j=0; j<=l; j++)
				{
					f		= A[i][j];
					e[j]	= g = e[j] - hh * f;

					for(k=0; k<=j; k++)
					{
						A[j][k]	-= (f * e[k] + g * A[i][k]);
					}
				}
			}
		}
		else
		{
			e[i]	= A[i][l];
		}

		d[i]	= h;
	}

	d[0]	= 0.;
	e[0]	= 0.;

	for(i=0; i<n; i++)
	{
		l	= i - 1;

		if( d[i] )
		{	
			for(j=0; j<=l; j++)
			{
				g	= 0.;

				for(k=0; k<=l; k++)
				{
					g		+= A[i][k] * A[k][j];
				}

				for(k=0; k<=l; k++)
				{
					A[k][j]	-= g * A[k][i];
				}
			}
		}

		d[i]	= A[i][i];
		A[i][i]	= 1.;

		for(j=0; j<=l; j++)
		{
			A[j][i]	= A[i][j] = 0.;
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
// Tridiagonal QL algorithm -- Implicit

bool SG_Matrix_Tridiagonal_QL(CSG_Matrix &Q, CSG_Vector &d, CSG_Vector &e)
{
	if( Q.Get_NX() != Q.Get_NY() || Q.Get_NX() != d.Get_N() || Q.Get_NX() != e.Get_N() )
	{
		return( false );
	}

	int		m, l, iter, i, k, n;
	double	s, r, p, g, f, dd, c, b;

	n	= d.Get_N();

	for(i=1; i<n; i++)
	{
		e[i - 1]	= e[i];
	}

	e[n - 1]	= 0.;

	for(l=0; l<n; l++)
	{
		iter	= 0;

		do
		{
			for(m=l; m<n-1; m++)
			{
				dd	= fabs(d[m]) + fabs(d[m + 1]);

				if( fabs(e[m]) + dd == dd )
				{
					break;
				}
			}

			if( m != l )
			{
				if( iter++ == 30 )
				{
					return( false );	// erhand("No convergence in TLQI.");
				}

				g	= (d[l+1] - d[l]) / (2. * e[l]);
				r	= sqrt((g * g) + 1.);
				g	= d[m] - d[l] + e[l] / (g + M_SET_SIGN(r, g));
				s	= c = 1.;
				p	= 0.;

				for(i = m-1; i >= l; i--)
				{
					f = s * e[i];
					b = c * e[i];

					if (fabs(f) >= fabs(g))
					{
						c = g / f;
						r = sqrt((c * c) + 1.);
						e[i+1] = f * r;
						c *= (s = 1. / r);
					}
					else
					{
						s = f / g;
						r = sqrt((s * s) + 1.);
						e[i+1] = g * r;
						s *= (c = 1. / r);
					}

					g		= d[i+1] - p;
					r		= (d[i] - g) * s + 2. * c * b;
					p		= s * r;
					d[i+1]	= g + p;
					g		= c * r - b;

					for(k=0; k<n; k++)
					{
						f			= Q[k][i+1];
						Q[k][i+1]	= s * Q[k][i] + c * f;
						Q[k][i]		= c * Q[k][i] - s * f;
					}
				}

				d[l] = d[l] - p;
				e[l] = g;
				e[m] = 0.;
			}
		}
		while( m != l );
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
