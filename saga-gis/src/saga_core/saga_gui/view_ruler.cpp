
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
//                    VIEW_Ruler.cpp                     //
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
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <wx/wx.h>

#include <saga_gdi/sgdi_helper.h>

#include "helper.h"

#include "view_layout.h"
#include "view_ruler.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
BEGIN_EVENT_TABLE(CVIEW_Ruler, wxPanel)
	EVT_SYS_COLOUR_CHANGED(CVIEW_Ruler::On_SysColourChanged)
	EVT_PAINT             (CVIEW_Ruler::On_Paint           )
END_EVENT_TABLE()


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CVIEW_Ruler::CVIEW_Ruler(wxWindow *pParent, int Style)
	: wxPanel(pParent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	SetBackgroundColour(SYS_Get_Color(wxSYS_COLOUR_WINDOW));

	m_bHorizontal = (Style & RULER_VERTICAL    ) == false;
	m_bAscendent  = (Style & RULER_DESCENDING  ) == false;
	m_bTickAtTop  = (Style & RULER_TICKATBOTTOM) == false;
	m_Mode        = (Style & RULER_MODE_SCALE  ) ? 1 : ((Style & RULER_MODE_CORNERS) ? 2 : 0);
	m_Edge        = (Style & RULER_EDGE_BLACK  ) ? 1 : ((Style & RULER_EDGE_SUNKEN ) ? 2 : 0);

	m_Position[0] = m_Position[1] = -1;
	m_Min         =  0;
	m_Max         =  1;
	m_Min_Core    =  1;
	m_Max_Core    =  0;
}

//---------------------------------------------------------
CVIEW_Ruler::~CVIEW_Ruler(void)
{}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Ruler::On_SysColourChanged(wxSysColourChangedEvent &event)
{
	SetBackgroundColour(SYS_Get_Color(wxSYS_COLOUR_WINDOW));
}

//---------------------------------------------------------
void CVIEW_Ruler::On_Paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	_Draw(dc, wxRect(GetClientAreaOrigin(), GetClientSize()));
}

//---------------------------------------------------------
void CVIEW_Ruler::_Draw(wxDC &dc, const wxRect &r)
{
	int Width  = m_bHorizontal ? r.GetWidth () : r.GetHeight();
	int Height = m_bHorizontal ? r.GetHeight() : r.GetWidth ();

	if( m_Position[0] != m_Position[1] )
	{
		dc.SetPen(wxPen(SYS_Get_Color(wxSYS_COLOUR_WINDOW)));

		_Draw_Position(dc, Width, Height, m_Position[0]); m_Position[0] = m_Position[1];
	}

	dc.SetTextForeground(SYS_Get_Color(wxSYS_COLOUR_WINDOWTEXT) );
	dc.SetPen(wxPen     (SYS_Get_Color(wxSYS_COLOUR_WINDOWTEXT)));

	_Draw_Core(dc, Width, Height);

	//-----------------------------------------------------
	switch( m_Mode )
	{
	default: Draw_Scale(dc, r, m_Min, m_Max        , m_bHorizontal, m_bAscendent, m_bTickAtTop); break;
	case  1: Draw_Scale(dc, r,    0., m_Max - m_Min, m_bHorizontal, m_bAscendent, m_bTickAtTop); break;
	case  2: _Draw_Corners(dc, Width, Height); break;
	}

	//-----------------------------------------------------
	switch( m_Edge )
	{
	default: break;
	case  1: Draw_Edge(dc, EDGE_STYLE_SIMPLE, 0, 0, GetClientSize().x - 1, GetClientSize().y - 1); break;
	case  2: Draw_Edge(dc, EDGE_STYLE_SUNKEN, 0, 0, GetClientSize().x - 1, GetClientSize().y - 1); break;
	}

	//-----------------------------------------------------
	_Draw_Position(dc, Width, Height, m_Position[0]);
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Ruler::_Draw_Corners(wxDC &dc, int Width, int Height)
{
	#define TEXTSPACE 4

	wxFont Font((int)(0.65 * (double)Height), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

	dc.SetFont(Font);

	if( m_bHorizontal )
	{
		Draw_Text(dc, TEXTALIGN_BOTTOMLEFT ,         TEXTSPACE, Height, wxString::Format("%f", m_Min));
		Draw_Text(dc, TEXTALIGN_BOTTOMRIGHT, Width - TEXTSPACE, Height, wxString::Format("%f", m_Max));
	}
	else
	{
		Draw_Text(dc, TEXTALIGN_TOPLEFT , 0, Width - TEXTSPACE, 90., wxString::Format("%f", m_Min));
		Draw_Text(dc, TEXTALIGN_TOPRIGHT, 0,         TEXTSPACE, 90., wxString::Format("%f", m_Max));
	}
}

//---------------------------------------------------------
void CVIEW_Ruler::_Draw_Core(wxDC &dc, int Width, int Height)
{
	if( m_Min < m_Max && m_Min_Core < m_Max_Core )
	{
		if( m_Min_Core > m_Min )
		{
			double x = (m_Min_Core - m_Min) * (double)Width / (m_Max - m_Min);

			if( m_bHorizontal )
			{
				Draw_FillRect(dc, SYS_Get_Color(wxSYS_COLOUR_3DFACE), 0, 0, (int)x, Height);
			}
			else
			{
				Draw_FillRect(dc, SYS_Get_Color(wxSYS_COLOUR_3DFACE), 0, 0, Height, (int)x);
			}
		}

		if( m_Max_Core < m_Max )
		{
			double x = (m_Max_Core - m_Min) * (double)Width / (m_Max - m_Min);

			if( m_bHorizontal )
			{
				Draw_FillRect(dc, SYS_Get_Color(wxSYS_COLOUR_3DFACE), (int)x, 0, Width, Height);
			}
			else
			{
				Draw_FillRect(dc, SYS_Get_Color(wxSYS_COLOUR_3DFACE), 0, (int)x, Height, Width);
			}
		}
	}
}

//---------------------------------------------------------
void CVIEW_Ruler::_Draw_Position(wxDC &dc, int Width, int Height, int Position)
{
	if( Position >= 0 && Position < Width )
	{
		if( m_bHorizontal )
		{
			dc.DrawLine(Position, 0, Position, Height);
		}
		else
		{
			dc.DrawLine(0, Position, Height, Position);
		}
	}
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Ruler::Set_Mode(int Mode)
{
	Mode = (Mode & RULER_MODE_SCALE) ? 1 : ((Mode & RULER_MODE_CORNERS) ? 2 : 0);

	if( m_Mode != Mode )
	{
		m_Mode  = Mode;

		Refresh();
	}
}

//---------------------------------------------------------
void CVIEW_Ruler::Set_Range(double Min, double Max)
{
	if( m_Min != Min || m_Max != Max )
	{
		m_Min = Min;
		m_Max = Max;

		Refresh();
	}
}

//---------------------------------------------------------
void CVIEW_Ruler::Set_Range_Core(double Min, double Max)
{
	if( m_Min_Core != Min || m_Max_Core != Max )
	{
		m_Min_Core = Min;
		m_Max_Core = Max;

		Refresh();
	}
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Ruler::Set_Position(int Position)
{
	if( m_Position[0] != Position )
	{
	#ifdef _SAGA_MSW
		wxClientDC dc(this); wxRect r(GetClientAreaOrigin(), GetClientSize());

		int Width  = m_bHorizontal ? r.GetWidth () : r.GetHeight();
		int Height = m_bHorizontal ? r.GetHeight() : r.GetWidth ();

		dc.SetLogicalFunction(wxINVERT);
		_Draw_Position(dc, Width, Height, m_Position[0]);
		_Draw_Position(dc, Width, Height, m_Position[0] = m_Position[1] = Position);
	#else
		m_Position[1] = Position;

		Refresh(false);
	#endif
	}
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
