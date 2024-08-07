
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                   Projection_Proj4                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                      crs_base.h                       //
//                                                       //
//                 Copyright (C) 2010 by                 //
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
#ifndef HEADER_INCLUDED__crs_base_H
#define HEADER_INCLUDED__crs_base_H


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "crs_transform.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CCRS_Base : public CSG_Tool
{
public:
	CCRS_Base(void);

	virtual bool			do_Sync_Projections		(void)	const	{	return( false );	}

	static bool				Parameter_Changed		(CSG_Parameters *pParameters, CSG_Parameter *pParameter);
	static bool				Parameters_Enable		(CSG_Parameters *pParameters, CSG_Parameter *pParameter);


protected:

	virtual bool			On_Before_Execution		(void);

	virtual int				On_Parameter_Changed	(CSG_Parameters *pParameters, CSG_Parameter *pParameter);
	virtual int				On_Parameters_Enable	(CSG_Parameters *pParameters, CSG_Parameter *pParameter);

	bool					Get_Projection			(CSG_Projection &Projection);


private:

	bool					Set_User_Parameters		(CSG_Parameters &Parameters);
	bool					Add_User_Projection		(CSG_Parameters &Parameters, const CSG_String &ID, const CSG_String &Args);

	static CSG_String		Get_User_Definition		(CSG_Parameters &Parameters);
	static bool				Set_User_Definition		(CSG_Parameters &Parameters, const CSG_String &Proj4);

};


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CCRS_Picker : public CCRS_Base
{
public:
	CCRS_Picker(void);

	virtual CSG_String		Get_MenuPath		(void)	{	return( _TL("Tools") );	}


protected:

	virtual bool			On_Execute			(void);

};


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CCRS_Transform : public CCRS_Base
{
public:
	CCRS_Transform(void)	{}


protected:

	CSG_CRSProjector		m_Projector;


	virtual bool			On_Execute					(void);

	virtual bool			On_Execute_Transformation	(void)	= 0;

};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__crs_base_H
