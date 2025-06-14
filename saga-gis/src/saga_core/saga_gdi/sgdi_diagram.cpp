
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                  Library: SAGA_GDI                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    sgdi_diagram.cpp                   //
//                                                       //
//                 Copyright (C) 2009 by                 //
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
#include <wx/wxprec.h>
#include <wx/settings.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/clipbrd.h>

//---------------------------------------------------------
#include "sgdi_diagram.h"
#include "sgdi_helper.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
BEGIN_EVENT_TABLE(CSGDI_Diagram, wxPanel)
	EVT_LEFT_DOWN			(CSGDI_Diagram::_On_Mouse_Click)
	EVT_RIGHT_DOWN			(CSGDI_Diagram::_On_Mouse_Click)
	EVT_PAINT				(CSGDI_Diagram::_On_Paint)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSGDI_Diagram::CSGDI_Diagram(wxWindow *pParent, const wxString &xLabel, const wxString &yLabel)
	: wxPanel(pParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxSUNKEN_BORDER)
{
	m_xLabel = xLabel;
	m_yLabel = yLabel;
}

//---------------------------------------------------------
CSGDI_Diagram::~CSGDI_Diagram(void)
{
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSGDI_Diagram::Set_xScale(double Minimum, double Maximum)
{
	return( false );
}

//---------------------------------------------------------
bool CSGDI_Diagram::Set_yScale(double Minimum, double Maximum)
{
	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CSGDI_Diagram::_On_Mouse_Click(wxMouseEvent &event)
{
	if( event.RightDown() && SG_UI_Dlg_Continue(_TL("Copy to Clipboard"), _TL("Variogram")) )
	{
		wxBitmap BMP(GetSize()); wxMemoryDC dc;
	
		dc.SelectObject(BMP);
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.Clear();

		_Draw(dc);

		dc.SelectObject(wxNullBitmap);

		if( wxTheClipboard->Open() )
		{
			wxBitmapDataObject *pBMP = new wxBitmapDataObject;
			pBMP->SetBitmap(BMP);
			wxTheClipboard->SetData(pBMP);
			wxTheClipboard->Close();
		}
	}
}

//---------------------------------------------------------
void CSGDI_Diagram::_On_Paint(wxPaintEvent &WXUNUSED(event))
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	wxPaintDC dc(this); _Draw(dc);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define RULER_LABEL_HEIGHT	20

//---------------------------------------------------------
bool CSGDI_Diagram::_Draw(wxDC &dc)
{
	if( m_xMin < m_xMax && m_yMin < m_yMax )
	{
		m_rDiagram = wxRect(RULER_LABEL_HEIGHT, 0, GetClientSize().x - RULER_LABEL_HEIGHT, GetClientSize().y - RULER_LABEL_HEIGHT);

		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.SetFont(wxFont((int)(0.5 * RULER_LABEL_HEIGHT), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

		Draw_Text(dc, TEXTALIGN_TOPCENTER   , 0, m_rDiagram.GetY() + m_rDiagram.GetHeight() / 2,                90, m_yLabel);
		Draw_Text(dc, TEXTALIGN_BOTTOMCENTER,    m_rDiagram.GetX() + m_rDiagram.GetWidth () / 2, GetClientSize().y, m_xLabel);

		Draw_Ruler(dc, m_rDiagram, true , m_xMin, m_xMax);
		Draw_Ruler(dc, m_rDiagram, false, m_yMin, m_yMax);

		On_Draw(dc, m_rDiagram);

		return( true );
	}

	dc.DrawLine(0, 0, GetClientSize().x, GetClientSize().y);
	dc.DrawLine(0, GetClientSize().y, GetClientSize().x, 0);

	return( false );
}

//---------------------------------------------------------
#define DIAGRAM_BUFFER	100

//---------------------------------------------------------
int CSGDI_Diagram::Get_xToScreen(double x, bool bKeepInRange)
{
	int i = m_rDiagram.GetX() + (int)(m_rDiagram.GetWidth () * (x - m_xMin) / (m_xMax - m_xMin));

	if( bKeepInRange )
	{
		if(      i < m_rDiagram.GetLeft  () - DIAGRAM_BUFFER )
		         i = m_rDiagram.GetLeft  () - DIAGRAM_BUFFER;
		else if( i > m_rDiagram.GetRight () + DIAGRAM_BUFFER )
		         i = m_rDiagram.GetRight () + DIAGRAM_BUFFER;
	}

	return( i );
}

//---------------------------------------------------------
int CSGDI_Diagram::Get_yToScreen(double y, bool bKeepInRange)
{
	int i = m_rDiagram.GetY() - (int)(m_rDiagram.GetHeight() * (y - m_yMin) / (m_yMax - m_yMin)) + m_rDiagram.GetHeight();

	if( bKeepInRange )
	{
		if(      i < m_rDiagram.GetTop   () - DIAGRAM_BUFFER )
		         i = m_rDiagram.GetTop   () - DIAGRAM_BUFFER;
		else if( i > m_rDiagram.GetBottom() + DIAGRAM_BUFFER )
		         i = m_rDiagram.GetBottom() + DIAGRAM_BUFFER;
	}

	return( i );
}

//---------------------------------------------------------
bool CSGDI_Diagram::Get_ToScreen(wxPoint &Point, double x, double y)
{
	bool bResult = true;

	Point.x = Get_xToScreen(x, false);

	if( Point.x < m_rDiagram.GetLeft  () - DIAGRAM_BUFFER )
	{
		Point.x = m_rDiagram.GetLeft  () - DIAGRAM_BUFFER; bResult = false;
	} else
	if( Point.x > m_rDiagram.GetRight () + DIAGRAM_BUFFER )
	{
		Point.x = m_rDiagram.GetRight () + DIAGRAM_BUFFER; bResult = false;
	}

	Point.y = Get_yToScreen(y, false);

	if( Point.y < m_rDiagram.GetTop   () - DIAGRAM_BUFFER )
	{
		Point.y = m_rDiagram.GetTop   () - DIAGRAM_BUFFER; bResult = false;
	} else
	if( Point.y > m_rDiagram.GetBottom() + DIAGRAM_BUFFER )
	{
		Point.y = m_rDiagram.GetBottom() + DIAGRAM_BUFFER; bResult = false;
	}

	return( bResult );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
