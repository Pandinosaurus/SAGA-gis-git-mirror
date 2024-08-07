
/*******************************************************************************
    CreateChartLayer.cpp
    Copyright (C) Victor Olaya
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, USA
*******************************************************************************/ 

///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <saga_api/saga_api.h>


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CCreateChartLayer : public CSG_Tool
{
public:
	CCreateChartLayer(void);

	virtual CSG_String			Get_MenuPath	(void)	{	return( _TL("A:Visualization|Shapes") );	}


protected:

	virtual bool				On_Execute(void);


private:

	double						m_fMaxValue, m_fMinValue, m_fMaxSize, m_fMinSize;

	CSG_Shapes					*m_pOutput;


	void						AddPieChart			(CSG_Shape *pShape);
	void						AddBarChart			(CSG_Shape *pShape);

};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
