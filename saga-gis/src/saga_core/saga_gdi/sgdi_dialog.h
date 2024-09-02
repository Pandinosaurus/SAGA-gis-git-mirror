
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
//                     sgdi_dialog.h                     //
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
#ifndef HEADER_INCLUDED__SAGA_GDI_sgdi_dialog_H
#define HEADER_INCLUDED__SAGA_GDI_sgdi_dialog_H


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "sgdi_controls.h"

#include <wx/stattext.h>


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define SGDI_DLG_STYLE_CTRLS_RIGHT			0x01
#define SGDI_DLG_STYLE_START_MAXIMISED		0x02


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class SGDI_API_DLL_EXPORT CSGDI_Dialog : public wxDialog
{
public:
	CSGDI_Dialog(const wxString &Name, int Style = 0);
	virtual ~CSGDI_Dialog(void);

	virtual int				ShowModal			(void);


protected:

	wxWindow *				Get_Control			(void) { return( m_pCtrl ); }

	void					Add_Spacer			(int Space = SGDI_CTRL_SPACE);
	wxStaticText *			Add_Label			(const wxString &Name, bool bCenter, int ID = wxID_ANY);
	wxButton *				Add_Button			(const wxString &Name, int ID, const wxSize &Size = SGDI_BTN_SIZE);
	wxChoice *				Add_Choice			(const wxString &Name, const wxArrayString &Choices, int iSelect = 0, int ID = wxID_ANY);
	wxCheckBox *			Add_CheckBox		(const wxString &Name, bool bCheck, int ID = wxID_ANY);
	wxTextCtrl *			Add_TextCtrl		(const wxString &Name, int Style = 0, const wxString &Text = wxT(""), int ID = wxID_ANY);
	CSGDI_Slider *			Add_Slider			(const wxString &Name, double Value, double minValue, double maxValue, bool bValueAsPercent = false, int ID = wxID_ANY, int Width = SGDI_CTRL_WIDTH);
	CSGDI_SpinCtrl *		Add_SpinCtrl		(const wxString &Name, double Value, double minValue, double maxValue, bool bValueAsPercent = false, int ID = wxID_ANY, int Width = SGDI_CTRL_WIDTH);
	void					Add_CustomCtrl		(const wxString &Name, wxWindow *pControl);

	bool					Add_Output			(wxWindow *pOutput, bool bReparent = false);
	bool					Add_Output			(wxWindow *pOutput_A, wxWindow *pOutput_B, int Proportion_A = 1, int Proportion_B = 0, bool bReparent = false);


private:

	wxSizer					*m_pCtrl_Sizer;

	class wxStaticBoxSizer	*m_pOutput_Sizer;

	class wxScrolledWindow	*m_pCtrl;


	DECLARE_EVENT_TABLE()

};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__SAGA_GDI_sgdi_dialog_H
