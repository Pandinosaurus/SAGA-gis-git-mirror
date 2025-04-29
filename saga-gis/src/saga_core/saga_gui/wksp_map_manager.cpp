
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    User Interface                     //
//                                                       //
//                    Program: SAGA                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                 WKSP_Map_Manager.cpp                  //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
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
#include <saga_api/saga_api.h>

#include <wx/fs_mem.h>

#include "res_commands.h"
#include "res_controls.h"
#include "res_dialogs.h"

#include "helper.h"
#include "saga_frame.h"

#include "wksp_data_manager.h"
#include "wksp_map_buttons.h"

#include "wksp_layer.h"

#include "wksp_map_control.h"
#include "wksp_map_manager.h"
#include "wksp_map.h"
#include "wksp_map_layer.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Map_Manager *g_pMaps = NULL;


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Map_Manager::CWKSP_Map_Manager(void)
{
	wxMemoryFSHandler::AddFile("preview_map.png", "dummy");

	g_pMaps = this; m_Numbering = 0; m_Parameters.Set_Name(_TL("Maps"));

	//-----------------------------------------------------
	m_Parameters.Add_Bool (""          , "THUMBNAILS"        , _TL("Thumbnails"         ), _TL(""), true);
	m_Parameters.Add_Int  ("THUMBNAILS", "THUMBNAIL_SIZE"    , _TL("Size"               ), _TL(""), 75, 10, true);
	m_Parameters.Add_Color("THUMBNAILS", "THUMBNAIL_SELCOLOR", _TL("Selection Color"    ), _TL(""), Get_Color_asInt(SYS_Get_Color(wxSYS_COLOUR_BTNSHADOW)));

	m_Parameters.Add_Node (""          , "NODE_DESC"         , _TL("Description"        ), _TL(""));
	m_Parameters.Add_Bool ("NODE_DESC" , "PREVIEW"           , _TL("Preview"            ), _TL("Show a preview in description."), false);
	m_Parameters.Add_Int  ("PREVIEW"   , "PREVIEW_SIZE"      , _TL("Size"               ), _TL(""), 200, 10, true);

	m_Parameters.Add_Int  (""          , "NUMBERING"         , _TL("Map Numeration"     ),
		_TL("Minimum width of map numbering. If set to 0 no numbering is applied at all. Negative values will prepend zeros. Having many maps numbering helps in unambiguos map identification."),
		2
	);

	//-----------------------------------------------------
	m_Parameters.Add_Choice("",
		"CROSSHAIR"	, _TL("Cross Hair"),
		_TL("Display a cross hair of a map's current mouse position in all maps."),
		CSG_String::Format("%s|%s|%s",
			_TL("no"),
			_TL("yes"),
			_TL("projected")
		), 0
	);

	m_Parameters.Add_Bool("",
		"CACHE"		, _TL("Base Map Caching"),
		_TL("Enables local disk caching for base maps."),
		false
	);

	m_Parameters.Add_FilePath("CACHE",
		"CACHE_DIR"	, _TL("Cache Directory"),
		_TL("If not specified the cache will be created in the current user's temporary directory."),
		NULL, NULL, false, true
	);

	//-----------------------------------------------------
	m_Parameters.Add_Node  ("", "NODE_DEFAULTS", _TL("Defaults"), _TL(""));

	//-----------------------------------------------------
	m_Parameters.Add_Node  ("NODE_DEFAULTS", "NODE_NEW_MAPS", _TL("New Maps"), _TL(""));

	m_Parameters.Add_Choice("NODE_NEW_MAPS", "NAMING"       , _TL("Naming"             ), _TL(""), CSG_String::Format("\'%s\'|%s", _TL("Map"), _TL("first added layer name")));
	m_Parameters.Add_Int   ("NAMING"       , "NAMING_LENGTH", _TL("Maximum Characters" ), _TL(""), 256, 5, true);
	m_Parameters.Add_Choice("NODE_NEW_MAPS", "BG_COLOR"     , _TL("Background Color"   ), _TL(""), CSG_String::Format("%s|%s", _TL("system"), _TL("user defined")));
	m_Parameters.Add_Color ("BG_COLOR"     , "BG_USER"      , _TL("User Defined"       ), _TL(""), SG_COLOR_WHITE);
	m_Parameters.Add_Bool  ("NODE_NEW_MAPS", "GOTO_NEWLAYER", _TL("Zoom to added layer"), _TL(""), false);
	m_Parameters.Add_Bool  ("NODE_NEW_MAPS", "CRS_CHECK"    , _TL("CRS Check"          ), _TL("Perform a coordinate system compatibility check before a layer is added."), true);
	m_Parameters.Add_Bool  ("NODE_NEW_MAPS", "SCALE_BAR"    , _TL("Scale Bar"          ), _TL(""), false);
	m_Parameters.Add_Bool  ("NODE_NEW_MAPS", "FRAME_SHOW"   , _TL("Frame"              ), _TL(""), true );
	m_Parameters.Add_Int   ("FRAME_SHOW"   , "FRAME_WIDTH"  , _TL("Width"              ), _TL(""), 17, 10, true);

	//-----------------------------------------------------
	m_Parameters.Add_Node  ("NODE_DEFAULTS", "NODE_3DVIEW", _TL("3D View"), _TL(""));

	m_Parameters.Add_Double("NODE_3DVIEW", "3DVIEW_Z_SCALE", _TL("Exaggeration"), _TL(""), 1.);

	//-----------------------------------------------------
	m_Parameters.Add_Node  ("", "NODE_CLIPBOARD", _TL("Clipboard"), _TL(""));

	m_Parameters.Add_Int   ("NODE_CLIPBOARD" , "CLIP_NX"         , _TL("Width" ), _TL(""), 400, 10, true);
	m_Parameters.Add_Int   ("NODE_CLIPBOARD" , "CLIP_NY"         , _TL("Height"), _TL(""), 400, 10, true);
	m_Parameters.Add_Bool  ("NODE_CLIPBOARD" , "CLIP_FRAME_SHOW" , _TL("Frame" ), _TL(""), true);
	m_Parameters.Add_Int   ("CLIP_FRAME_SHOW", "CLIP_FRAME_WIDTH", _TL("Width" ), _TL(""),  17,  0, true);

	//-----------------------------------------------------
	m_Parameters.Add_Node  ("NODE_CLIPBOARD", "NODE_CLIP_LEGEND", _TL("Legend"), _TL(""));

	m_Parameters.Add_Double("NODE_CLIP_LEGEND", "CLIP_LEGEND_SCALE", _TL("Scale"       ), _TL(""),  2., 1., true);
	m_Parameters.Add_Int   ("NODE_CLIP_LEGEND", "CLIP_LEGEND_FRAME", _TL("Frame Width" ), _TL(""), 10 , 0 , true);
	m_Parameters.Add_Color ("NODE_CLIP_LEGEND", "CLIP_LEGEND_COLOR", _TL("Border Color"), _TL(""), SG_GET_RGB(0, 0, 0));

	//-----------------------------------------------------
	CONFIG_Read("/MAPS", &m_Parameters);

	m_CrossHair	= m_Parameters("CROSSHAIR")->asInt();
}

//---------------------------------------------------------
CWKSP_Map_Manager::~CWKSP_Map_Manager(void)
{
	CONFIG_Write("/MAPS", &m_Parameters);

	g_pMaps = NULL;
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
wxString CWKSP_Map_Manager::Get_Name(void)
{
	return( _TL("Maps") );
}

//---------------------------------------------------------
wxString CWKSP_Map_Manager::Get_Description(void)
{
	wxString s;

	//-----------------------------------------------------
	s += wxString::Format("<h4>%s</h4>", _TL("Maps"));

	s += "<table border=\"0\">";

	DESC_ADD_INT(_TL("Number of Maps"), Get_Count());

	s += "</table>";

	//-----------------------------------------------------
	#define USAGE_START(title) s += wxString::Format("<h6><b>%s</b></h6><table border=\"1\"><tr><td valign=\"top\"><b>%s</b></td><td valign=\"top\"><b>%s</b></td><td valign=\"top\"><b>%s</b></td></tr>", title, _TL("Keyboard"), _TL("Mouse"), _TL("Description"));
	#define USAGE_STOP s += "</table>";
	#define USAGE_ADD(keys, mouse, desc) s += wxString::Format("<tr><td valign=\"top\">%s</td><td valign=\"top\">%s</td><td valign=\"top\">%s</td></tr>", keys, mouse, desc);

	s += wxString::Format("<hr><h5>%s</h5>", _TL("Keyboard and Mouse Usage"));

	USAGE_START(_TL("All Modes"));
	USAGE_ADD(_TL("Left"         ), _TL(""      ), _TL("Move the view left"));
	USAGE_ADD(_TL("Right"        ), _TL(""      ), _TL("Move the view right"));
	USAGE_ADD(_TL("Up"           ), _TL(""      ), _TL("Move the view up"));
	USAGE_ADD(_TL("Down"         ), _TL(""      ), _TL("Move the view down"));
	USAGE_ADD(_TL("Page Up"      ), _TL(""      ), _TL("Zoom in"));
	USAGE_ADD(_TL("Page Down"    ), _TL(""      ), _TL("Zoom out"));
	USAGE_ADD(_TL("Control + C"  ), _TL(""      ), _TL("Copy map to clipboard using map manager's clipboard settings"));
	USAGE_ADD(_TL("Alternate + C"), _TL(""      ), _TL("Copy map to clipboard exactly as displayed in map window"));
	USAGE_ADD(_TL(""             ), _TL("Middle"), _TL("Pan"));
	USAGE_ADD(_TL(""             ), _TL("Wheel" ), _TL("Zoom in/out"));
	USAGE_STOP;

	USAGE_START(_TL("Zoom"));
	USAGE_ADD(_TL(""             ), _TL("Left"  ), _TL("Zoom in"));
	USAGE_ADD(_TL(""             ), _TL("Right" ), _TL("Zoom out"));
	USAGE_ADD(_TL("Control"      ), _TL("Left"  ), _TL("Copy map to clipboard using map manager's clipboard settings"));
	USAGE_ADD(_TL("Alternate"    ), _TL("Left"  ), _TL("Copy map to clipboard exactly as displayed in map window"));
	USAGE_ADD(_TL("Control"      ), _TL("Right" ), _TL("Context menu for map"));
	USAGE_ADD(_TL("Alternate"    ), _TL("Right" ), _TL("Show clicked position's coordinates in a dialog"));
	USAGE_STOP;

	USAGE_START(_TL("Pan"));
	USAGE_ADD(_TL(""             ), _TL("Left"  ), _TL("Pan"));
	USAGE_ADD(_TL(""             ), _TL("Right" ), _TL("Context menu"));
	USAGE_STOP;

	USAGE_START(_TL("Measure"));
	USAGE_ADD(_TL(""             ), _TL("Left"  ), _TL("Add vertex"));
	USAGE_ADD(_TL(""             ), _TL("Right" ), _TL("1.) Stop adding vertices, 2.) Clear and restart adding vertices"));
	USAGE_ADD(_TL("Control"      ), _TL("Right" ), _TL("Context menu"));
	USAGE_STOP;

	USAGE_START(_TL("Interact with Shapes"));
	USAGE_ADD(_TL(""             ), _TL("Left"  ), _TL("Clicked shape as new selection"));
	USAGE_ADD(_TL("Control"      ), _TL("Left"  ), _TL("Add/remove clicked shape to current selection"));
	USAGE_ADD(_TL(""             ), _TL("Right" ), _TL("Context menu"));
	USAGE_ADD(_TL("Return"       ), _TL(""      ), _TL("Enter/leave shape edit mode"));
	USAGE_ADD(_TL("Delete"       ), _TL(""      ), _TL("Delete selection"));
	USAGE_STOP;

	USAGE_START(_TL("Interact with Grid"));
	USAGE_ADD(_TL(""             ), _TL("Left"  ), _TL("Select cells"));
	USAGE_ADD(_TL(""             ), _TL("Right" ), _TL("Context menu"));
	USAGE_ADD(_TL("Delete"       ), _TL(""      ), _TL("Set selected cells to no-data"));
	USAGE_STOP;

	s += wxString::Format("<p><b>%s:</b>%s</p>", _TL("Remark"), _TL("Pure keyboard events work only for the map window that has the focus."));

	//-----------------------------------------------------
	return( s );
}

//---------------------------------------------------------
wxMenu * CWKSP_Map_Manager::Get_Menu(void)
{
	if( Get_Count() < 1 )
	{
		return( NULL );
	}

	wxMenu *pMenu = new wxMenu(Get_Name());

	CMD_Menu_Add_Item(pMenu, false, ID_CMD_WKSP_ITEM_CLOSE);
	pMenu->AppendSeparator();
	CMD_Menu_Add_Item(pMenu, false, ID_CMD_WKSP_ITEM_SEARCH);

	return( pMenu );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Map_Manager::On_Command(int Cmd_ID)
{
	switch( Cmd_ID )
	{
	default:
		return( CWKSP_Base_Manager::On_Command(Cmd_ID) );

	case ID_CMD_WKSP_ITEM_RETURN:
		break;
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Map_Manager::Parameters_Changed(void)
{
	if( g_pMap_Buttons ) { g_pMap_Buttons->Update_Buttons(); }

	CWKSP_Base_Manager::Parameters_Changed();

	if( m_CrossHair != m_Parameters("CROSSHAIR")->asInt() )
	{
		m_CrossHair	= m_Parameters("CROSSHAIR")->asInt();

		if( !m_CrossHair )
		{
			for(int i=0; i<Get_Count(); i++)
			{
				Get_Map(i)->Set_CrossHair_Off();
			}
		}
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CWKSP_Map_Manager::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter, int Flags)
{
	//-----------------------------------------------------
	if( Flags & PARAMETER_CHECK_VALUES )
	{
		if( g_pSAGA_Frame && g_pData )
		{
			if( pParameter->Cmp_Identifier("THUMBNAILS") )
			{
				if( pParameter->asBool() != m_Parameters["THUMBNAILS"].asBool() && DLG_Message_Confirm(_TL("Close now ?"), _TL("Restart SAGA to apply the changes")) )
				{
					m_Parameters.Assign_Values(pParameters);

					if( g_pData->Close(true) )
					{
						g_pSAGA_Frame->Close();
					}
				}
			}
		}
	}

	//-----------------------------------------------------
	if( Flags & PARAMETER_CHECK_ENABLE )
	{
		if(	pParameter->Cmp_Identifier("THUMBNAILS") )
		{
			pParameter->Set_Children_Enabled(pParameter->asBool());
		}

		if(	pParameter->Cmp_Identifier("PREVIEW") )
		{
			pParameter->Set_Children_Enabled(pParameter->asBool());
		}
	}

	//-----------------------------------------------------
	return( CWKSP_Base_Manager::On_Parameter_Changed(pParameters, pParameter, Flags) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Map_Manager::Exists(CWKSP_Map *pMap)
{
	if( pMap )
	{
		for(int i=0; i<Get_Count(); i++)
		{
			if( pMap == Get_Map(i) )
			{
				return( true );
			}
		}
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Close(bool bSilent)
{
	return( g_pMap_Ctrl->Close(bSilent) );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Add(CWKSP_Map *pMap)
{
	if( pMap )
	{
		if( !Exists(pMap) )
		{
			Add_Item(pMap);
		}

		return( true );
	}

	return( false );
}

bool CWKSP_Map_Manager::Add(CWKSP_Layer *pLayer)
{
	int	iMap = DLG_Maps_Add();

	if( iMap >= 0 && Add(pLayer, Get_Map(iMap)) )
	{
		Get_Map(iMap)->View_Show(true);

		return( true );
	}

	return( false );
}

bool CWKSP_Map_Manager::Add(CWKSP_Layer *pLayer, CWKSP_Map *pMap)
{
	if( pLayer )
	{
		if( pMap == NULL )
		{
			Add_Item(pMap = new CWKSP_Map);
		}
		else if( !Exists(pMap) )
		{
			Add_Item(pMap);
		}

		return( pMap->Add_Layer(pLayer) != NULL );
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Del(CWKSP_Layer *pLayer)
{
	int n = 0;

	for(int i=Get_Count()-1; i>=0; i--)
	{
		if( g_pMap_Ctrl->Del_Item(Get_Map(i), pLayer) )
		{
			n++;
		}
	}

	return( n > 0 );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Update(CWKSP_Layer *pLayer, bool bMapsOnly)
{
	int n = 0;

	for(int i=0; i<Get_Count(); i++)
	{
		if( Get_Map(i)->Update(pLayer, bMapsOnly) )
		{
			n++;
		}
	}

	return( n > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Map_Manager::Reset_Numbering(void)
{
	if( Get_Count() == 0 )
	{
		m_Numbering = 0;

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
wxString CWKSP_Map_Manager::Get_Numbering(void)
{
	m_Numbering++; int Numbering = g_pMaps->Get_Parameter("NUMBERING")->asInt();

	if( Numbering  >  1 ) { return( wxString::Format( "%*d. ",  Numbering, m_Numbering) ); }
	if( Numbering  < -1 ) { return( wxString::Format("%0*d. ", -Numbering, m_Numbering) ); }
	if( Numbering !=  0 ) { return( wxString::Format(  "%d. "            , m_Numbering) ); }

	return( "" );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Map_Manager::Set_Extent(const TSG_Rect &Extent, int Maps)
{
	CWKSP_Map *pMap = NULL;

	switch( Maps )
	{
	case SG_UI_MAP_ACTIVE: pMap = (CWKSP_Map *)g_pSAGA_Frame->Get_Active_Child(ID_VIEW_MAP); break;
	case SG_UI_MAP_LAST  : pMap = Get_Map(Get_Count() - 1); break;
	case SG_UI_MAP_ALL   : return( Set_Extents(Extent, CSG_Projection()) );
	}

	return( pMap ? pMap->Set_Extent(Extent) : false );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Set_Extents(const TSG_Rect &Extent, const CSG_Projection &Projection)
{
	for(int i=0; i<Get_Count(); i++)
	{
		Get_Map(i)->Lock_Synchronising(true);
		Get_Map(i)->Set_Extent(Extent, Projection);
		Get_Map(i)->Lock_Synchronising(false);
	}

	return( true );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Set_Mouse_Position(const TSG_Point &Point, const CSG_Projection &Projection)
{
	if( m_CrossHair > 0 )
	{
		m_CrossHair *= -1;

		CSG_Projection Invalid;

		for(int i=0; i<Get_Count(); i++)
		{
			Get_Map(i)->Set_CrossHair(Point, m_CrossHair == -2 ? Projection : Invalid);
		}

		m_CrossHair *= -1;

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Map_Manager::Toggle_CrossHair(void)
{
	m_Parameters("CROSSHAIR")->Set_Value(m_CrossHair = m_CrossHair != 0 ? 0 : 2);

	if( !m_CrossHair )
	{
		for(int i=0; i<Get_Count(); i++)
		{
			Get_Map(i)->Set_CrossHair_Off();
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
