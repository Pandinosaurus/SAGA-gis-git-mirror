
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
//                WKSP_Data_Menu_Files.h                 //
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
//                University of Hamburg                  //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef _HEADER_INCLUDED__SAGA_GUI__WKSP_Data_Menu_Files_H
#define _HEADER_INCLUDED__SAGA_GUI__WKSP_Data_Menu_Files_H


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "wksp_data_menu_file.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CWKSP_Data_Menu_Files
{
public:
	CWKSP_Data_Menu_Files(void);
	virtual ~CWKSP_Data_Menu_Files(void);

	wxMenu *						Get_Menu		(void)			{	return( m_pMenu );		}

	void							Set_Update		(bool bUpdate)	{	m_bUpdate = bUpdate;	}

	bool							Recent_Open		(int Cmd_ID);
	void							Recent_Add		(CWKSP_Data_Menu_File::Recent_Type Type, const wxString &File);
	void							Recent_Del		(CWKSP_Data_Menu_File::Recent_Type Type, const wxString &File);
	bool							Recent_Get		(CWKSP_Data_Menu_File::Recent_Type Type, wxArrayString &Files, bool bAppend);
	int								Recent_Count	(CWKSP_Data_Menu_File::Recent_Type Type);


private:

	bool							m_bUpdate;

	wxMenu							*m_pMenu;

	CWKSP_Data_Menu_File			m_Folder, m_Project, m_Data;


	CWKSP_Data_Menu_File *			_Get_Menu		(CWKSP_Data_Menu_File::Recent_Type Type);

};


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef _HEADER_INCLUDED__SAGA_GUI__WKSP_Data_Menu_Files_H
