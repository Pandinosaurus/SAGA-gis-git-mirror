
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
//                    RES_Commands.h                     //
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
#ifndef _HEADER_INCLUDED__SAGA_GUI__RES_Commands_H
#define _HEADER_INCLUDED__SAGA_GUI__RES_Commands_H


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <wx/event.h>

//---------------------------------------------------------
#include <saga_api/saga_api.h>


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define MAX_COUNT_RECENT_PROJECTS 16
#define MAX_COUNT_RECENT_FILES    32
#define MAX_COUNT_RECENT_TOOLS    16
#define MAX_COUNT_TOOLS           2048

//---------------------------------------------------------
enum ID_COMMANDS
{
	ID_CMD_FIRST = wxID_HIGHEST + 1024,

	ID_CMD_FRAME_CASCADE,
	ID_CMD_FRAME_TILE_HORZ,
	ID_CMD_FRAME_TILE_VERT,
	ID_CMD_FRAME_ARRANGEICONS,
	ID_CMD_FRAME_UNSPLIT,
	ID_CMD_FRAME_SPLIT_LEFT,
	ID_CMD_FRAME_SPLIT_RIGHT,
	ID_CMD_FRAME_SPLIT_TOP,
	ID_CMD_FRAME_SPLIT_BOTTOM,
	ID_CMD_FRAME_SPLIT_ALL_HORZ,
	ID_CMD_FRAME_SPLIT_ALL_VERT,
	ID_CMD_FRAME_NEXT,
	ID_CMD_FRAME_PREVIOUS,
	ID_CMD_FRAME_FIND,
	ID_CMD_FRAME_CLOSE,
	ID_CMD_FRAME_CLOSE_ALL,
	ID_CMD_FRAME_CLOSE_ALL_OTHERS,

	ID_CMD_FRAME_QUIT,
	ID_CMD_FRAME_HELP,
	ID_CMD_FRAME_ABOUT,
	ID_CMD_FRAME_TIPS,

	ID_CMD_FRAME_INFO_SHOW,
	ID_CMD_FRAME_DATA_SOURCE_SHOW,
	ID_CMD_FRAME_ACTIVE_SHOW,
	ID_CMD_FRAME_WKSP_SHOW,

	//-----------------------------------------------------
	ID_CMD_WKSP_FIRST, // begin workspace commands
	ID_CMD_WKSP_ITEM_RETURN,
	ID_CMD_WKSP_ITEM_CLOSE,
	ID_CMD_WKSP_ITEM_SEARCH,
	ID_CMD_WKSP_ITEM_SHOW,

	ID_CMD_INFO_CLEAR,
	ID_CMD_INFO_SELECTALL,
	ID_CMD_INFO_COPY,

	ID_CMD_DATASOURCE_REFRESH,
	ID_CMD_DATASOURCE_OPEN,

	ID_CMD_DB_REFRESH,
	ID_CMD_DB_SOURCE_CREATE,
	ID_CMD_DB_SOURCE_DROP,
	ID_CMD_DB_SOURCE_OPEN,
	ID_CMD_DB_SOURCE_CLOSE,
	ID_CMD_DB_SOURCE_CLOSE_ALL,
	ID_CMD_DB_SOURCE_DELETE,
	ID_CMD_DB_SOURCE_SQL,
	ID_CMD_DB_TABLE_OPEN,
	ID_CMD_DB_TABLE_DELETE,
	ID_CMD_DB_TABLE_RENAME,
	ID_CMD_DB_TABLE_INFO,
	ID_CMD_DB_TABLE_FROM_QUERY,

	//-----------------------------------------------------
	ID_CMD_DATA_FIRST, // begin data commands
	ID_CMD_DATA_PROJECT,
	ID_CMD_DATA_PROJECT_RECENT_FIRST,
	ID_CMD_DATA_PROJECT_RECENT_LAST = ID_CMD_DATA_PROJECT_RECENT_FIRST + MAX_COUNT_RECENT_PROJECTS,
	ID_CMD_DATA_PROJECT_OPEN,
	ID_CMD_DATA_PROJECT_OPEN_ADD,
	ID_CMD_DATA_PROJECT_BROWSE,
	ID_CMD_DATA_PROJECT_NEW,
	ID_CMD_DATA_PROJECT_SAVE,
	ID_CMD_DATA_PROJECT_SAVE_AS,
	ID_CMD_DATA_PROJECT_COPY,
	ID_CMD_DATA_PROJECT_COPY_DB,

	ID_CMD_DATA_MANAGER_LIST,

	ID_CMD_DATA_LEGEND_COPY,
	ID_CMD_DATA_LEGEND_SIZE_INC,
	ID_CMD_DATA_LEGEND_SIZE_DEC,
	ID_CMD_DATA_LEGEND_BACKGROUND_INVERT,

	ID_CMD_DATA_HISTORY_CLEAR,
	ID_CMD_DATA_HISTORY_TO_MODEL,
	ID_CMD_DATA_HISTORY_OPTIONS_COLLAPSE,
	ID_CMD_DATA_HISTORY_OPTIONS_EXPAND,

	ID_CMD_DATA_OPEN,
	ID_CMD_DATA_SAVE,
	ID_CMD_DATA_SAVEAS,
	ID_CMD_DATA_SAVEAS_IMAGE,
	ID_CMD_DATA_SAVETODB,
	ID_CMD_DATA_RELOAD,
	ID_CMD_DATA_DEL_FILES,
	ID_CMD_DATA_CLIPBOARD_COPY,
	ID_CMD_DATA_CLIPBOARD_PASTE_TABLE,
	ID_CMD_DATA_CLIPBOARD_PASTE_IMAGE,
	ID_CMD_DATA_PROJECTION,
	ID_CMD_DATA_METADATA,
	ID_CMD_DATA_FORCE_UPDATE,
	ID_CMD_DATA_SETTINGS_LOAD,
	ID_CMD_DATA_SETTINGS_COPY,
	ID_CMD_DATA_RANGE_MINMAX,
	ID_CMD_DATA_RANGE_STDDEV,
	ID_CMD_DATA_RANGE_STDDEV150,
	ID_CMD_DATA_RANGE_STDDEV200,
	ID_CMD_DATA_RANGE_PERCENTILE,
	ID_CMD_DATA_CLASSIFY,
	ID_CMD_DATA_CLASSIFY_IMPORT,
	ID_CMD_DATA_SELECTION_CLEAR,
	ID_CMD_DATA_SELECTION_INVERT,
	ID_CMD_DATA_SELECTION_DELETE,
	ID_CMD_DATA_SHOW_MAP,
	ID_CMD_DATA_HISTOGRAM,
	ID_CMD_DATA_SCATTERPLOT,
	ID_CMD_DATA_SCATTERPLOT_3D,
	ID_CMD_DATA_DIAGRAM,
	ID_CMD_DATA_LAST,

	ID_CMD_DATA_FOLDER_RECENT,
	ID_CMD_DATA_FOLDER_RECENT_FIRST,
	ID_CMD_DATA_FOLDER_RECENT_LAST = ID_CMD_DATA_FOLDER_RECENT_FIRST + MAX_COUNT_RECENT_FILES,

	ID_CMD_DATA_FILE_RECENT,
	ID_CMD_DATA_FILE_RECENT_FIRST,
	ID_CMD_DATA_FILE_RECENT_LAST = ID_CMD_DATA_FILE_RECENT_FIRST + MAX_COUNT_RECENT_FILES,

	ID_CMD_GRID_FIRST,
	ID_CMD_GRID_OPEN,
	ID_CMD_GRID_LAST,

	ID_CMD_GRIDS_FIRST,
	ID_CMD_GRIDS_OPEN,
	ID_CMD_GRIDS_3DVIEW,
	ID_CMD_GRIDS_LAST,

	ID_CMD_TABLE_FIRST,
	ID_CMD_TABLE_OPEN,
	ID_CMD_TABLE_SHOW,
	ID_CMD_TABLE_JOIN_DATA,
	ID_CMD_TABLE_FIELD_ADD,
	ID_CMD_TABLE_FIELD_MOVE,
	ID_CMD_TABLE_FIELD_DEL,
	ID_CMD_TABLE_FIELD_HIDE,
	ID_CMD_TABLE_FIELD_SORT,
	ID_CMD_TABLE_FIELD_RENAME,
	ID_CMD_TABLE_FIELD_TYPE,
	ID_CMD_TABLE_FIELD_CALC,
	ID_CMD_TABLE_FIELD_OPEN_APP,
	ID_CMD_TABLE_FIELD_OPEN_DATA,
	ID_CMD_TABLE_RECORD_ADD,
	ID_CMD_TABLE_RECORD_INS,
	ID_CMD_TABLE_RECORD_DEL,
	ID_CMD_TABLE_RECORD_DEL_ALL,
	ID_CMD_TABLE_SELECTION_SHOW,
	ID_CMD_TABLE_SELECT_NUMERIC,
	ID_CMD_TABLE_SELECT_STRING,
	ID_CMD_TABLE_AUTOSIZE_COLS,
	ID_CMD_TABLE_AUTOSIZE_ROWS,
	ID_CMD_TABLE_LAST,

	ID_CMD_SHAPES_FIRST,
	ID_CMD_SHAPES_OPEN,
	ID_CMD_SHAPES_SAVE_ATTRIBUTES,
	ID_CMD_SHAPES_EDIT_SHAPE,
    ID_CMD_SHAPES_EDIT_SEL_COPY,
	ID_CMD_SHAPES_EDIT_ADD_SHAPE,
	ID_CMD_SHAPES_EDIT_ADD_PART,
	ID_CMD_SHAPES_EDIT_DEL_SHAPE,
	ID_CMD_SHAPES_EDIT_DEL_PART,
	ID_CMD_SHAPES_EDIT_DEL_POINT,
	ID_CMD_SHAPES_EDIT_SPLIT,
	ID_CMD_SHAPES_EDIT_MERGE,
	ID_CMD_SHAPES_EDIT_MOVE,
	ID_CMD_SHAPES_LAST,

	ID_CMD_POINTCLOUD_FIRST,
	ID_CMD_POINTCLOUD_OPEN,
	ID_CMD_POINTCLOUD_3DVIEW,
	ID_CMD_POINTCLOUD_LAST,

	ID_CMD_TIN_FIRST,
	ID_CMD_TIN_OPEN,
	ID_CMD_TIN_3DVIEW,
	ID_CMD_TIN_LAST,

	ID_CMD_MAPS_FIRST,
	ID_CMD_MAPS_SHOW,
	ID_CMD_MAPS_3D_SHOW,
	ID_CMD_MAPS_LAYOUT_SHOW,
	ID_CMD_MAPS_SAVE_IMAGE,
	ID_CMD_MAPS_SAVE_IMAGE_ON_CHANGE,
	ID_CMD_MAPS_SAVE_IMAGE_TO_KMZ,
	ID_CMD_MAPS_SAVE_IMAGE_TO_MEMORY,
	ID_CMD_MAPS_SAVE_TO_CLIPBOARD,
	ID_CMD_MAPS_SAVE_TO_CLIPBOARD_LEGEND,
	ID_CMD_MAPS_SYNCHRONIZE,
	ID_CMD_MAPS_SCALEBAR,
	ID_CMD_MAPS_PROJECTION,
	ID_CMD_MAPS_GRATICULE_ADD,
	ID_CMD_MAPS_BASEMAP_ADD,
	ID_CMD_MAPS_LAYER_SHOW,
	ID_CMD_MAPS_MOVE_TOP,
	ID_CMD_MAPS_MOVE_BOTTOM,
	ID_CMD_MAPS_MOVE_UP,
	ID_CMD_MAPS_MOVE_DOWN,
	ID_CMD_MAPS_GRID_FITCOLORS,
	ID_CMD_MAPS_PROJECT,
	ID_CMD_MAPS_PROJECT_AREA,
	ID_CMD_MAPS_LAST,

	ID_CMD_WKSP_LAST, // end workspace commands

	//-----------------------------------------------------
	ID_CMD_ACTIVE_FIRST, // begin active object's properties commands
	ID_CMD_ACTIVE_CLIPBOARD_COPY,
	ID_CMD_ACTIVE_LAST,  // end active object's properties commands

	//-----------------------------------------------------
	ID_CMD_CHILD_FIRST,	// begin view commands (mdi child windows)

	ID_CMD_VIEW_CLIPBOARD_COPY,
	ID_CMD_VIEW_CLIPBOARD_PASTE,
	ID_CMD_VIEW_SAVE,

	ID_CMD_MAP_FIRST,
	ID_CMD_MAP_CLOSE,
	ID_CMD_MAP_TOOLBAR,
	ID_CMD_MAP_3D_SHOW,
	ID_CMD_MAP_LAYOUT_SHOW,
	ID_CMD_MAP_SCALEBAR,
	ID_CMD_MAP_GRATICULE_ADD,
	ID_CMD_MAP_BASEMAP_ADD,
	ID_CMD_MAP_SAVE_IMAGE,
	ID_CMD_MAP_SAVE_IMAGE_ON_CHANGE,
	ID_CMD_MAP_SYNCHRONIZE,
	ID_CMD_MAP_NORTH_ARROW,
	ID_CMD_MAP_LEGEND_VERTICAL,
	ID_CMD_MAP_LEGEND_HORIZONTAL,
	ID_CMD_MAP_MODE_ZOOM,
	ID_CMD_MAP_MODE_PAN,
	ID_CMD_MAP_MODE_SELECT,
	ID_CMD_MAP_MODE_DISTANCE,
	ID_CMD_MAP_ZOOM_FULL,
	ID_CMD_MAP_ZOOM_BACK,
	ID_CMD_MAP_ZOOM_FORWARD,
	ID_CMD_MAP_ZOOM_EXTENT,
	ID_CMD_MAP_ZOOM_ACTIVE,
	ID_CMD_MAP_ZOOM_SELECTION,
	ID_CMD_MAP_PAN_ACTIVE,
	ID_CMD_MAP_PAN_SELECTION,
	ID_CMD_MAP_ZOOM_LAYER_ACTIVE,
	ID_CMD_MAP_ZOOM_LAYER_SELECTION,
	ID_CMD_MAP_PAN_LAYER_ACTIVE,
	ID_CMD_MAP_PAN_LAYER_SELECTION,
	ID_CMD_MAP_CROSSHAIR,
	ID_CMD_MAP_LAST,

	ID_CMD_MAP3D_FIRST,
	ID_CMD_MAP3D_PARAMETERS,
	ID_CMD_MAP3D_USAGE,
	ID_CMD_MAP3D_ROTATE_X_LESS,
	ID_CMD_MAP3D_ROTATE_X_MORE,
	ID_CMD_MAP3D_ROTATE_Y_LESS,
	ID_CMD_MAP3D_ROTATE_Y_MORE,
	ID_CMD_MAP3D_ROTATE_Z_LESS,
	ID_CMD_MAP3D_ROTATE_Z_MORE,
	ID_CMD_MAP3D_SHIFT_X_LESS,
	ID_CMD_MAP3D_SHIFT_X_MORE,
	ID_CMD_MAP3D_SHIFT_Y_LESS,
	ID_CMD_MAP3D_SHIFT_Y_MORE,
	ID_CMD_MAP3D_SHIFT_Z_LESS,
	ID_CMD_MAP3D_SHIFT_Z_MORE,
	ID_CMD_MAP3D_EXAGGERATE_LESS,
	ID_CMD_MAP3D_EXAGGERATE_MORE,
	ID_CMD_MAP3D_CENTRAL,
	ID_CMD_MAP3D_CENTRAL_LESS,
	ID_CMD_MAP3D_CENTRAL_MORE,
	ID_CMD_MAP3D_ANAGLYPH,
	ID_CMD_MAP3D_STEREO_VIEW,
	ID_CMD_MAP3D_STEREO_LESS,
	ID_CMD_MAP3D_STEREO_MORE,
	ID_CMD_MAP3D_INTERPOLATED,
	ID_CMD_MAP3D_SRC_RES_LESS,
	ID_CMD_MAP3D_SRC_RES_MORE,
	ID_CMD_MAP3D_SAVE,
	ID_CMD_MAP3D_SEQ_POS_ADD,
	ID_CMD_MAP3D_SEQ_POS_DEL,
	ID_CMD_MAP3D_SEQ_POS_DEL_ALL,
	ID_CMD_MAP3D_SEQ_POS_EDIT,
	ID_CMD_MAP3D_SEQ_PLAY,
	ID_CMD_MAP3D_SEQ_PLAY_LOOP,
	ID_CMD_MAP3D_SEQ_SAVE,
	ID_CMD_MAP3D_LAST,

	ID_CMD_DIAGRAM_FIRST,
	ID_CMD_DIAGRAM_PARAMETERS,
	ID_CMD_DIAGRAM_SIZE_FIT,
	ID_CMD_DIAGRAM_SIZE_INC,
	ID_CMD_DIAGRAM_SIZE_DEC,
	ID_CMD_DIAGRAM_LEGEND,
	ID_CMD_DIAGRAM_LAST,

	ID_CMD_SCATTERPLOT_FIRST,
	ID_CMD_SCATTERPLOT_PARAMETERS,
	ID_CMD_SCATTERPLOT_UPDATE,
	ID_CMD_SCATTERPLOT_AS_TABLE,
	ID_CMD_SCATTERPLOT_LAST,

	ID_CMD_HISTOGRAM_FIRST,
	ID_CMD_HISTOGRAM_PARAMETERS,
	ID_CMD_HISTOGRAM_TOOLBAR,
	ID_CMD_HISTOGRAM_CUMULATIVE,
	ID_CMD_HISTOGRAM_GAUSSIAN,
	ID_CMD_HISTOGRAM_SET_MINMAX,
	ID_CMD_HISTOGRAM_AS_TABLE,
	ID_CMD_HISTOGRAM_LAST,

	ID_CMD_LAYOUT_FIRST,
	ID_CMD_LAYOUT_TOOLBAR,
	ID_CMD_LAYOUT_LOAD,
	ID_CMD_LAYOUT_SAVE,
	ID_CMD_LAYOUT_PAGE_SETUP,
	ID_CMD_LAYOUT_PRINT_SETUP,
	ID_CMD_LAYOUT_PRINT_PREVIEW,
	ID_CMD_LAYOUT_PRINT,
	ID_CMD_LAYOUT_EXPORT,
	ID_CMD_LAYOUT_PROPERTIES,
	ID_CMD_LAYOUT_ZOOM_IN,
	ID_CMD_LAYOUT_ZOOM_OUT,
	ID_CMD_LAYOUT_ZOOM_FULL,
	ID_CMD_LAYOUT_ZOOM_ORIGINAL,
	ID_CMD_LAYOUT_ITEM_MAP,
	ID_CMD_LAYOUT_ITEM_LEGEND,
	ID_CMD_LAYOUT_ITEM_SCALEBAR,
	ID_CMD_LAYOUT_ITEM_SCALE,
	ID_CMD_LAYOUT_ITEM_LABEL,
	ID_CMD_LAYOUT_ITEM_TEXT,
	ID_CMD_LAYOUT_ITEM_IMAGE,
	ID_CMD_LAYOUT_ITEM_PROPERTIES,
	ID_CMD_LAYOUT_ITEM_HIDE,
	ID_CMD_LAYOUT_ITEM_DELETE,
	ID_CMD_LAYOUT_ITEM_MOVE_TOP,
	ID_CMD_LAYOUT_ITEM_MOVE_BOTTOM,
	ID_CMD_LAYOUT_ITEM_MOVE_UP,
	ID_CMD_LAYOUT_ITEM_MOVE_DOWN,
	ID_CMD_LAYOUT_IMAGE_SAVE,
	ID_CMD_LAYOUT_IMAGE_RESTORE,
	ID_CMD_LAYOUT_LAST,

	ID_CMD_CHILD_LAST, // end view commands (mdi child windows)

	//-----------------------------------------------------
	ID_CMD_TOOL_FIRST,  // begin tool commands
	ID_CMD_TOOL_OPEN,
	ID_CMD_TOOL_CLOSE_ALL,
	ID_CMD_TOOL_RELOAD,
	ID_CMD_TOOL_SEARCH,
	ID_CMD_TOOL_SAVE_SCRIPT,
	ID_CMD_TOOL_SAVE_DOCS,
	ID_CMD_TOOL_SAVE_TO_CLIPBOARD,
	ID_CMD_TOOL_CHAIN_RELOAD,
	ID_CMD_TOOL_CHAIN_EDIT,
	ID_CMD_TOOL_MANAGER_MENU,
	ID_CMD_TOOL_RECENT_MENU,
	ID_CMD_TOOL_RECENT_FIRST,
	ID_CMD_TOOL_RECENT_LAST = ID_CMD_TOOL_RECENT_FIRST + MAX_COUNT_RECENT_TOOLS,
	ID_CMD_TOOL_MENU_FIRST,
	ID_CMD_TOOL_MENU_LAST = ID_CMD_TOOL_MENU_FIRST + MAX_COUNT_TOOLS,
	ID_CMD_TOOL_LAST // end tool commands
};


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
wxString				CMD_Get_Name				(int Cmd_ID);
wxString				CMD_Get_Help				(int Cmd_ID);
int						CMD_Get_ImageID				(int Cmd_ID);

void					CMD_Menu_Add_Item			(class wxMenu *pMenu, bool bCheck, int Cmd_ID);
void					CMD_Menu_Ins_Item			(class wxMenu *pMenu, bool bCheck, int Cmd_ID, int Position);

class wxToolBarBase *	CMD_ToolBar_Create			(int ID);
void					CMD_ToolBar_Add				(class wxToolBarBase *pToolBar, const wxString &Name);
void					CMD_ToolBar_Add_Item		(class wxToolBarBase *pToolBar, bool bCheck, int Cmd_ID);
void					CMD_ToolBar_Add_Separator	(class wxToolBarBase *pToolBar);


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef _HEADER_INCLUDED__SAGA_GUI__RES_Commands_H
