/**********************************************************
 * Version $Id: surfer.h 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                        Grid_IO                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                       Surfer.h                        //
//                                                       //
//                 Copyright (C) 2003 by                 //
//                      Olaf Conrad                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//						Surfer.h						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef HEADER_INCLUDED__Surfer_H
#define HEADER_INCLUDED__Surfer_H


//---------------------------------------------------------
#include "MLB_Interface.h"

//---------------------------------------------------------
class CSurfer_Import : public CSG_Tool
{
public:
	CSurfer_Import(void);
	virtual ~CSurfer_Import(void);

	virtual CSG_String		Get_MenuPath(void)				{	return( _TL("Import") );	}


protected:

	virtual bool			On_Execute(void);


private:

};

//---------------------------------------------------------
class CSurfer_Export : public CSG_Tool_Grid
{
public:
	CSurfer_Export(void);
	virtual ~CSurfer_Export(void);

	virtual CSG_String		Get_MenuPath(void)				{	return( _TL("Export") );	}


protected:

	virtual bool			On_Execute(void);


private:

};

#endif // #ifndef HEADER_INCLUDED__Surfer_H