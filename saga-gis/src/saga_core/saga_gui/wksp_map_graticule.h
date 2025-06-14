
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
//                 wksp_map_graticule.h                  //
//                                                       //
//          Copyright (C) 2014 by Olaf Conrad            //
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
//                University of Hamburg                  //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef _HEADER_INCLUDED__SAGA_GUI__wksp_map_graticule_H
#define _HEADER_INCLUDED__SAGA_GUI__wksp_map_graticule_H


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "wksp_base_item.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CWKSP_Map_Graticule : public CWKSP_Base_Item
{
public:
	CWKSP_Map_Graticule(CSG_MetaData *pEntry = NULL);
	virtual ~CWKSP_Map_Graticule(void);

	virtual TWKSP_Item			Get_Type				(void)	{	return( WKSP_ITEM_Map_Graticule );	}

	virtual wxString			Get_Name				(void);
	virtual wxString			Get_Description			(void);

	virtual wxMenu *			Get_Menu				(void);

	virtual bool				On_Command				(int Cmd_ID);
	virtual bool				On_Command_UI			(wxUpdateUIEvent &event);

	virtual int					On_Parameter_Changed	(CSG_Parameters *pParameters, CSG_Parameter *pParameter, int Flags);
	virtual void				Parameters_Changed		(void);

	bool						do_Show					(void)	{	return( m_bShow );	}

	class CWKSP_Map *			Get_Map					(void)	{	return( (class CWKSP_Map *)Get_Manager() );	}

	bool						Draw					(class CSG_Map_DC &dc);

	bool						Load					(CSG_MetaData &Entry);
	bool						Save					(CSG_MetaData &Entry);


private:

	bool						m_bShow;

	CSG_Shapes					m_Graticule, m_Coordinates;


	bool						Get_Graticule			(const CSG_Rect &Extent);

	CSG_String					Get_Unit				(CSG_Shape *pPoint, int Units, int Decimals, bool bLatitude);

};


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef _HEADER_INCLUDED__SAGA_GUI__wksp_map_graticule_H
