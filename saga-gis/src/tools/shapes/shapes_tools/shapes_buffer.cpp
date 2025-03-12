
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                     shapes_tools                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   Shapes_Buffer.cpp                   //
//                                                       //
//                 Copyright (C) 2008 by                 //
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
//                University of Hamburg                  //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "shapes_buffer.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CShapes_Buffer::CShapes_Buffer(void)
{
	Set_Name		(_TL("Shapes Buffer"));

	Set_Author		("O.Conrad, V.Wichmann (c) 2008-2025");

	Set_Description	(_TW(
		"The tool allows one to buffer point, line and polygon shapes layers.\n"
		"Uses the free and open source software library <b>Clipper</b> created by Angus Johnson."
	));

	Add_Reference("https://github.com/AngusJohnson/Clipper2/", SG_T("Clipper Homepage"));


	//-----------------------------------------------------
	Parameters.Add_Shapes("",
		"SHAPES"	, _TL("Shapes"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Shapes("",
		"BUFFER"	, _TL("Buffer"),
		_TL(""),
		PARAMETER_OUTPUT, SHAPE_TYPE_Polygon
	);

	Parameters.Add_Table_Field_or_Const("SHAPES",
		"DIST_FIELD", _TL("Buffer Distance"),
		_TL(""),
		100.0, 0.0, true
	);

	Parameters.Add_Double("DIST_FIELD",
		"DIST_SCALE", _TL("Scaling Factor for Attribute Value"),
		_TL(""),
		1.0, 0.0, true
	);

	Parameters.Add_Bool("",
		"DISSOLVE"	, _TL("Dissolve Buffers"),
		_TL(""),
		true
	);

	Parameters.Add_Int("",
		"NZONES"	, _TL("Number of Buffer Zones"),
		_TL(""),
		1, 1, true
	);

	Parameters.Add_Bool("",
		"POLY_INNER", _TL("Inner Buffer"),
		_TL(""),
		false
	);

	Parameters.Add_Double("",
		"DARC"		, _TL("Arc Vertex Distance [Degree]"),
		_TL(""),
		5.0, 0.01, true, 45.0, true
	);

#ifndef CLIPPER_ONE
	Parameters.Add_Choice("",
		"JOIN_TYPE"	, _TL("Join Type"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s",
            _TL("Square"),
			_TL("Bevel"),
			_TL("Round"),
            _TL("Miter")
        ), 2
	);
	
	Parameters.Add_Choice("",
		"END_TYPE"	, _TL("End Type"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s|%s",
            _TL("Polygon"),
			_TL("Joined"),
			_TL("Butt"),
			_TL("Square"),
            _TL("Round")
        ), 4
	);
	
#endif // !CLIPPER_ONE

}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CShapes_Buffer::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	pParameter->Cmp_Identifier("SHAPES") )
	{
		pParameters->Set_Enabled("POLY_INNER", pParameter->asShapes() && pParameter->asShapes()->Get_Type() == SHAPE_TYPE_Polygon);
#ifndef CLIPPER_ONE
		pParameters->Set_Enabled("JOIN_TYPE" , pParameter->asShapes() && pParameter->asShapes()->Get_Type() != SHAPE_TYPE_Point && pParameter->asShapes()->Get_Type() != SHAPE_TYPE_Points);
		pParameters->Set_Enabled("END_TYPE"  , pParameter->asShapes() && pParameter->asShapes()->Get_Type() == SHAPE_TYPE_Line);
#endif // !CLIPPER_ONE
	}

	if(	pParameter->Cmp_Identifier("NZONES") )
	{
		pParameters->Set_Enabled("DISSOLVE"  , pParameter->asInt() == 1);
	}

	if(	pParameter->Cmp_Identifier("DIST_FIELD") )
	{
		pParameters->Set_Enabled("DIST_SCALE", pParameter->asInt() >= 0);
	}

	return( CSG_Tool::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CShapes_Buffer::On_Execute(void)
{
	CSG_Shapes	*pShapes	= Parameters("SHAPES")->asShapes();
	CSG_Shapes	*pBuffers	= Parameters("BUFFER")->asShapes();

	int	nZones		= Parameters("NZONES"    )->asInt();
	int	Field		= Parameters("DIST_FIELD")->asInt();

	m_dArc			= Parameters("DARC"      )->asDouble() * M_DEG_TO_RAD;
	m_bPolyInner	= Parameters("POLY_INNER")->asBool() && pShapes->Get_Type() == SHAPE_TYPE_Polygon;

#ifndef CLIPPER_ONE
	m_JoinType		= TSG_Line_JoinType(Parameters("JOIN_TYPE" )->asInt());
	m_EndType		= TSG_Line_EndType (Parameters("END_TYPE"  )->asInt());
#endif // !CLIPPER_ONE

	//-----------------------------------------------------
	if( !pShapes->is_Valid() )
	{
		Message_Add(_TL("Invalid Shapes"));

		return( false );
	}

	if( Field < 0 && Parameters("DIST_FIELD")->asDouble() <= 0.0 )
	{
		Message_Add(_TL("Invalid Buffer Distance"));

		return( false );
	}

	//-----------------------------------------------------
	if( nZones == 1 )
	{
		Get_Buffers(pShapes, Field, pBuffers, 1.0, Parameters("DISSOLVE")->asBool());
	}

	//-----------------------------------------------------
	else if( nZones > 1 )
	{
		CSG_Shape	*pBuffer;
		CSG_Shapes	Buffers;

		pBuffers->Create(SHAPE_TYPE_Polygon);
		pBuffers->Add_Field("ID"  , SG_DATATYPE_Int   );
		pBuffers->Add_Field("ZONE", SG_DATATYPE_Double);

		double	dZone	= 1.0 / nZones;

		for(int iZone=0; iZone<nZones; iZone++)
		{
			Get_Buffers(pShapes, Field, &Buffers, (nZones - iZone) * dZone, true);

			if( iZone > 0 )
			{
				SG_Shape_Get_Difference(pBuffer, Buffers.Get_Shape(0)->asPolygon());
			}

			pBuffer	= pBuffers->Add_Shape(Buffers.Get_Shape(0));
			pBuffer	->Set_Value(0, (nZones - iZone) + 1);
			pBuffer	->Set_Value(1, (nZones - iZone) * dZone * 100.0);
		}
	}

	//-----------------------------------------------------
	pBuffers->Fmt_Name("%s [%s]", pShapes->Get_Name(), _TL("Buffer"));

	return( pBuffers->is_Valid() );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffers(CSG_Shapes *pShapes, int Field, CSG_Shapes *pBuffers, double _Scale, bool bDissolve)
{
	CSG_Shapes Part(SHAPE_TYPE_Polygon); CSG_Shape_Polygon *pPart = Part.Add_Shape()->asPolygon(), *pBuffer;

	double Distance = Parameters("DIST_FIELD")->asDouble() * _Scale;
	double Scale    = Parameters("DIST_SCALE")->asDouble() * _Scale;

	if( !bDissolve )
	{
		pBuffers->Create(SHAPE_TYPE_Polygon, NULL, pShapes);
	}
	else
	{
		pBuffers->Create(SHAPE_TYPE_Polygon);
		pBuffers->Add_Field(_TL("ID"), SG_DATATYPE_Int);
		pBuffer	= pBuffers->Add_Shape()->asPolygon();
	}

	//-----------------------------------------------------
	for(sLong iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape, pShapes->Get_Count()); iShape++)
	{
		CSG_Shape	*pShape	= pShapes->Get_Shape(iShape);

		if( Field < 0 || (Distance = Scale * pShape->asDouble(Field)) > 0.0 )
		{
			if( !bDissolve )
			{
				pBuffer	= pBuffers->Add_Shape(pShape, SHAPE_COPY_ATTR)->asPolygon();
			}

			if( pBuffer->Get_Part_Count() == 0 )
			{
				Get_Buffer(pShape, pBuffer, Distance);
			}
			else
			{
				Get_Buffer(pShape, pPart  , Distance);

				SG_Shape_Get_Union(pBuffer, pPart);

				pPart->Del_Parts();
			}
		}
	}

	//-----------------------------------------------------
	return( pBuffers->is_Valid() );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffer(CSG_Shape *pShape, CSG_Shape *pBuffer, double Distance)
{
	switch( pShape->Get_Type() )
	{
	case SHAPE_TYPE_Point  : return( Get_Buffer_Point  (pShape, pBuffer, Distance) );
	case SHAPE_TYPE_Points : return( Get_Buffer_Points (pShape, pBuffer, Distance) );
	case SHAPE_TYPE_Line   : return( Get_Buffer_Line   (pShape, pBuffer, Distance) );
	case SHAPE_TYPE_Polygon: return( Get_Buffer_Polygon(pShape, pBuffer, Distance) );

	default:
		return( false );
	}
}

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffer_Point(CSG_Shape *pPoint, CSG_Shape *pBuffer, double Distance)
{
	Add_Arc(pBuffer, pPoint->Get_Point(), Distance, 0.0, M_PI_360);

	return( true );
}

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffer_Points(CSG_Shape *pPoints, CSG_Shape *pBuffer, double Distance)
{
	CSG_Shapes Part(SHAPE_TYPE_Polygon); CSG_Shape_Polygon *pPart = Part.Add_Shape()->asPolygon();

	for(int iPart=0; iPart<pPoints->Get_Part_Count(); iPart++)
	{
		for(int iPoint=0; iPoint<pPoints->Get_Point_Count(iPart); iPoint++)
		{
			if( pBuffer->Get_Part_Count() == 0 )
			{
				Add_Arc(pBuffer, pPoints->Get_Point(iPoint), Distance, 0.0, M_PI_360);
			}
			else
			{
				Add_Arc(pPart  , pPoints->Get_Point(iPoint), Distance, 0.0, M_PI_360);

				SG_Shape_Get_Union(pBuffer, pPart);

				pPart->Del_Parts();
			}
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffer_Line(CSG_Shape *pLine, CSG_Shape *pBuffer, double Distance)
{
#ifndef CLIPPER_ONE
	return( SG_Shape_Get_Offset(pLine, Distance, m_dArc, pBuffer, m_JoinType, m_EndType) );
#else
	return( SG_Shape_Get_Offset(pLine, Distance, m_dArc, pBuffer) );
#endif
}

//---------------------------------------------------------
bool CShapes_Buffer::Get_Buffer_Polygon(CSG_Shape *pPolygon, CSG_Shape *pBuffer, double Distance)
{
	if( m_bPolyInner )
	{
#ifndef CLIPPER_ONE
		if(	SG_Shape_Get_Offset(pPolygon, -Distance, m_dArc, pBuffer, m_JoinType, m_EndType) )
#else
		if(	SG_Shape_Get_Offset(pPolygon, -Distance, m_dArc, pBuffer) )
#endif
		{
			SG_Shape_Get_Difference(pPolygon, pBuffer->asPolygon(), pBuffer);
		}
		else
		{
			pBuffer->Assign(pPolygon, false);
		}

		return( true );
	}

#ifndef CLIPPER_ONE
	return( SG_Shape_Get_Offset(pPolygon, Distance, m_dArc, pBuffer, m_JoinType, m_EndType) );
#else
	return( SG_Shape_Get_Offset(pPolygon, Distance, m_dArc, pBuffer) );
#endif
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline double CShapes_Buffer::Get_Direction(const TSG_Point &From, const TSG_Point &To)
{
	double	dx	= To.x - From.x;
	double	dy	= To.y - From.y;

	return(	dx != 0.0 ?	M_PI_180 + atan2(dy, dx)
		: ( dy  > 0.0 ?	M_PI_270
		: ( dy  < 0.0 ?	M_PI_090 : 0.0) )
	);
}

//---------------------------------------------------------
inline bool CShapes_Buffer::Get_Parallel(const TSG_Point &A, const TSG_Point &B, TSG_Point AB[2], double Distance)
{
	double d = SG_Get_Distance(A, B);

	if( d > 0. )
	{
		d = Distance / d;

		CSG_Point C(
			d * (A.y - B.y),
			d * (B.x - A.x)
		);

		AB[0].x = A.x + C.x;
		AB[0].y = A.y + C.y;

		AB[1].x = B.x + C.x;
		AB[1].y = B.y + C.y;

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline void CShapes_Buffer::Add_Vertex(CSG_Shape *pBuffer, const TSG_Point &Center, double Distance, double theta)
{
	pBuffer->Add_Point(
		Center.x + Distance * cos(theta),
		Center.y + Distance * sin(theta)
	);
}

//---------------------------------------------------------
inline void CShapes_Buffer::Add_Arc(CSG_Shape *pBuffer, const TSG_Point &Center, double Distance, double alpha, double beta)
{
	double	theta;

	if( alpha < beta )
	{
		for(theta=alpha; theta<beta; theta+=m_dArc)
		{
			Add_Vertex(pBuffer, Center, Distance, theta);
		}
	}
	else
	{
		for(theta=alpha; theta>beta; theta-=m_dArc)
		{
			Add_Vertex(pBuffer, Center, Distance, theta);
		}
	}

	Add_Vertex(pBuffer, Center, Distance, beta);
}

//---------------------------------------------------------
inline void CShapes_Buffer::Add_Arc(CSG_Shape *pBuffer, const TSG_Point &Center, double Distance, const TSG_Point &A, const TSG_Point &B)
{
	double	alpha, beta;

	alpha	= Get_Direction(A, Center);
	beta	= Get_Direction(B, Center);

	if( alpha - beta >= M_PI_180 )
	{
		beta	+= M_PI_360;
	}

	Add_Arc(pBuffer, Center, Distance, alpha, beta);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
