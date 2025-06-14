
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
//                WKSP_Layer_Classify.cpp                //
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
#include "helper.h"

#include "wksp_layer.h"
#include "wksp_layer_classify.h"
#include "wksp_grid.h"
#include "wksp_grids.h"
#include "wksp_shapes.h"
#include "wksp_pointcloud.h"
#include "wksp_tin.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Scaler::CSG_Scaler(void)
{
	Create(0., 0.);
}

//-----------------------------------------------------
CSG_Scaler::CSG_Scaler(double Minimum, double Maximum, double Interval)
{
	Create(Minimum, Maximum, Interval);
}

//-----------------------------------------------------
bool CSG_Scaler::Create(double Minimum, double Maximum, double Interval)
{
	return( Set_Interval(Interval) && Set_Range(Minimum, Maximum) );
}

//-----------------------------------------------------
bool CSG_Scaler::Set_Range(double Minimum, double Maximum)
{
	m_Minimum	= Minimum < Maximum ? Minimum : Maximum;
	m_Range		= Minimum < Maximum ? Maximum - Minimum : Minimum - Maximum;

	return( m_Range > 0. );
}

bool CSG_Scaler::Set_Interval(double Interval)
{
	m_Interval	= Interval < 0. ? DECREASING : Interval > 0. ? INCREASING : LINEAR;
	m_LogRange	= fabs(Interval);

	return( true );
}

//-----------------------------------------------------
bool CSG_Scaler::Set_Linear(CSG_Table *pTable, int Field, double Interval, double Minimum, double Maximum)
{
	Minimum	= pTable->Get_Minimum(Field) + pTable->Get_Range(Field) * Minimum / 100.;
	Maximum	= pTable->Get_Minimum(Field) + pTable->Get_Range(Field) * Maximum / 100.;

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_StdDev(CSG_Table *pTable, int Field, double Interval, double StdDev, bool bKeepInRange)
{
	double	Minimum	= pTable->Get_Mean(Field) - StdDev * pTable->Get_StdDev(Field); if( bKeepInRange && Minimum < pTable->Get_Minimum(Field) ) Minimum = pTable->Get_Minimum(Field);
	double	Maximum	= pTable->Get_Mean(Field) + StdDev * pTable->Get_StdDev(Field); if( bKeepInRange && Maximum > pTable->Get_Maximum(Field) ) Maximum = pTable->Get_Maximum(Field);

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_Percentile(CSG_Table *pTable, int Field, double Interval, double Minimum, double Maximum)
{
	return( false );

	//	double	Minimum	= pTable->Get_Percentile(Field,        Percentile);
	//	double	Maximum	= pTable->Get_Percentile(Field, 100. - Percentile);

	//	return( Create(Minimum, Maximum, Interval) );
}


//-----------------------------------------------------
bool CSG_Scaler::Set_Linear(CSG_Grid *pGrid, double Interval, double Minimum, double Maximum)
{
	Minimum	= pGrid->Get_Min() + pGrid->Get_Range() * Minimum / 100.;
	Maximum	= pGrid->Get_Min() + pGrid->Get_Range() * Maximum / 100.;

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_StdDev(CSG_Grid *pGrid, double Interval, double StdDev, bool bKeepInRange)
{
	double	Minimum	= pGrid->Get_Mean() - StdDev * pGrid->Get_StdDev(); if( bKeepInRange && Minimum < pGrid->Get_Min() ) Minimum = pGrid->Get_Min();
	double	Maximum	= pGrid->Get_Mean() + StdDev * pGrid->Get_StdDev(); if( bKeepInRange && Maximum > pGrid->Get_Max() ) Maximum = pGrid->Get_Max();

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_Percentile(CSG_Grid *pGrid, double Interval, double Minimum, double Maximum)
{
	Minimum	= pGrid->Get_Percentile(Minimum);
	Maximum	= pGrid->Get_Percentile(Maximum);

	return( Create(Minimum, Maximum, Interval) );
}

//-----------------------------------------------------
bool CSG_Scaler::Set_Linear(CSG_Grids *pGrids, double Interval, double Minimum, double Maximum)
{
	Minimum	= pGrids->Get_Min() + pGrids->Get_Range() * Minimum / 100.;
	Maximum	= pGrids->Get_Min() + pGrids->Get_Range() * Maximum / 100.;

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_StdDev(CSG_Grids *pGrids, double Interval, double StdDev, bool bKeepInRange)
{
	double	Minimum	= pGrids->Get_Mean() - StdDev * pGrids->Get_StdDev(); if( bKeepInRange && Minimum < pGrids->Get_Min() ) Minimum = pGrids->Get_Min();
	double	Maximum	= pGrids->Get_Mean() + StdDev * pGrids->Get_StdDev(); if( bKeepInRange && Maximum > pGrids->Get_Max() ) Maximum = pGrids->Get_Max();

	return( Create(Minimum, Maximum, Interval) );
}

bool CSG_Scaler::Set_Percentile(CSG_Grids *pGrids, double Interval, double Minimum, double Maximum)
{
	Minimum	= pGrids->Get_Percentile(Minimum);
	Maximum	= pGrids->Get_Percentile(Maximum);

	return( Create(Minimum, Maximum, Interval) );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Layer_Classify::CWKSP_Layer_Classify(void)
{
	m_Count			= 64;

	m_Mode			= CLASSIFY_SINGLE;
	m_Shade_Mode	= SHADE_MODE_DSC_GREY;

	m_pLayer		= NULL;
	m_pColors		= NULL;
	m_pLUT			= NULL;
}

//---------------------------------------------------------
CWKSP_Layer_Classify::~CWKSP_Layer_Classify(void)
{}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Layer_Classify::Initialise(CWKSP_Layer *pLayer, CSG_Table *pLUT, CSG_Colors *pColors)
{
	m_pLayer	= pLayer;
	m_pLUT		= pLUT;
	m_pColors	= pColors;

	m_Count		=  m_pLayer->Get_Object()->Get_ObjectType() == SG_DATAOBJECT_TYPE_Grid
				|| m_pLayer->Get_Object()->Get_ObjectType() == SG_DATAOBJECT_TYPE_Grids
				|| m_pLayer->Get_Object()->Get_ObjectType() == SG_DATAOBJECT_TYPE_PointCloud ? 64 : 16;

	//-----------------------------------------------------
	if( m_pLUT && m_pLUT->Get_Count() == 0 )
	{
		//-------------------------------------------------
		#define nColours 10

		static int Colours[nColours]	=
		{
			SG_COLOR_BLUE,
			SG_COLOR_GREEN,
			SG_COLOR_RED,
			SG_COLOR_YELLOW_DARK,
			SG_COLOR_BLUE_DARK,
			SG_COLOR_GREEN_DARK,
			SG_COLOR_RED_DARK,
			SG_COLOR_BLUE_GREEN,
			SG_COLOR_PURPLE,
			SG_COLOR_PINK
		};

		int Colour = (int)CSG_Random::Get_Uniform(0, nColours); 

		CSG_Table_Record *pRecord;

		pRecord	= m_pLUT->Add_Record();
		pRecord->Set_Value(LUT_COLOR		, Colours[Colour]); Colour = (Colour + 1) % nColours;
		pRecord->Set_Value(LUT_TITLE		, _TL("Class 1"));
		pRecord->Set_Value(LUT_DESCRIPTION	, _TL("First Class"));
		pRecord->Set_Value(LUT_MIN			, 0.0);
		pRecord->Set_Value(LUT_MAX			, 1.0);

		pRecord	= m_pLUT->Add_Record();
		pRecord->Set_Value(LUT_COLOR		, Colours[Colour]);
		pRecord->Set_Value(LUT_TITLE		, _TL("Class 2"));
		pRecord->Set_Value(LUT_DESCRIPTION	, _TL("Second Class"));
		pRecord->Set_Value(LUT_MIN			, 1.0);
		pRecord->Set_Value(LUT_MAX			, 2.0);
	}

	if( m_pLUT )
	{
		m_pLUT->Set_Index(LUT_MIN, TABLE_INDEX_Ascending);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
double CWKSP_Layer_Classify::Get_Class_Value_Minimum(int iClass)
{
	switch( m_Mode )
	{
	case CLASSIFY_LUT:
		if( iClass >= 0 && iClass < m_pLUT->Get_Count() )
		{
			return( m_pLUT->Get_Record(iClass)->asDouble(LUT_MIN) );
		}
		break;

	case CLASSIFY_GRADUATED:
	case CLASSIFY_DISCRETE:
	case CLASSIFY_SHADE:
	case CLASSIFY_OVERLAY:
		if( m_zRange > 0.0 )
		{
			return( Get_RelativeToMetric(iClass / (double)Get_Class_Count()) );
		}
		break;

	default: break;
	}

	return( m_zMin );
}

double CWKSP_Layer_Classify::Get_Class_Value_Maximum(int iClass)
{
	switch( m_Mode )
	{
	case CLASSIFY_LUT:
		if( iClass >= 0 && iClass < m_pLUT->Get_Count() )
		{
			return( m_pLUT->Get_Record(iClass)->asDouble(LUT_MAX) );
		}
		break;

	case CLASSIFY_GRADUATED:
	case CLASSIFY_DISCRETE:
	case CLASSIFY_SHADE:
	case CLASSIFY_OVERLAY:
		if( m_zRange > 0.0 )
		{
			return( Get_RelativeToMetric((1.0 + iClass) / (double)Get_Class_Count()) );
		}
		break;

	default: break;
	}

	return( m_zMin + m_zRange );
}

double CWKSP_Layer_Classify::Get_Class_Value_Center(int iClass)
{
	switch( m_Mode )
	{
	case CLASSIFY_LUT:
		if( iClass >= 0 && iClass < m_pLUT->Get_Count() )
		{
			return( 0.5 * (m_pLUT->Get_Record(iClass)->asDouble(LUT_MIN) + m_pLUT->Get_Record(iClass)->asDouble(LUT_MAX)) );
		}
		break;

	case CLASSIFY_GRADUATED:
	case CLASSIFY_DISCRETE:
	case CLASSIFY_SHADE:
	case CLASSIFY_OVERLAY:
		if( m_zRange > 0.0 )
		{
			return( Get_RelativeToMetric((0.5 + iClass) / (double)Get_Class_Count()) );
		}
		break;

	default: break;
	}

	return( m_zMin + 0.5 * m_zRange );
}

//---------------------------------------------------------
wxString CWKSP_Layer_Classify::Get_Class_Name(int iClass)
{
	CSG_String	s;

	switch( m_Mode )
	{
	case CLASSIFY_LUT:
		if( iClass >= 0 && iClass < m_pLUT->Get_Count() )
		{
			s.Printf(SG_T("%s"), m_pLUT->Get_Record(iClass)->asString(LUT_TITLE));
		}
		break;

	case CLASSIFY_GRADUATED:
	case CLASSIFY_DISCRETE:
	case CLASSIFY_SHADE:
	case CLASSIFY_OVERLAY:
		s	= SG_Get_String(Get_Class_Value_Minimum(iClass), -2) + SG_T(" < ")
			+ SG_Get_String(Get_Class_Value_Maximum(iClass), -2);
		break;

	default: break;
	}

	return( s.c_str() );
}

//---------------------------------------------------------
wxString CWKSP_Layer_Classify::Get_Class_Name_byValue(double Value)
{
	return( Get_Class_Name(Get_Class(Value)) );
}

wxString CWKSP_Layer_Classify::Get_Class_Name_byValue(const wxString &Value)
{
	return( SG_Data_Type_is_Numeric(m_pLUT->Get_Field_Type(LUT_MIN))
		? Get_Class_Name(Get_Class(CSG_String(&Value).asDouble()))
		: Get_Class_Name(Get_Class(CSG_String(&Value)))
	);
}


///////////////////////////////////////////////////////////
//                                                       //
//						Unique							 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Layer_Classify::Set_Unique_Color(int Color)
{
	m_UNI_Color	= Color;
}


///////////////////////////////////////////////////////////
//                                                       //
//						Metric							 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Layer_Classify::Set_Class_Count(int Count)
{
	if( Count > 0 && Count != m_Count )
	{
		m_Count	= Count;

		return( Histogram_Update() );
	}

	return( false );
}

//---------------------------------------------------------
CSG_Colors CWKSP_Layer_Classify::Get_Class_Colors(void)	const
{
	CSG_Colors Colors(Get_Class_Count());

	for(int i=0; i<Get_Class_Count(); i++)
	{
		Colors[i] = Get_Class_Color(i);
	}

	return( Colors );
}

//---------------------------------------------------------
void CWKSP_Layer_Classify::Set_Metric(int Mode, double LogFactor, double zMin, double zMax)
{
	m_zMode     = Mode;
	m_zMin      = zMin < zMax ? zMin : zMax;
	m_zRange    = zMin < zMax ? (zMax - zMin) : (zMin - zMax);
	m_zLogRange = pow(10., LogFactor);
	m_zLogMax   = log(1. + m_zLogRange);

	if( m_zRange <= 0. || (m_zMode != 0 && m_zLogRange <= 0.) )
	{
		m_zRange = 0.;
	}
}


///////////////////////////////////////////////////////////
//                                                       //
//						Lookup Table					 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline int CWKSP_Layer_Classify::_LUT_Cmp_Class(double Value, int iClass)	const
{
	CSG_Table_Record	*pClass	= m_pLUT->Get_Record_byIndex(iClass);

	double	min	= pClass->asDouble(LUT_MIN);

	if( Value == min )	{	return(  0 );	}
	if( Value  < min )	{	return(  1 );	}

	double	max	= pClass->asDouble(LUT_MAX);

	if( max    < min )	{	return( -1 );	}
	if( Value  < max )	{	return(  0 );	}

	return( iClass == m_pLUT->Get_Count() - 1 && Value == max ? 0 : -1 );
}

//---------------------------------------------------------
int CWKSP_Layer_Classify::_LUT_Get_Class(double Value)	const
{
	int		a, b, i, c;

	if( m_pLUT->Get_Count() > 0 )
	{
		if( m_pLUT->Get_Index_Field(0) != LUT_MIN || m_pLUT->Get_Index_Order(0) != TABLE_INDEX_Ascending )
		{
			m_pLUT->Set_Index(LUT_MIN, TABLE_INDEX_Ascending);
		}

		for(a=0, b=m_pLUT->Get_Count()-1; a < b; )
		{
			i	= a + (b - a) / 2;
			c	= _LUT_Cmp_Class(Value, i);

			if( c > 0 )
			{
				b	= b > i ? i : b - 1;
			}
			else if( c < 0 )
			{
				a	= a < i ? i : a + 1;
			}
			else
			{
				return( m_pLUT->Get_Record_byIndex(i)->Get_Index() );
			}
		}

		if( _LUT_Cmp_Class(Value, a) == 0 )
		{
			return( m_pLUT->Get_Record_byIndex(a)->Get_Index() );
		}

		if( a != b && _LUT_Cmp_Class(Value, b) == 0 )
		{
			return( m_pLUT->Get_Record_byIndex(b)->Get_Index() );
		}
	}

	return( -1 );
}

//---------------------------------------------------------
inline int CWKSP_Layer_Classify::_LUT_Cmp_Class(const CSG_String &Value, int iClass)	const
{
	CSG_Table_Record	*pClass	= m_pLUT->Get_Record_byIndex(iClass);

	int		c	= Value.Cmp(pClass->asString(LUT_MIN));

	if( c < 0 )
	{
		return( 1 );
	}

	if( c > 0 && Value.Cmp(pClass->asString(LUT_MAX)) > 0 )
	{
		return( -1 );
	}

	return( 0 );
}

//---------------------------------------------------------
int CWKSP_Layer_Classify::_LUT_Get_Class(const CSG_String &Value)	const
{
	int		a, b, i, c;

	if( m_pLUT->Get_Count() > 0 )
	{
		if( m_pLUT->Get_Index_Field(0) != LUT_MIN || m_pLUT->Get_Index_Order(0) != TABLE_INDEX_Ascending )
		{
			m_pLUT->Set_Index(LUT_MIN, TABLE_INDEX_Ascending);
		}

		for(a=0, b=m_pLUT->Get_Count()-1; a < b; )
		{
			i	= a + (b - a) / 2;
			c	= _LUT_Cmp_Class(Value, i);

			if( c > 0 )
			{
				b	= b > i ? i : b - 1;
			}
			else if( c < 0 )
			{
				a	= a < i ? i : a + 1;
			}
			else
			{
				return( m_pLUT->Get_Record_byIndex(i)->Get_Index() );
			}
		}

		if( _LUT_Cmp_Class(Value, a) == 0 )
		{
			return( m_pLUT->Get_Record_byIndex(a)->Get_Index() );
		}

		if( a != b && _LUT_Cmp_Class(Value, b) == 0 )
		{
			return( m_pLUT->Get_Record_byIndex(b)->Get_Index() );
		}
	}

	return( -1 );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Layer_Classify::Metric2EqualElements(void)
{
	if( m_pLayer->Get_Type() == WKSP_ITEM_Grid && m_pColors->Get_Count() > 1 )
	{
		m_pLUT->Del_Records();

		CSG_Grid	*pGrid	= ((CWKSP_Grid *)m_pLayer)->Get_Grid();

		double	zA, zB, dClass	= (double)pGrid->Get_NCells() / (double)(m_pColors->Get_Count());

		if( !pGrid->Set_Index() )
		{
			return;
		}

		int		x, y;

		pGrid->Get_Sorted(0, x, y, false, false);
		zA		= pGrid->asDouble(x, y);

		for(int iClass=0; iClass<m_pColors->Get_Count()-1; iClass++)
		{
			pGrid->Get_Sorted((sLong)(dClass * (iClass + 1.0)), x, y, false, false);
			zB		= zA;
			zA		= pGrid->asDouble(x, y);

			CSG_Table_Record	*pClass	= m_pLUT->Add_Record();
			pClass->Set_Value(LUT_COLOR      , m_pColors->Get_Color(iClass));
			pClass->Set_Value(LUT_TITLE      , CSG_String::Format(">=%f"     , zB    ));
			pClass->Set_Value(LUT_DESCRIPTION, CSG_String::Format("%f <-> %f", zB, zA));
			pClass->Set_Value(LUT_MIN        , zB);
			pClass->Set_Value(LUT_MAX        , zA);
		}

		pGrid->Get_Sorted(pGrid->Get_NCells() - 1, x, y, false, false);
		zB		= zA;
		zA		= pGrid->asDouble(x, y);

		CSG_Table_Record	*pClass	= m_pLUT->Add_Record();
		pClass->Set_Value(LUT_COLOR      , m_pColors->Get_Color(m_pColors->Get_Count() - 1));
		pClass->Set_Value(LUT_TITLE      , CSG_String::Format(">=%f"     , zB    ));
		pClass->Set_Value(LUT_DESCRIPTION, CSG_String::Format("%f <-> %f", zB, zA));
		pClass->Set_Value(LUT_MIN        , zB);
		pClass->Set_Value(LUT_MAX        , zA);

		Set_Mode(CLASSIFY_LUT);
		m_pLayer->Update_Views();
	}
}


///////////////////////////////////////////////////////////
//                                                       //
//						Histogram						 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Layer_Classify::Histogram_Update(void)
{
	m_Statistics.Create();

	if( Get_Class_Count() < 1 )
	{
		m_Histogram.Destroy();

		return( false );
	}

	//-----------------------------------------------------
	STATUSBAR_Set_Text(_TL("Build Histogram..."));

	m_Histogram.Create(Get_Class_Count(), 0., Get_Class_Count() - 1.);

	switch( m_pLayer->Get_Type() )
	{
	case WKSP_ITEM_Grid      :
		_Histogram_Update(((CWKSP_Grid  *)m_pLayer)->Get_Grid ());
		break;

	case WKSP_ITEM_Grids     :
		if( m_Mode == CLASSIFY_OVERLAY )
		{
			_Histogram_Update(((CWKSP_Grids *)m_pLayer)->Get_Grids());
		}
		else
		{
			_Histogram_Update(((CWKSP_Grids *)m_pLayer)->Get_Grid ());
		}
		break;

	case WKSP_ITEM_Shapes    :
		_Histogram_Update(m_pLayer->Get_Object()->asTable(true),
			((CWKSP_Shapes     *)m_pLayer)->Get_Field_Value (),
			((CWKSP_Shapes     *)m_pLayer)->Get_Field_Normal(),
			((CWKSP_Shapes     *)m_pLayer)->Get_Scale_Normal()
		);
		break;

	case WKSP_ITEM_PointCloud:
		_Histogram_Update(m_pLayer->Get_Object()->asTable(true),
			((CWKSP_PointCloud *)m_pLayer)->Get_Field_Value (),
			((CWKSP_PointCloud *)m_pLayer)->Get_Field_Normal(),
			((CWKSP_PointCloud *)m_pLayer)->Get_Scale_Normal()
		);
		break;

	case WKSP_ITEM_TIN       :
		_Histogram_Update(m_pLayer->Get_Object()->asTable(true),
			((CWKSP_TIN        *)m_pLayer)->Get_Field_Value (),
			((CWKSP_TIN        *)m_pLayer)->Get_Field_Normal(),
			((CWKSP_TIN        *)m_pLayer)->Get_Scale_Normal()
		);
		break;

	default: break;
	}

	m_Histogram.Update();

	PROCESS_Set_Okay();

	//-----------------------------------------------------
	return( true );
}

//---------------------------------------------------------
bool CWKSP_Layer_Classify::_Histogram_Update(CSG_Grid *pGrid)
{
	if( pGrid->Get_Max_Samples() > 0 && pGrid->Get_Max_Samples() < pGrid->Get_NCells() )
	{
		double d = (double)pGrid->Get_NCells() / (double)pGrid->Get_Max_Samples();

		for(double i=0; i<(double)pGrid->Get_NCells() && PROGRESSBAR_Set_Position(i, (double)pGrid->Get_NCells()); i+=d)
		{
			if( !pGrid->is_NoData((sLong)i) )
			{
				m_Histogram	+= Get_Class(pGrid->asDouble((sLong)i));
			}
		}

		if( m_Histogram.Update() && m_Histogram.Get_Element_Count() < (size_t)pGrid->Get_Max_Samples() )	// any no-data cells ?
		{
			d *= (double)m_Histogram.Get_Element_Count() / (double)pGrid->Get_Max_Samples();
		}

		m_Histogram.Scale_Element_Count(d);

		m_Statistics = pGrid->Get_Statistics();

		return( true );
	}

	for(sLong i=0; i<pGrid->Get_NCells() && PROGRESSBAR_Set_Position((double)i, (double)pGrid->Get_NCells()); i++)
	{
		if( !pGrid->is_NoData(i) )
		{
			m_Histogram	+= Get_Class(pGrid->asDouble(i));
		}
	}

	m_Statistics = pGrid->Get_Statistics();

	return( true );
}

//---------------------------------------------------------
bool CWKSP_Layer_Classify::_Histogram_Update(CSG_Grids *pGrids)
{
	if( pGrids->Get_Max_Samples() > 0 && pGrids->Get_Max_Samples() < pGrids->Get_NCells() )
	{
		double d = (double)pGrids->Get_NCells() / (double)pGrids->Get_Max_Samples();

		for(double i=0; i<(double)pGrids->Get_NCells() && PROGRESSBAR_Set_Position(i, (double)pGrids->Get_NCells()); i+=d)
		{
			if( !pGrids->is_NoData((sLong)i) )
			{
				m_Histogram += Get_Class(pGrids->asDouble((sLong)i));
			}
		}

		if( m_Histogram.Update() && m_Histogram.Get_Element_Count() < (size_t)pGrids->Get_Max_Samples() )	// any no-data cells ?
		{
			d *= (double)m_Histogram.Get_Element_Count() / (double)pGrids->Get_Max_Samples();
		}

		m_Histogram.Scale_Element_Count(d);

		m_Statistics = pGrids->Get_Statistics();

		return( true );
	}

	for(sLong i=0; i<pGrids->Get_NCells() && PROGRESSBAR_Set_Position((double)i, (double)pGrids->Get_NCells()); i++)
	{
		if( !pGrids->is_NoData(i) )
		{
			m_Histogram += Get_Class(pGrids->asDouble(i));
		}
	}

	m_Statistics = pGrids->Get_Statistics();

	return( true );
}

//---------------------------------------------------------
bool CWKSP_Layer_Classify::_Histogram_Update(CSG_Table *pTable, int Field, int Normalize, double Scale)
{
	if( !pTable || Field < 0 || Field >= pTable->Get_Field_Count() )
	{
		return( false );
	}

	if( pTable->Get_Max_Samples() > 0 && pTable->Get_Max_Samples() < pTable->Get_Count() )
	{
		double d = (double)pTable->Get_Count() / (double)pTable->Get_Max_Samples();

		for(double i=0; i<(double)pTable->Get_Count() && PROGRESSBAR_Set_Position(i, (double)pTable->Get_Count()); i+=d)
		{
			CSG_Table_Record &Record = *pTable->Get_Record((int)i);

			if( m_Mode == CLASSIFY_LUT )
			{
				m_Histogram	+= SG_Data_Type_is_Numeric(m_pLUT->Get_Field_Type(LUT_MIN))
					? Get_Class(Record.asDouble(Field))
					: Get_Class(Record.asDouble(Field));
			}
			else if( !Record.is_NoData(Field) )
			{
				double z = Record.asDouble(Field);

				if( Normalize < 0 )
				{
					m_Histogram += Get_Class(z); m_Statistics += z;
				}
				else if( !Record.is_NoData(Normalize) && Record.asDouble(Normalize) )
				{
					z *= Scale / Record.asDouble(Normalize);

					m_Histogram += Get_Class(z); m_Statistics += z;
				}
			}
		}

		if( m_Histogram.Update() && m_Histogram.Get_Element_Count() < (size_t)pTable->Get_Max_Samples() )	// any no-data cells ?
		{
			d *= (double)m_Histogram.Get_Element_Count() / (double)pTable->Get_Max_Samples();
		}

		m_Histogram.Scale_Element_Count(d);

		return( true );
	}

	for(sLong i=0; i<pTable->Get_Count() && PROGRESSBAR_Set_Position(i, pTable->Get_Count()); i++)
	{
		CSG_Table_Record &Record = *pTable->Get_Record(i);

		if( m_Mode == CLASSIFY_LUT )
		{
			m_Histogram	+= SG_Data_Type_is_Numeric(m_pLUT->Get_Field_Type(LUT_MIN))
				? Get_Class(Record.asDouble(Field))
				: Get_Class(Record.asString(Field));
		}
		else if( !Record.is_NoData(Field) )
		{
			double z = Record.asDouble(Field);

			if( Normalize < 0 )
			{
				m_Histogram += Get_Class(z); m_Statistics += z;
			}
			else if( !Record.is_NoData(Normalize) && Record.asDouble(Normalize) )
			{
				z *= Scale / Record.asDouble(Normalize);

				m_Histogram += Get_Class(z); m_Statistics += z;
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
