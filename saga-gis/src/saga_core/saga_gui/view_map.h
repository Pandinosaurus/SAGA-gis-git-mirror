
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
//                      VIEW_Map.h                       //
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
#ifndef _HEADER_INCLUDED__SAGA_GUI__VIEW_Map_H
#define _HEADER_INCLUDED__SAGA_GUI__VIEW_Map_H


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "view_base.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CVIEW_Map : public CVIEW_Base
{
public:
	CVIEW_Map(class CWKSP_Map *pMap, int Frame_Width);

	static class wxToolBarBase *	_Create_ToolBar				(void);
	static class wxMenu *			_Create_Menu				(void);

	virtual void					Do_Update					(void);

	class CWKSP_Map *				Get_Map						(void)	{	return(	m_pMap     );	}
	class CVIEW_Map_Control *		Get_Map_Control				(void)	{	return( m_pControl );	}

	void							Ruler_Set_Width				(int Width);
	void							Ruler_Set_Position			(int x, int y);
	void							Ruler_Refresh				(void);


private:

	int								m_Ruler_Size;

	class CWKSP_Map					*m_pMap;

	class CVIEW_Map_Control			*m_pControl;

	class CVIEW_Ruler				*m_pRuler_X1, *m_pRuler_X2, *m_pRuler_Y1, *m_pRuler_Y2;


	void							_Set_Positions				(void);

	void							On_SysColourChanged			(wxSysColourChangedEvent &event);
	void							On_Paint					(wxPaintEvent            &event);
	void							On_Size						(wxSizeEvent             &event);
	void							On_Key_Down					(wxKeyEvent              &event);

	virtual void					On_Command					(wxCommandEvent          &event);
	virtual void					On_Command_UI				(wxUpdateUIEvent         &event);


	//-----------------------------------------------------
	DECLARE_CLASS(CVIEW_Map)
	DECLARE_EVENT_TABLE()
};


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef _HEADER_INCLUDED__SAGA_GUI__VIEW_Map_H
