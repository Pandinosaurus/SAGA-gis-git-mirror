
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                       pj_proj4                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                crs_transform_point.cpp                //
//                                                       //
//                 Copyright (C) 2018 by                 //
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
#include "crs_transform_point.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CCRS_Transform_Point::CCRS_Transform_Point(void)
{
	Set_Name		(_TL("Single Coordinate Transformation"));

	Set_Author		("O.Conrad (c) 2018");

	Set_Description	(_TW(
		"Transformation of a single coordinate. "
		"Projections can be defined in different formats. Supported formats are:<ul>"
		"<li>proj strings</li>"
		"<li>WKT strings</li>"
		"<li>object codes (e.g. \"EPSG:4326\", \"ESRI:31493\", \"urn:ogc:def:crs:EPSG::4326\", \"urn:ogc:def:coordinateOperation:EPSG::1671\")</li>"
		"<li>object names (e.g. \"WGS 84\", \"WGS 84 / UTM zone 31N\", \"Germany_Zone_3\". In this case as uniqueness is not guaranteed, heuristics are applied to determine the appropriate best match.</li>"
		"<li>OGC URN combining references for compound CRS (e.g \"urn:ogc:def:crs,crs:EPSG::2393,crs:EPSG::5717\" or custom abbreviated syntax \"EPSG:2393+5717\")</li>"
		"<li>OGC URN combining references for concatenated operations (e.g. \"urn:ogc:def:coordinateOperation,coordinateOperation:EPSG::3895,coordinateOperation:EPSG::1618\")</li>"
		"<li>PROJJSON strings (find the jsonschema at <a href=\"https://proj.org/schemas/v0.4/projjson.schema.json\">proj.org</a>)</li>"
		"<li>compound CRS made from two object names separated with \" + \" (e.g. \"WGS 84 + EGM96 height\")</li>"
		"</ul>"
	));

	Set_Description	(Get_Description() + "\n" + CSG_CRSProjector::Get_Description());

	//-----------------------------------------------------
	CCRS_Picker	CRS_Picker;

	Parameters.Add_Node  (""      , "SOURCE"    , _TL("Source"    ), _TL(""));
	Parameters.Add_String("SOURCE", "SOURCE_CRS", _TL("Projection"), _TL(""), "EPSG:4326");
	Parameters.Add_Double("SOURCE", "SOURCE_X"  , _TL("X"         ), _TL(""));
	Parameters.Add_Double("SOURCE", "SOURCE_Y"  , _TL("Y"         ), _TL(""));

	Parameters.Add_Node  (""      , "TARGET"    , _TL("Target"    ), _TL(""));
	Parameters.Add_String("TARGET", "TARGET_CRS", _TL("Projection"), _TL(""), "EPSG:4326");
	Parameters.Add_Double("TARGET", "TARGET_X"  , _TL("X"         ), _TL(""));
	Parameters.Add_Double("TARGET", "TARGET_Y"  , _TL("Y"         ), _TL(""));

	Parameters.Add_String("", "SOURCE_WKT2", "WKT2", "", "")->Set_UseInGUI(false);
	Parameters.Add_String("", "SOURCE_PROJ", "PROJ", "", "")->Set_UseInGUI(false);
	Parameters.Add_String("", "TARGET_WKT2", "WKT2", "", "")->Set_UseInGUI(false);
	Parameters.Add_String("", "TARGET_PROJ", "PROJ", "", "")->Set_UseInGUI(false);

//	Parameters.Add_Parameters("", "PICKER", _TL("CRS Picker"), _TL(""))->asParameters()->Assign_Parameters(CRS_Picker.Get_Parameters());
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CCRS_Transform_Point::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( pParameter->Get_Parent() )
	{
		CSG_String Source(pParameter->Get_Parent()->Cmp_Identifier("SOURCE") ? "SOURCE_" : "TARGET_");
		CSG_String Target(pParameter->Get_Parent()->Cmp_Identifier("TARGET") ? "SOURCE_" : "TARGET_");

		double x = (*pParameters)(Source + "X")->asDouble();
		double y = (*pParameters)(Source + "Y")->asDouble();

		if( Transform(x, y,
			CSG_Projection((*pParameters)(Source + "CRS")->asString()),
			CSG_Projection((*pParameters)(Target + "CRS")->asString())) )
		{
			pParameters->Set_Parameter(Target + "X", x);
			pParameters->Set_Parameter(Target + "Y", y);
		}
	}

	if( pParameter->Cmp_Identifier("SOURCE_CRS") )
	{
		CSG_Projection Projection(pParameter->asString());

		pParameters->Set_Parameter("SOURCE_WKT2", Projection.Get_WKT2());
		pParameters->Set_Parameter("SOURCE_PROJ", Projection.Get_PROJ());
	}

	if( pParameter->Cmp_Identifier("TARGET_CRS") )
	{
		CSG_Projection Projection(pParameter->asString());

		pParameters->Set_Parameter("TARGET_WKT2", Projection.Get_WKT2());
		pParameters->Set_Parameter("TARGET_PROJ", Projection.Get_PROJ());
	}

	return( CSG_Tool::On_Parameter_Changed(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CCRS_Transform_Point::On_Execute(void)
{
	double x = Parameters("SOURCE_X")->asDouble();
	double y = Parameters("SOURCE_Y")->asDouble();

	CSG_Projection Source(Parameters("SOURCE_WKT2")->asString(), Parameters("SOURCE_PROJ")->asString());
	CSG_Projection Target(Parameters("TARGET_WKT2")->asString(), Parameters("TARGET_PROJ")->asString());

	if( Transform(x, y, Source, Target) )
	{
		Parameters.Set_Parameter("TARGET_X", x);
		Parameters.Set_Parameter("TARGET_Y", y);

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CCRS_Transform_Point::Transform(double &x, double &y, const CSG_Projection &Source, const CSG_Projection &Target)
{
	CSG_CRSProjector Projector;

	return( Projector.Set_Transformation(Source, Target) && Projector.Get_Projection(x, y) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
