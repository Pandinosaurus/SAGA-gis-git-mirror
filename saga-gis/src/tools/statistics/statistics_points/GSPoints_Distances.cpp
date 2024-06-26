
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                   statistics_points                   //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                 GSPoints_Distances.cpp                //
//                                                       //
//                 Copyright (C) 2010 by                 //
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
#include "GSPoints_Distances.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGSPoints_Distances::CGSPoints_Distances(void)
{
	Set_Name		(_TL("Minimum Distance Analysis"));

	Set_Author		("O.Conrad (c) 2010");

	Set_Description(
		_TL("")
	);

	Parameters.Add_Shapes("", "POINTS"   , _TL("Points"                   ), _TL(""), PARAMETER_INPUT, SHAPE_TYPE_Point);
	Parameters.Add_Table ("", "TABLE"    , _TL("Minimum Distance Analysis"), _TL(""), PARAMETER_OUTPUT);
	Parameters.Add_Choice("", "DIMENSION", _TL("Distance Measurement"     ), _TL(""),
		CSG_String::Format("%s|%s",
			_TL("2-dimensional (planar)"),
			_TL("3-dimensional")
		), 0
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGSPoints_Distances::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( pParameter->Cmp_Identifier("POINTS") )
	{
		pParameters->Set_Enabled("DIMENSION", pParameter->asShapes() && pParameter->asShapes()->Get_Vertex_Type() != SG_VERTEX_TYPE_XY);
	}

	return( CSG_Tool::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGSPoints_Distances::On_Execute(void)
{
	CSG_Simple_Statistics s; CSG_Shapes *pPoints = Parameters("POINTS")->asShapes();

	if( pPoints->Get_Vertex_Type() == SG_VERTEX_TYPE_XY || Parameters("DIMENSION")->asInt() == 0 )
	{
		CSG_KDTree_2D Search(pPoints, 0);

		for(sLong iPoint=0; iPoint<pPoints->Get_Count() && Set_Progress(iPoint, pPoints->Get_Count()); iPoint++)
		{
			size_t Index[2]; double Distance[2]; CSG_Point p = pPoints->Get_Shape(iPoint)->Get_Point();

			if( Search.Get_Nearest_Points(p.x, p.y, 2, Index, Distance) && Distance[1] > 0. )
			{
				s += Distance[1];
			}
		}
	}
	else
	{
		CSG_KDTree_3D Search(pPoints, 0);

		for(sLong iPoint=0; iPoint<pPoints->Get_Count() && Set_Progress(iPoint, pPoints->Get_Count()); iPoint++)
		{
			size_t Index[2]; double Distance[2]; CSG_Point_3D p = pPoints->Get_Shape(iPoint)->Get_Point_Z();

			if( Search.Get_Nearest_Points(p.x, p.y, p.z, 2, Index, Distance) && Distance[1] > 0. )
			{
				s += Distance[1];
			}
		}
	}

	//-----------------------------------------------------
	if( s.Get_Count() > 0 )
	{
		CSG_Table *pTable = Parameters("TABLE")->asTable(); pTable->Destroy();

		if( pPoints->Get_Vertex_Type() == SG_VERTEX_TYPE_XY || Parameters("DIMENSION")->asInt() == 0 )
		{
			pTable->Fmt_Name("%s [%s]"   , _TL("Minimum Distance Analysis"), pPoints->Get_Name());
		}
		else
		{
			pTable->Fmt_Name("%s [3D %s]", _TL("Minimum Distance Analysis"), pPoints->Get_Name());
		}

		pTable->Add_Field("NAME" , SG_DATATYPE_String);
		pTable->Add_Field("VALUE", SG_DATATYPE_Double);

		#define SET_VALUE(name, value) { CSG_Table_Record &r = *pTable->Add_Record(); r.Set_Value(0, name); r.Set_Value(1, value); }

		SET_VALUE(_TL("Mean Average"      ), s.Get_Mean   ());
		SET_VALUE(_TL("Minimum"           ), s.Get_Minimum());
		SET_VALUE(_TL("Maximum"           ), s.Get_Maximum());
		SET_VALUE(_TL("Standard Deviation"), s.Get_StdDev ());
		SET_VALUE(_TL("Duplicates"        ), pPoints->Get_Count() - s.Get_Count());

		DataObject_Update(pTable, SG_UI_DATAOBJECT_SHOW_MAP);

		return( true );
	}

	Message_Dlg(_TL("not enough observations"));

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
