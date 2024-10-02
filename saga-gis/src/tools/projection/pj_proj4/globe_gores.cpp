
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                       pj_proj                         //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   globe_gores.cpp                     //
//                                                       //
//                 Olaf Conrad (C) 2020                  //
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
#include "globe_gores.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGlobe_Gores::CGlobe_Gores(void)
{
	Set_Name(_TL("Globe Gores"));

	Set_Author("O.Conrad (c) 2020");

	Set_Description(_TW(
		"With this tool you can easily create globe gores from "
		"a (global) grid data set. The tool uses a polyconic "
		"projection for each gore and plots all gores side by "
		"side into one target grid. Target grid resolution is "
		"specified as number of pixels (cells) in meridional "
		"direction. The target grid is not suitable for further "
		"geoprocessing, but is thought to serve as print template "
		"for the creation of globes, i.e. to become glued onto "
		"a ball. "
	));

	//-----------------------------------------------------
	Parameters.Add_Grid("",
		"GRID"		, _TL("Grid"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid_Output("",
		"GORES"		, _TL("Gores"),
		_TL("")
	);

	Parameters.Add_Int("",
		"NUMBER"	, _TL("Number of Gores"),
		_TL(""),
		12, 2, true, 360, true
	);

	Parameters.Add_Double("",
		"BLEED"		, _TL("Bleed"),
		_TL("Overlap of gores given as percentage of the gore width."),
		0., 0., true, 10., true
	);

	Parameters.Add_Int("",
		"RESOLUTION", _TL("Resolution"),
		_TL("Number of cells/pixels from pole to pole (i.e. in North-South direction)."),
		1000, 100, true
	);

	Parameters.Add_Bool("",
		"BYTEWISE"	, _TL("Bytewise Interpolation"),
		_TL("To be used for RGB and CMYK coded values (i.e. images)."),
		false
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGlobe_Gores::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	return( CSG_Tool_Grid::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGlobe_Gores::On_Execute(void)
{
	m_pGores = NULL;

	int nGores = Parameters("NUMBER")->asInt();

	for(int iGore=0; iGore<nGores && Set_Progress(iGore, nGores); iGore++)
	{
		Process_Set_Text("%s [%d/%d]", _TL("Globe Gores"), iGore + 1, nGores);

		SG_UI_Progress_Lock(true);
		Add_Gore(iGore, nGores);
		SG_UI_Progress_Lock(false);
	}

	//-----------------------------------------------------
	if( m_pGores )
	{
		Parameters("GORES")->Set_Value(m_pGores);

		DataObject_Set_Parameters(m_pGores, Parameters("GRID")->asGrid());

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGlobe_Gores::Add_Gore(int iGore, int nGores)
{
	CSG_Tool *pTool = SG_Get_Tool_Library_Manager().Create_Tool("pj_proj4", 4);	// Coordinate Transformation (Grid)

	if(	pTool == NULL )
	{
		Error_Fmt("%s [%s]", _TL("failed to load tool"), _TL("Coordinate Transformation (Grid)"));

		return( false );
	}

	pTool->Set_Manager(NULL);

	//-----------------------------------------------------
	double    Bleed = 1. + Parameters("BLEED")->asDouble() / 100.;
	double    Width = Bleed * 360. / nGores;
	double Meridian = (iGore + 0.5) * 360. / nGores;
	int  Resolution = Parameters("RESOLUTION")->asInt();

	CSG_Grid Gore, Grid(CSG_Grid_System(180. / Resolution, Meridian - Width / 2., -90., Meridian + Width / 2., 90.));

	pTool->Set_Parameter("CRS_WKT"          , CSG_Projection::Get_GCS_WGS84().Get_WKT2());
	pTool->Set_Parameter("CRS_PROJ"         , CSG_Projection::Get_GCS_WGS84().Get_PROJ());
	pTool->Set_Parameter("SOURCE"           , Parameters("GRID"));
	pTool->Set_Parameter("GRID"             , &Grid);
	pTool->Set_Parameter("RESAMPLING"       , 3); // B-Spline Interpolation
//	pTool->Set_Parameter("DATA_TYPE"        , 8); // SG_Data_Type_Get_Name(SG_DATATYPE_Float) => _TL("4 byte floating point number")
	pTool->Set_Parameter("DATA_TYPE"        , SG_Data_Type_Get_Name(SG_DATATYPE_Float)); // => _TL("4 byte floating point number")
	pTool->Set_Parameter("BYTEWISE"         , Parameters("BYTEWISE"));
	pTool->Set_Parameter("TARGET_AREA"      , false);
	pTool->Set_Parameter("TARGET_DEFINITION", 1);

	if( !pTool->Execute() )
	{
		SG_Get_Tool_Library_Manager().Delete_Tool(pTool);

		return( false );
	}

	//-----------------------------------------------------
	const char *PROJ = "+proj=poly +ellps=sphere +lon_0=%f +x_0=%f";
	const char *WKT2 =
		"PROJCRS[\"<custom>\","
		"    BASEGEODCRS[\"<custom>\","
		"        DATUM[\"Unknown based on Normal Sphere (r=6370997) ellipsoid\","
		"            ELLIPSOID[\"Normal Sphere (r=6370997)\",6370997,0]],"
		"        UNIT[\"degree\",0.0174532925199433,"
		"            ID[\"EPSG\",9122]]],"
		"    CONVERSION[\"<custom>\","
		"        METHOD[\"American Polyconic\","
		"            ID[\"EPSG\",9818]],"
		"        PARAMETER[\"Latitude of natural origin\",0,"
		"            ID[\"EPSG\",8801]],"
		"        PARAMETER[\"Longitude of natural origin\",%f,"
		"            ID[\"EPSG\",8802]],"
		"        PARAMETER[\"False easting\",%f,"
		"            ID[\"EPSG\",8806]],"
		"        PARAMETER[\"False northing\",0,"
		"            ID[\"EPSG\",8807]]],"
		"    CS[Cartesian,2],"
		"        AXIS[\"(E)\",east],"
		"        AXIS[\"(N)\",north],"
		"        UNIT[\"metre\",1,"
		"            ID[\"EPSG\",9001]]]";

	double Easting = iGore * M_PI_360 * 6370997. / nGores;
	pTool->Set_Parameter("CRS_WKT"          , CSG_String::Format(WKT2, Meridian, Easting));
	pTool->Set_Parameter("CRS_PROJ"         , CSG_String::Format(PROJ, Meridian, Easting));
	pTool->Set_Parameter("SOURCE"           , &Grid);
	pTool->Set_Parameter("GRID"             , &Gore);
	pTool->Set_Parameter("TARGET_AREA"      , true);
	pTool->Set_Parameter("TARGET_DEFINITION", 0);

	if( !pTool->Execute() )
	{
		SG_Get_Tool_Library_Manager().Delete_Tool(pTool);

		return( false );
	}

	SG_Get_Tool_Library_Manager().Delete_Tool(pTool);

	Add_Gore(iGore, nGores, &Gore);

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGlobe_Gores::Add_Gore(int iGore, int nGores, CSG_Grid *pGore)
{
	if( !m_pGores )
	{
		m_pGores = Parameters("GORES")->asGrid();

		if( !m_pGores )
		{
			Parameters("GORES")->Set_Value(m_pGores = SG_Create_Grid());
		}

		m_pGores->Create(SG_DATATYPE_Float, nGores * pGore->Get_NX(), pGore->Get_NY());

		m_pGores->Fmt_Name("%s [%s]", _TL("Globe Gores"), Parameters("GRID")->asGrid()->Get_Name());

		m_pGores->Assign_NoData();
	}

	//-----------------------------------------------------
//	int xOff = iGore * pGore->Get_NX();
	int xOff = (int)(iGore * (double)m_pGores->Get_NX() / nGores);

	#pragma omp parallel for
	for(int y=0; y<pGore->Get_NY(); y++)
	{
		for(int x=0; x<pGore->Get_NX(); x++)
		{
			if( m_pGores->is_InGrid(xOff + x, y, false) && !pGore->is_NoData(x, y) )
			{
				m_pGores->Set_Value(xOff + x, y, pGore->asDouble(x, y));
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
