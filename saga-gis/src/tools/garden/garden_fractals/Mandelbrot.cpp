/**********************************************************
 * Version $Id: Mandelbrot.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                       Fractals                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    Mandelbrot.cpp                     //
//                                                       //
//                 Copyright (C) 2003 by                 //
//                      Olaf Conrad                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation, either version 2 of the     //
// License, or (at your option) any later version.       //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not, see   //
// <http://www.gnu.org/licenses/>.                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "Mandelbrot.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CMandelbrot::CMandelbrot(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name(_TL("Mandelbrot Set"));

	Set_Author		(SG_T("(c) 2001 by O.Conrad"));

	Set_Description	(_TW(
		"Calculates Mandelbrot and Julia sets.\n\n"
		"References:\n"
		"- Mandelbrot, B.B. (1983): 'The Fractal Geometry of Nature', New York, 490p.\n")
	);


	//-----------------------------------------------------
	// 2. Grids...

	Parameters.Add_Grid_Output(
		NULL	, "GRID"	, _TL("Grid"),
		_TL("")
	);

	Parameters.Add_Value(
		NULL	, "NX"		, _TL("Width (Cells)"),
		_TL(""),
		PARAMETER_TYPE_Int, 100, 1, true
	);

	Parameters.Add_Value(
		NULL	, "NY"		, _TL("Height (Cells)"),
		_TL(""), PARAMETER_TYPE_Int, 100, 1, true
	);

	Parameters.Add_Range(
		NULL	, "XRANGE"	, _TL("X-Range"),
		_TL(""), -2.0, 1.0
	);

	Parameters.Add_Range(
		NULL	, "YRANGE"	, _TL("Y-Range"),
		_TL(""),
		-1.5, 1.5
	);

	Parameters.Add_Value(
		NULL	, "JULIA_X"	, _TL("Julia - X"),
		_TL(""), PARAMETER_TYPE_Double, -0.7
	);

	Parameters.Add_Value(
		NULL	, "JULIA_Y"	, _TL("Julia - Y"),
		_TL(""), PARAMETER_TYPE_Double,  0.3
	);

	Parameters.Add_Value(
		NULL	, "MAXITER"	, _TL("Maximum Iterations"),
		_TL(""),
		PARAMETER_TYPE_Int, 300, 1, true
	);

	Parameters.Add_Choice(
		NULL	, "METHOD"	, _TL("Fractal Type"),
		_TL(""),

		CSG_String::Format(SG_T("%s|%s|"),
			_TL("Mandelbrot"),
			_TL("Julia")
		), 0
	);
}

//---------------------------------------------------------
CMandelbrot::~CMandelbrot(void)
{}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CMandelbrot::On_Execute(void)
{
	//-----------------------------------------------------
	m_Extent.Assign(
		Parameters("XRANGE")->asRange()->Get_Min(),
		Parameters("YRANGE")->asRange()->Get_Min(),
		Parameters("XRANGE")->asRange()->Get_Max(),
		Parameters("YRANGE")->asRange()->Get_Max()
	);

	m_maxIterations	= Parameters("MAXITER")	->asInt();
	m_maxDistance	= 4.0;

	m_Method		= Parameters("METHOD")	->asInt();

	m_xJulia		= Parameters("JULIA_X")	->asDouble();
	m_yJulia		= Parameters("JULIA_Y")	->asDouble();

	m_pGrid			= SG_Create_Grid(SG_DATATYPE_Int, Parameters("NX")->asInt(), Parameters("NY")->asInt());
	m_pGrid->Set_Name(m_Method == 0 ? _TL("Mandelbrot Set") : _TL("Julia Set"));
	Parameters("GRID")->Set_Value(m_pGrid);

	//-----------------------------------------------------
	Calculate();

	//-----------------------------------------------------
	return( true );
}

//---------------------------------------------------------
#define GET_POS(p)		p.Assign(\
						m_Extent.Get_XMin() + m_Extent.Get_XRange() * (ptWorld.Get_X() - m_pGrid->Get_XMin()) / m_pGrid->Get_XRange(),\
						m_Extent.Get_YMin() + m_Extent.Get_YRange() * (ptWorld.Get_Y() - m_pGrid->Get_YMin()) / m_pGrid->Get_YRange());

#define SET_POS(a, b)	if( a.Get_X() > b.Get_X() )	{	d	= a.Get_X(); a.Set_X(b.Get_X()); b.Set_X(d);	}\
						if( a.Get_Y() > b.Get_Y() )	{	d	= a.Get_Y(); a.Set_Y(b.Get_Y()); b.Set_Y(d);	}

//---------------------------------------------------------
bool CMandelbrot::On_Execute_Position(CSG_Point ptWorld, TSG_Tool_Interactive_Mode Mode)
{
	double	d;

	switch( Mode )
	{
	default:
		break;

	case TOOL_INTERACTIVE_LDOWN:
	case TOOL_INTERACTIVE_RDOWN:
		GET_POS(m_Down);

		return( true );

	case TOOL_INTERACTIVE_LUP:
		GET_POS(m_Up);
		SET_POS(m_Up, m_Down);

		if( m_Up.Get_X() >= m_Down.Get_X() || m_Up.Get_Y() >= m_Down.Get_Y() )
		{
			m_Extent.Inflate(50.0);
			m_Extent.Move(m_Up - m_Extent.Get_Center());
		}
		else
		{
			m_Extent.Assign(m_Up, m_Down);
		}

		Calculate();

		return( true );

	case TOOL_INTERACTIVE_RUP:
		GET_POS(m_Up);
		SET_POS(m_Up, m_Down);

		if( m_Up.Get_X() >= m_Down.Get_X() || m_Up.Get_Y() >= m_Down.Get_Y() )
		{
			m_Extent.Deflate(50.0);
			m_Extent.Move(m_Up - m_Extent.Get_Center());
		}
		else
		{
			m_Extent.Deflate(100.0 * (m_Down.Get_X() - m_Up.Get_X()) / m_Extent.Get_XRange());
			m_Extent.Move(m_Up - m_Extent.Get_Center());
		}

		Calculate();

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CMandelbrot::Calculate(void)
{
	int		x, y, i;
	double	xPos, yPos, dx, dy;

	dx	= m_Extent.Get_XRange() / (m_pGrid->Get_NX() - 1.0);
	dy	= m_Extent.Get_YRange() / (m_pGrid->Get_NY() - 1.0);

	for(y=0, yPos=m_Extent.Get_YMin(); y<m_pGrid->Get_NY() && Set_Progress(y, m_pGrid->Get_NY()); y++, yPos+=dy)
	{
		for(x=0, xPos=m_Extent.Get_XMin(); x<m_pGrid->Get_NX(); x++, xPos+=dx)
		{
			switch( m_Method )
			{
			default:
			case 0:	i	= Get_Mandelbrot	(xPos, yPos);	break;
			case 1:	i	= Get_Julia			(xPos, yPos);	break;
			}

			if( i >= m_maxIterations )
			{
				m_pGrid->Set_NoData(x, y);
			}
			else
			{
				m_pGrid->Set_Value(x, y, i);
			}
		}
	}

	DataObject_Update(m_pGrid, true);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CMandelbrot::Get_Mandelbrot(double xPos, double yPos)
{
	int		i;
	double	x, y, k;

	for(i=0, x=0.0, y=0.0; i<m_maxIterations; i++)
	{
		k	= xPos + x*x - y*y;
		y	= yPos + 2.0 * x * y;
		x	= k;

		if( m_maxDistance < x*x + y*y )
		{
			return( i );
		}
	}

	return( i );
}

//---------------------------------------------------------
int CMandelbrot::Get_Julia(double xPos, double yPos)
{
	int		i;
	double	x, y, k;

	for(i=0, x=xPos, y=yPos; i<m_maxIterations; i++)
	{
		k	= m_xJulia + x*x - y*y;
		y	= m_yJulia + 2.0 * x * y;
		x	= k;

		if( m_maxDistance < x*x + y*y )
		{
			return( i );
		}
	}

	return( i );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
