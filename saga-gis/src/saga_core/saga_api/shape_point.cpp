
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                    Library: Shapes                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    shape_point.cpp                    //
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
#include "shapes.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Shape_Point::CSG_Shape_Point(CSG_Shapes *pOwner, sLong Index)
	: CSG_Shape(pOwner, Index)
{}

//---------------------------------------------------------
CSG_Shape_Point::~CSG_Shape_Point(void)
{}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Shape_Point::On_Assign(CSG_Shape *pShape)
{
	if( pShape->Get_Point_Count(0) > 0 )
	{
		Set_Point(pShape->Get_Point());
		Set_Z    (pShape->Get_Z    ());
		Set_M    (pShape->Get_M    ());

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
const CSG_Rect & CSG_Shape_Point::Get_Extent(void)
{
	static CSG_Rect	Extent;

	Extent.Assign(m_Point.x, m_Point.y, m_Point.x, m_Point.y);

	return( Extent );
}

//---------------------------------------------------------
TSG_Intersection CSG_Shape_Point::On_Intersects(CSG_Shape *pShape)
{
	return( INTERSECTION_None );
}

//---------------------------------------------------------
TSG_Intersection CSG_Shape_Point::On_Intersects(TSG_Rect Extent)
{
	if(	Extent.xMin <= m_Point.x && m_Point.x <= Extent.xMax
	&&	Extent.yMin <= m_Point.y && m_Point.y <= Extent.yMax )
	{
		return( INTERSECTION_Overlaps );
	}

	return( INTERSECTION_None );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
