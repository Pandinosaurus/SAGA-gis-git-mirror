
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                     io_grid_image                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    grid_to_kml.cpp                    //
//                                                       //
//                 Copyright (C) 2014 by                 //
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
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>

//---------------------------------------------------------
#include "grid_to_kml.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGrid_to_KML::CGrid_to_KML(void)
{
	Set_Name		(_TL("Export Grid to KML"));

	Set_Author		("O.Conrad (c) 2014");

	Set_Description	(_TW(
		"Uses 'Export Image' tool to create the image file. Automatically projects raster to geographic "
		"coordinate system, if its projection is known and not geographic. "
	));

	Parameters.Add_Grid("", "GRID" , _TL("Grid" ), _TL(""), PARAMETER_INPUT);
	Parameters.Add_Grid("", "SHADE", _TL("Shade"), _TL(""), PARAMETER_INPUT_OPTIONAL);

	Parameters.Add_FilePath("",
		"FILE"     , _TL("File"),
		_TL(""),
		CSG_String::Format("%s (*.kmz)|*.kmz|%s (*.kml)|*.kml",
			_TL("Compressed Keyhole Markup Language Files"),
			_TL("Keyhole Markup Language Files")
		), NULL, true
	);

	Parameters.Add_Choice("",
		"FORMAT"   , _TL("Image Format"),
		_TL(""),
		CSG_String::Format("%s|%s|%s|%s|%s",
			_TL("Portable Network Graphics"),
			_TL("JPEG - JFIF Compliant"    ),
			_TL("Tagged Image File Format" ),
			_TL("Windows or OS/2 Bitmap"   ),
			_TL("Zsoft Paintbrush"         )
		), 1
	);

	if( has_GUI() )
	{
		Parameters.Add_Choice("",
			"COLOURING"  , _TL("Colors"),
			_TL(""),
			CSG_String::Format("%s|%s|%s|%s|%s|%s",
				_TL("stretch to grid's standard deviation"),
				_TL("stretch to grid's value range"),
				_TL("stretch to specified value range"),
				_TL("lookup table"),
				_TL("rgb coded values"),
				_TL("same as in graphical user interface")
			), 5
		);

		Parameters.Add_Colors("",
			"COL_PALETTE", _TL("Color Ramp"),
			_TL("")
		);
	}
	else
	{
		Parameters.Add_Choice("",
			"COLOURING"		, _TL("Colors"),
			_TL(""),
			CSG_String::Format("%s|%s|%s|%s|%s",
				_TL("stretch to grid's standard deviation"),
				_TL("stretch to grid's value range"),
				_TL("stretch to specified value range"),
				_TL("lookup table"),
				_TL("rgb coded values")
			), 0
		);

		Parameters.Add_Choice("",
			"COL_PALETTE"	, _TL("Color Ramp"),
			_TL(""),
			"DEFAULT"        "|DEFAULT_BRIGHT"  "|BLACK_WHITE"    "|BLACK_RED"     "|"
			"BLACK_GREEN"    "|BLACK_BLUE"      "|WHITE_RED"      "|WHITE_GREEN"   "|"
			"WHITE_BLUE"     "|YELLOW_RED"      "|YELLOW_GREEN"   "|YELLOW_BLUE"   "|"
			"RED_GREEN"      "|RED_BLUE"        "|GREEN_BLUE"     "|RED_GREY_BLUE" "|"
			"RED_GREY_GREEN" "|GREEN_GREY_BLUE" "|RED_GREEN_BLUE" "|RED_BLUE_GREEN""|"
			"GREEN_RED_BLUE" "|RAINBOW"         "|NEON"           "|TOPOGRAPHY"    "|"
			"ASPECT_1"       "|ASPECT_2"        "|ASPECT_3"       "|"
		);

		Parameters.Add_Int ("", "COL_COUNT" , _TL("Number of Colors"), _TL(""), 100);
		Parameters.Add_Bool("", "COL_REVERT", _TL("Invert Ramp"     ), _TL(""), false);
	}

	Parameters.Add_Double("",
		"STDDEV"   , _TL("Standard Deviation"),
		_TL(""),
		2., 0., true
	);

	Parameters.Add_Range("",
		"STRETCH"  , _TL("Stretch to Value Range"),
        _TL(""),
        0., 100.
    );

	//-----------------------------------------------------
	CSG_Table *pLUT = Parameters.Add_FixedTable("", "LUT", _TL("Lookup Table"), _TL(""))->asTable();

	pLUT->Add_Field("Color"      , SG_DATATYPE_Color );
	pLUT->Add_Field("Name"       , SG_DATATYPE_String);
	pLUT->Add_Field("Description", SG_DATATYPE_String);
	pLUT->Add_Field("Minimum"    , SG_DATATYPE_Double);
	pLUT->Add_Field("Maximum"    , SG_DATATYPE_Double);

	pLUT->Add_Record();

	//-----------------------------------------------------
	Parameters.Add_Bool("",
		"RESAMPLING", _TL("Interpolation"),
		_TL("Resampling method used when projection is needed"),
		true
	);

	if( !has_GUI() )
	{
		Parameters.Add_Range("",
			"SHADE_BRIGHT", _TL("Shade Brightness"),
			_TL("Allows one to scale shade brightness [percent]"),
			0., 100., 0., true, 100., true
		);
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGrid_to_KML::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	pParameter->Cmp_Identifier("COLOURING") )
	{
		pParameters->Set_Enabled("COL_PALETTE", pParameter->asInt() <= 2);
		pParameters->Set_Enabled("STDDEV"     , pParameter->asInt() == 0);
		pParameters->Set_Enabled("STRETCH"    , pParameter->asInt() == 2);
		pParameters->Set_Enabled("LUT"        , pParameter->asInt() == 3);
	}

	if(	pParameter->Cmp_Identifier("SHADE") && pParameters->Get_Parameter("SHADE_BRIGHT") )
	{
		pParameters->Set_Enabled("SHADE_BRIGHT", pParameter->asPointer() != NULL);
	}

	if(	pParameter->Cmp_Identifier("GRID") || pParameter->Cmp_Identifier("COLOURING") )
	{
		CSG_Grid *pGrid = pParameters->Get_Parameter("GRID")->asGrid();

		pParameters->Set_Enabled("RESAMPLING",
			pGrid && pGrid->Get_Projection().Get_Type() == ESG_CRS_Type::Projection && pParameters->Get_Parameter("COLOURING")->asInt() < 4
		);
	}

	return( CSG_Tool_Grid::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_to_KML::On_Execute(void)
{
	bool bDelete = false;

	CSG_Grid *pGrid  = Parameters("GRID" )->asGrid(), Image;
	CSG_Grid *pShade = Parameters("SHADE")->asGrid();

	//-----------------------------------------------------
	int Method = Parameters("COLOURING")->asInt();

	if( Method == 5 ) // same as in graphical user interface
	{
		if( !SG_UI_DataObject_asImage(pGrid, &Image) )
		{
			Error_Set("could not retrieve colour coding from graphical user interface.");

			return( false );
		}

		Image.Set_Name       (pGrid->Get_Name       ());
		Image.Set_Description(pGrid->Get_Description());
		Image.Flip();
		pGrid  = &Image;
		Method = 4; // rgb coded values
	}

	//-----------------------------------------------------
	if( pGrid->Get_Projection().Get_Type() == ESG_CRS_Type::Undefined )
	{
		Message_Add(_TL("layer uses undefined coordinate system, assuming geographic coordinates"));
	}
	else if( !(pGrid->Get_Projection().is_Geographic() || pGrid->Get_Projection().is_Geodetic()) )
	{
		Message_Fmt("\n%s (%s: %s)\n", _TL("re-projection to geographic coordinates"), _TL("original"), pGrid->Get_Projection().Get_Name().c_str());

		CSG_Tool *pTool = SG_Get_Tool_Library_Manager().Create_Tool("pj_proj4", 4);

		if(	pTool == NULL )	// Coordinate Transformation (Grid)
		{
			return( false );
		}

		pTool->Set_Manager(NULL);

		if( pTool->Set_Parameter("CRS_STRING", CSG_Projection::Get_GCS_WGS84().Get_WKT())
		&&  pTool->Set_Parameter("RESAMPLING", Parameters("RESAMPLING")->asBool() ? 3 : 0)
		&&  pTool->Set_Parameter("BYTEWISE"  , Method == 4)
		&&  pTool->Set_Parameter("SOURCE"    , pGrid)
		&&  pTool->Execute() )
		{
			bDelete = true; pGrid = pTool->Get_Parameter("GRID")->asGrid();

			if( pShade && pTool->Set_Parameter("SOURCE", pShade) && pTool->Execute() )
			{
				pShade = pTool->Get_Parameter("GRID")->asGrid();
			}
			else
			{
				pShade = NULL;
			}
		}

		SG_Get_Tool_Library_Manager().Delete_Tool(pTool);

		if( !bDelete )
		{
			Message_Fmt("\n%s: %s\n", _TL("re-projection"), _TL("failed"));

			return( false );
		}
	}

	//-----------------------------------------------------
	CSG_Tool *pTool = SG_Get_Tool_Library_Manager().Create_Tool("io_grid_image", 0, has_GUI());

	if(	pTool == NULL )	// Export Image
	{
		return( false );
	}

	bool bResult = false;

	CSG_String File(Parameters("FILE")->asString());

	switch( Parameters("FORMAT")->asInt() )
	{
		default: SG_File_Set_Extension(File, "png"); break; // Portable Network Graphics
		case  1: SG_File_Set_Extension(File, "jpg"); break; // JPEG - JFIF Compliant"    ),
		case  2: SG_File_Set_Extension(File, "tif"); break; // Tagged Image File Format" ),
		case  3: SG_File_Set_Extension(File, "bmp"); break; // Windows or OS/2 Bitmap"   ),
		case  4: SG_File_Set_Extension(File, "pcx"); break; // Zsoft Paintbrush"         )
	}

	pTool->Set_Manager(NULL);

	if( pTool->Set_Parameter("GRID"        , pGrid )
	&&  pTool->Set_Parameter("SHADE"       , pShade)
	&&  pTool->Set_Parameter("COLOURING"   , Method)
	&&  pTool->Set_Parameter("FILE_KML"    , true  )
	&&  pTool->Set_Parameter("FILE"        , File  )
	&&  pTool->Set_Parameter("COL_PALETTE" , Parameters("COL_PALETTE" ))
	&&  pTool->Set_Parameter("STDDEV"      , Parameters("STDDEV"      ))
	&&  pTool->Set_Parameter("STRETCH"     , Parameters("STRETCH"     ))
	&&  pTool->Set_Parameter("LUT"         , Parameters("LUT"         ))
	&&  (has_GUI() || !Parameters("SHADE_BRIGHT")
	||  pTool->Set_Parameter("SHADE_BRIGHT", Parameters("SHADE_BRIGHT")))
	&&  pTool->Execute() )
	{
		bResult = true;
	}

	SG_Get_Tool_Library_Manager().Delete_Tool(pTool);

	//-----------------------------------------------------
	if( bDelete )
	{
		delete(pGrid);

		if( pShade )
		{
			delete(pShade);
		}
	}

	//-----------------------------------------------------
	File = Parameters("FILE")->asString();

	if( SG_File_Cmp_Extension(File, "kmz") )
	{
		wxDir dir; wxString file; CSG_String Path(SG_File_Get_Path(File));

		if( dir.Open(Path.c_str()) && dir.GetFirst(&file, wxString::Format("%s.*", SG_File_Get_Name(File, false).c_str()), wxDIR_FILES) )
		{
			CSG_Strings Files;

			do
			{
				if( SG_File_Cmp_Extension(&file, "kmz") )
				{
					SG_File_Delete(SG_File_Make_Path(Path, &file));
				}
				else
				{
					Files += SG_File_Make_Path(Path, &file);
				}
			}
			while( dir.GetNext(&file) );

			dir.Close();

			//---------------------------------------------
			wxZipOutputStream Zip(new wxFileOutputStream(File.c_str()));

			for(int i=0; i<Files.Get_Count(); i++)
			{
				wxFileInputStream *pInput = new wxFileInputStream(Files[i].c_str());
				
				if( pInput != NULL )
				{
					Zip.PutNextEntry(SG_File_Get_Name(Files[i], true).c_str());
					Zip.Write(*pInput);
					delete(pInput);
				}

				SG_File_Delete(Files[i]);
			}
		}
	}

	//-----------------------------------------------------
	return( bResult );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGrid_from_KML::CGrid_from_KML(void)
{
	Set_Name		(_TL("Import Grids from KML"));

	Set_Author		("O.Conrad (c) 2015");

	Set_Description	(_TW(
		"Uses 'Import Image' tool to load the ground overlay image files associated with the kml."
	));

	Parameters.Add_Grid_List("",
		"GRIDS"		, _TL("Grids"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_FilePath("",
		"FILE"		, _TL("KML/KMZ File"),
		_TL(""),
		CSG_String::Format("%s|*.kml;*.kmz|%s|*.*",
			_TL("KML/KMZ Files"),
			_TL("All Files"    )
		), NULL, false
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_from_KML::On_Execute(void)
{
	//-----------------------------------------------------
	wxString	Dir, File	= Parameters("FILE")->asString();

	//-----------------------------------------------------
	bool	bKMZ	= SG_File_Cmp_Extension(&File, "kmz");

	if( !bKMZ )
	{
		Dir	= SG_File_Get_Path(&File).c_str();
	}
	else	// unzip to temporary directory
	{
		Dir	= wxFileName::CreateTempFileName("kml_");

		wxRemoveFile(Dir);
		wxFileName::Mkdir(Dir);

		wxZipEntry			*pEntry;
		wxZipInputStream	Zip(new wxFileInputStream(File));

		while( (pEntry = Zip.GetNextEntry()) != NULL )
		{
			wxFileName	fn(Dir, pEntry->GetName());

			wxFileOutputStream	*pOutput	= new wxFileOutputStream(fn.GetFullPath());

			pOutput->Write(Zip);

			delete(pOutput);
			delete(pEntry);

			if( !fn.GetExt().CmpNoCase("kml") )
			{
				File	= fn.GetFullPath();
			}
		}
	}

	//-----------------------------------------------------
	CSG_MetaData	KML;

	if( !KML.Load(&File) )
	{
		Error_Fmt("%s [%s]", _TL("failed to load file"), File.wx_str());

		return( false );
	}

	//-----------------------------------------------------
	m_pGrids	= Parameters("GRIDS")->asGridList();
	m_pGrids->Del_Items();

	Load_KML(Dir, KML);

	//-----------------------------------------------------
	if( bKMZ && wxDirExists(Dir) )
	{
		wxFileName::Rmdir(Dir, wxPATH_RMDIR_FULL|wxPATH_RMDIR_RECURSIVE);
	}

	//-----------------------------------------------------
	return( m_pGrids->Get_Grid_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_from_KML::Load_KML(const SG_Char *Dir, const CSG_MetaData &KML)
{
	for(int i=0; i<KML.Get_Children_Count(); i++)
	{
		if( KML.Get_Child(i)->Cmp_Name("GroundOverlay") )
		{
			Load_Overlay(Dir, *KML.Get_Child(i));
		}
		else
		{
			Load_KML    (Dir, *KML.Get_Child(i));
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CGrid_from_KML::Load_Overlay(const SG_Char *Dir, const CSG_MetaData &KML)
{
	//-----------------------------------------------------
	if( !KML("Icon") || !KML["Icon"]("HRef") )
	{
		Error_Set(_TL("missing icon tags"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Rect	r;

	if(    !KML("LatLonBox")
		|| !KML["LatLonBox"]("North") || !KML["LatLonBox"]["North"].Get_Content().asDouble(r.yMax)
		|| !KML["LatLonBox"]("South") || !KML["LatLonBox"]["South"].Get_Content().asDouble(r.yMin)
		|| !KML["LatLonBox"]("East" ) || !KML["LatLonBox"]["East" ].Get_Content().asDouble(r.xMax)
		|| !KML["LatLonBox"]("West" ) || !KML["LatLonBox"]["West" ].Get_Content().asDouble(r.xMin) )
	{
		Error_Set(_TL("failed to load georeference for KML ground overlay"));

		return( false );
	}

	//-----------------------------------------------------
	wxFileName	fn(KML["Icon"]["HRef"].Get_Content().c_str());

	if( !fn.FileExists() )
	{
		fn.SetPath(Dir);
	}

	CSG_Data_Manager Manager; CSG_String FullPath = fn.GetFullPath().wx_str();

	if( !Manager.Add(FullPath) || !Manager.Grid().Count() )
	{
		Error_Fmt("%s: %s", _TL("failed to load KML ground overlay icon"), fn.GetFullPath().wx_str());
	}

	//-----------------------------------------------------
	CSG_Grid *pIcon = Manager.Grid(0).asGrid();

	CSG_Grid *pGrid = SG_Create_Grid(pIcon->Get_Type(), pIcon->Get_NX(), pIcon->Get_NY(), r.Get_YRange() / (pIcon->Get_NY() - 1), r.Get_XMin(), r.Get_YMin());

	if( KML("Name") && !KML["Name"].Get_Content().is_Empty() )
		pGrid->Set_Name(KML["Name"].Get_Content());

	if( KML("Description") && !KML["Description"].Get_Content().is_Empty() )
		pGrid->Set_Name(KML["Description"].Get_Content());

	pGrid->Get_Projection().Set_GCS_WGS84();

	#pragma omp parallel for
	for(int y=0; y<pGrid->Get_NY(); y++)
	{
		for(int x=0; x<pGrid->Get_NX(); x++)
		{
			pGrid->Set_Value(x, y, pIcon->asDouble(x, y));
		}
	}

	m_pGrids->Add_Item(pGrid);

	DataObject_Add(pGrid);
	DataObject_Set_Parameter(pGrid, "COLORS_TYPE", 5);	// Color Classification Type: RGB Coded Values

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
