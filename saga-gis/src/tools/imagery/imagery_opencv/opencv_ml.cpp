
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                        OpenCV                         //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     opencv_ml.cpp                     //
//                                                       //
//                 Copyright (C) 2016 by                 //
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
#include "opencv_ml.h"

#if CV_MAJOR_VERSION >= 3


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
enum
{
	CLASS_ID	= 0,
	CLASS_NAME,
	CLASS_COUNT,
	CLASS_R,
	CLASS_G,
	CLASS_B
};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
COpenCV_ML::COpenCV_ML(bool bProbability)
{
	Add_Reference("https://docs.opencv.org/", SG_T("OpenCV - Open Source Computer Vision"));

	Add_Reference("https://docs.opencv.org/4.x/dc/dd6/ml_intro.html", SG_T("OpenCV - Machine Learning Overview"));

	//-----------------------------------------------------
	Parameters.Add_Grid_List("",
		"FEATURES"		, _TL("Features"),
		_TL(""),
		PARAMETER_INPUT, false
	);

	Parameters.Add_Bool("FEATURES",
		"NORMALIZE"		, _TL("Normalize"),
		_TL(""),
		false
	);

	Parameters.Add_Bool("FEATURES",
		"RGB_COLORS"	, _TL("Colors from Features"),
		_TL("Use the first three features in list to obtain blue, green, red components for class colour in look-up table."),
		false
	)->Set_UseInCMD(false);

	Parameters.Add_Grid_System("",
		"GRID_SYSTEM"	, _TL("Grid System"),
		_TL("")
	);

	Parameters.Add_Grid("GRID_SYSTEM",
		"CLASSES"		, _TL("Classification"),
		_TL(""),
		PARAMETER_OUTPUT, true, SG_DATATYPE_Short
	);

	if( bProbability )
	{
		Parameters.Add_Grid("GRID_SYSTEM",
			"PROBABILITY"	, _TL("Probability"),
			_TL(""),
			PARAMETER_OUTPUT_OPTIONAL
		);
	}

	Parameters.Add_Table("",
		"CLASSES_LUT"	, _TL("Look-up Table"),
		_TL("A reference list of the grid values that have been assigned to the training classes."),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Choice("",
		"MODEL_TRAIN"	, _TL("Training"),
		_TL(""),
		CSG_String::Format("%s|%s|%s",
			_TL("training areas"),
			_TL("training samples"),
			_TL("load from file")
		)
	);

	Parameters.Add_Table("MODEL_TRAIN",
		"TRAIN_SAMPLES"	, _TL("Training Samples"),
		_TL("Provide a class identifier in the first field followed by sample data corresponding to the input feature grids."),
		PARAMETER_INPUT
	);

	Parameters.Add_Shapes("MODEL_TRAIN",
		"TRAIN_AREAS"	, _TL("Training Areas"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field("TRAIN_AREAS",
		"TRAIN_CLASS"	, _TL("Class Identifier"),
		_TL("")
	);

	Parameters.Add_Double("TRAIN_AREAS",
		"TRAIN_BUFFER"	, _TL("Buffer Size"),
		_TL("For non-polygon type training areas, creates a buffer with a diameter of specified size."),
		1., 0., true
	);

	Parameters.Add_FilePath("",
		"MODEL_LOAD"	, _TL("Load Model"),
		_TL("Use a model previously stored to file."),
		CSG_String::Format("%s (*.xml)|*.xml|%s|*.*",
			_TL("XML Files"),
			_TL("All Files")
		)
	);

	Parameters.Add_FilePath("MODEL_TRAIN",
		"MODEL_SAVE"	, _TL("Save Model"),
		_TL("Stores model to file to be used for subsequent classifications instead of training areas."),
		CSG_String::Format("%s (*.xml)|*.xml|%s|*.*",
			_TL("XML Files"),
			_TL("All Files")
		), NULL, true
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int COpenCV_ML::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( pParameter->Cmp_Identifier("GRID_SYSTEM") )
	{
		if( pParameter->asGrid_System()->is_Valid() )
		{
			pParameters->Set_Parameter("TRAIN_BUFFER", pParameter->asGrid_System()->Get_Cellsize());
		}
	}

	if( pParameter->Cmp_Identifier("MODEL_LOAD") )
	{
		if( !Check_Model_File(pParameter->asString()) )
		{
			pParameter->Set_Value("");

			Error_Fmt("%s: %s", _TL("invalid model file"), pParameter->asString());
		}
	}

	//-----------------------------------------------------
	return( CSG_Tool::On_Parameter_Changed(pParameters, pParameter) );
}
	
//---------------------------------------------------------
int COpenCV_ML::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( pParameter->Cmp_Identifier("GRID_SYSTEM") )
	{
		pParameters->Set_Enabled("CLASSES"      , pParameter->asGrid_System()->is_Valid());
	}

	if( pParameter->Cmp_Identifier("FEATURES") )
	{
		pParameters->Set_Enabled("RGB_COLORS"   , pParameter->asGridList()->Get_Grid_Count() >= 3 && (*pParameters)("MODEL_TRAIN")->asInt() != 2);
	}

	if( pParameter->Cmp_Identifier("MODEL_TRAIN") )
	{
		pParameter->Set_Children_Enabled(pParameter->asInt() != 2);

		pParameters->Set_Enabled("MODEL_LOAD"   , pParameter->asInt() == 2);
		pParameters->Set_Enabled("TRAIN_SAMPLES", pParameter->asInt() == 1);
		pParameters->Set_Enabled("TRAIN_AREAS"  , pParameter->asInt() == 0);

		pParameters->Set_Enabled("CLASSES_LUT"  , pParameter->asInt() != 2);
		pParameters->Set_Enabled("RGB_COLORS"   , pParameter->asInt() != 2 && (*pParameters)("FEATURES")->asGridList()->Get_Grid_Count() >= 3);
	}

	if( pParameter->Cmp_Identifier("TRAIN_AREAS") )
	{
		pParameters->Set_Enabled("TRAIN_BUFFER" , pParameter->asShapes() && pParameter->asShapes()->Get_Type() != SHAPE_TYPE_Polygon);
	}

	//-----------------------------------------------------
	return( CSG_Tool::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::_Initialize(void)
{
	m_pFeatures = Parameters("FEATURES")->asGridList();

	if( m_pFeatures->Get_Grid_Count() < 1 || !m_pFeatures->Get_Grid(0)->Get_System().is_Valid() )
	{
		Error_Set(_TL("invalid features"));

		return( false );
	}

	m_bNormalize = Parameters("NORMALIZE")->asBool();

	//-----------------------------------------------------
	CSG_Grid_System System(*Parameters("GRID_SYSTEM")->asGrid_System());

	if( !System.is_Valid() )
	{
		System.Create(m_pFeatures->Get_Grid(0)->Get_System());
	}

	//-----------------------------------------------------
	m_pClasses = Parameters("CLASSES")->asGrid();

	if( m_pClasses == NULL )
	{
		Parameters("CLASSES")->Set_Value(m_pClasses = SG_Create_Grid(System, SG_DATATYPE_Short)); DataObject_Add(m_pClasses);
	}
	else if( !m_pClasses->Get_System().is_Equal(System) )
	{
		m_pClasses->Create(System, SG_DATATYPE_Short);
	}

	m_pClasses->Set_NoData_Value(-1.);

	//-----------------------------------------------------
	m_pProbability = (CSG_Grid *)(Parameters("PROBABILITY") ? Parameters("PROBABILITY")->asPointer() : NULL);

	if( m_pProbability == DATAOBJECT_CREATE )
	{
		Parameters("PROBABILITY")->Set_Value(m_pProbability = SG_Create_Grid(System)); DataObject_Add(m_pProbability);
	}
	else if( m_pProbability && !m_pProbability->Get_System().is_Equal(System) )
	{
		m_pProbability->Create(System);
	}

	return( true );
}

//---------------------------------------------------------
bool COpenCV_ML::_Finalize(void)
{
	if( m_Classes.Get_Count() > 0 )
	{
		CSG_Parameter *pLUT = DataObject_Get_Parameter(m_pClasses, "LUT");

		if( pLUT )
		{
			bool bRGB = m_pFeatures->Get_Grid_Count() >= 3 && Parameters("RGB_COLORS")->asBool();

			for(int i=0; i<m_Classes.Get_Count(); i++)
			{
				CSG_Table_Record *pClass = pLUT->asTable()->Get_Record(i);

				if( !pClass )
				{
					(pClass = pLUT->asTable()->Add_Record())->Set_Value(0, SG_Color_Get_Random());
				}

				pClass->Set_Value(1, m_Classes[i].asString(1));
				pClass->Set_Value(2, m_Classes[i].asString(1));
				pClass->Set_Value(3, m_Classes[i].asInt   (0));
				pClass->Set_Value(4, m_Classes[i].asInt   (0));

				if( bRGB )
				{
					#define SET_COLOR_COMPONENT(c, i)	c = 127 + 127 * (m_bNormalize ? c : (c - m_pFeatures->Get_Grid(i)->Get_Mean()) / m_pFeatures->Get_Grid(i)->Get_StdDev());\
						if( c < 0 ) c = 0; else if( c > 255 ) c = 255;

					double r = m_Classes[i].asDouble(CLASS_R) / m_Classes[i].asInt(CLASS_COUNT); SET_COLOR_COMPONENT(r, 2);
					double g = m_Classes[i].asDouble(CLASS_G) / m_Classes[i].asInt(CLASS_COUNT); SET_COLOR_COMPONENT(g, 1);
					double b = m_Classes[i].asDouble(CLASS_B) / m_Classes[i].asInt(CLASS_COUNT); SET_COLOR_COMPONENT(b, 0);

					pClass->Set_Value(0, SG_GET_RGB(r, g, b));
				}
			}

			pLUT->asTable()->Set_Count(m_Classes.Get_Count());

			DataObject_Set_Parameter(m_pClasses, pLUT);
			DataObject_Set_Parameter(m_pClasses, "COLORS_TYPE", 1);	// Color Classification Type: Lookup Table
		}

		//-------------------------------------------------
		if( Parameters("CLASSES_LUT")->asTable() )
		{
			CSG_Table &LUT = *Parameters("CLASSES_LUT")->asTable();

			LUT.Destroy();
			LUT.Set_Name(m_pClasses->Get_Name());
			LUT.Add_Field("VALUE", SG_DATATYPE_Int   );
			LUT.Add_Field("CLASS", SG_DATATYPE_String);

			for(int i=0; i<m_Classes.Get_Count(); i++)
			{
				CSG_Table_Record &Class = *LUT.Add_Record();

				Class.Set_Value(0, m_Classes[i].asInt   (0));
				Class.Set_Value(1, m_Classes[i].asString(1));
			}
		}

		m_Classes.Destroy();
	}

	//-----------------------------------------------------
	CSG_String Name(Get_Model_Name());

	m_pClasses->Fmt_Name("%s [%s]", _TL("Classification"), Name.c_str());

	if( m_pProbability )
	{
		m_pProbability->Fmt_Name("%s %s [%s]", _TL("Classification"), _TL("Probability"), Name.c_str());
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::Check_Model_File(const CSG_String &File)
{
	CSG_MetaData Model;

	return( Model.Load(File) && Model.Cmp_Name("opencv_storage") && Model("opencv_ml_" + CSG_String(Get_Model_ID())) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::_Get_Features(int x, int y, CSG_Vector &Features)
{
	for(int i=0; i<m_pFeatures->Get_Grid_Count(); i++)
	{
		CSG_Grid *pFeature = m_pFeatures->Get_Grid(i);

		if( pFeature->Get_System().is_Equal(m_pClasses->Get_System()) )
		{
			if( pFeature->is_NoData(x, y) )
			{
				return( false );
			}

			Features[i] = pFeature->asDouble(x, y);
		}
		else if( pFeature->Get_Value(m_pClasses->Get_System().Get_Grid_to_World(x, y), Features[i]) == false )
		{
			return( false );
		}

		if( m_bNormalize && pFeature->Get_StdDev() > 0. )
		{
			Features[i] = (Features[i] - pFeature->Get_Mean()) / pFeature->Get_StdDev();
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::_Get_Prediction(const Ptr<StatModel> &Model)
{
	for(int y=0; y<m_pClasses->Get_NY() && Set_Progress(y, m_pClasses->Get_NY()); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<m_pClasses->Get_NX(); x++)
		{
			CSG_Vector Features(m_pFeatures->Get_Grid_Count());

			if( _Get_Features(x, y, Features) )
			{
				Mat	Sample(1, m_pFeatures->Get_Grid_Count(), CV_32FC1);

				for(int i=0; i<m_pFeatures->Get_Grid_Count(); i++)
				{
					Sample.at<float>(i)	= (float)Features[i];
				}

				m_pClasses->Set_Value(x, y, Model->predict(Sample));

				if( m_pProbability )
				{
					m_pProbability->Set_Value(x, y, Get_Probability(Model, Sample));
				}
			}
			else
			{
				m_pClasses->Set_NoData(x, y);

				if( m_pProbability )
				{
					m_pProbability->Set_NoData(x, y);
				}
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::_Get_Training(CSG_Matrix &Data)
{
	m_Classes.Destroy();

	m_Classes.Add_Field("ID"   , SG_DATATYPE_Int   );	// CLASS_ID
	m_Classes.Add_Field("NAME" , SG_DATATYPE_String);	// CLASS_NAME
	m_Classes.Add_Field("COUNT", SG_DATATYPE_Int   );	// CLASS_COUNT
	m_Classes.Add_Field("RED"  , SG_DATATYPE_Double);	// CLASS_R
	m_Classes.Add_Field("GREEN", SG_DATATYPE_Double);	// CLASS_G
	m_Classes.Add_Field("BLUE" , SG_DATATYPE_Double);	// CLASS_B

	//-----------------------------------------------------
	if( Parameters("MODEL_TRAIN")->asInt() == 1 && Parameters("TRAIN_SAMPLES")->asTable() )
	{
		CSG_Table *pSamples = Parameters("TRAIN_SAMPLES")->asTable();

		if( pSamples->Get_Field_Count() < m_pFeatures->Get_Grid_Count() + 1 )
		{
			Error_Set(_TL("Training samples have to provide a class identifier in the first field followed by a value for each feature."));

			return( false );
		}

		int Field = 0;

		CSG_Index Index; pSamples->Set_Index(Index, Field);

		CSG_String Label; CSG_Table_Record *pClass = NULL;

		//-------------------------------------------------
		for(sLong i=0, ID=0; i<pSamples->Get_Count(); i++)
		{
			CSG_Table_Record *pSample = pSamples->Get_Record(Index[i]);

			if( !pClass || Label.Cmp(pSample->asString(Field)) )
			{
				pClass = m_Classes.Add_Record();

				pClass->Set_Value(CLASS_ID  , ID++);
				pClass->Set_Value(CLASS_NAME, Label = pSample->asString(Field));
			}

			_Get_Training(Data, pClass, pSample);
		}
	}

	//-----------------------------------------------------
	else if( Parameters("MODEL_TRAIN")->asInt() == 0 && Parameters("TRAIN_AREAS")->asShapes() )
	{
		CSG_Shapes Polygons, *pPolygons = Parameters("TRAIN_AREAS")->asShapes();

		int Field = Parameters("TRAIN_CLASS")->asInt();

		if( pPolygons->Get_Type() != SHAPE_TYPE_Polygon )
		{
			double Buffer = Parameters("TRAIN_BUFFER")->asDouble() / 2.;	// diameter, not radius!

			if( Buffer <= 0. )
			{
				Error_Set(_TL("buffer size must not be zero"));

				return( false );
			}

			Polygons.Create(SHAPE_TYPE_Polygon);
			Polygons.Add_Field(pPolygons->Get_Field_Name(Field), pPolygons->Get_Field_Type(Field));

			for(sLong i=0; i<pPolygons->Get_Count(); i++)
			{
				CSG_Shape *pShape = pPolygons->Get_Shape(i), *pBuffer = Polygons.Add_Shape();

				*pBuffer->Get_Value(0) = *pShape->Get_Value(Field);

				SG_Shape_Get_Offset(pShape, Buffer, 5 * M_DEG_TO_RAD, pBuffer);
			}

			pPolygons = &Polygons; Field = 0;
		}

		//-------------------------------------------------
		CSG_Index Index; pPolygons->Set_Index(Index, Field);

		CSG_String Label; CSG_Table_Record *pClass = NULL;

		//-------------------------------------------------
		for(sLong i=0, ID=0; i<pPolygons->Get_Count(); i++)
		{
			CSG_Shape_Polygon *pPolygon = pPolygons->Get_Shape(Index[i])->asPolygon();

			if( !pClass || Label.Cmp(pPolygon->asString(Field)) )
			{
				pClass = m_Classes.Add_Record();

				pClass->Set_Value(CLASS_ID  , ID++);
				pClass->Set_Value(CLASS_NAME, Label = pPolygon->asString(Field));
			}

			_Get_Training(Data, pClass, pPolygon);
		}

		//-------------------------------------------------
		for(sLong iClass=m_Classes.Get_Count()-1; iClass>=0; iClass--)
		{
			if( m_Classes[iClass].asInt(CLASS_COUNT) < 1 )
			{
				m_Classes.Del_Record(iClass);
			}
		}
	}

	//-----------------------------------------------------
	return( m_Classes.Get_Count() > 1 );
}

//---------------------------------------------------------
bool COpenCV_ML::_Get_Training(CSG_Matrix &Data, CSG_Table_Record *pClass, CSG_Table_Record *pSample)
{
	double r = 0., g = 0., b = 0.;

	CSG_Vector z(1 + (sLong)m_pFeatures->Get_Grid_Count());

	z[m_pFeatures->Get_Grid_Count()] = pClass->asInt(CLASS_ID);

	for(int i=0; i<m_pFeatures->Get_Grid_Count(); i++)
	{
		double Feature = pSample->asDouble(i + 1);

		if( m_bNormalize )
		{
			CSG_Grid *pFeature = m_pFeatures->Get_Grid(i);

			Feature = (Feature - pFeature->Get_Mean()) / pFeature->Get_StdDev();
		}

		z[i] = Feature;
	}

	Data.Add_Row(z);

	if( m_pFeatures->Get_Grid_Count() >= 3 )
	{
		r += z[2]; g += z[1]; b += z[0];
	}

	pClass->Add_Value(CLASS_COUNT, 1);
	pClass->Add_Value(CLASS_R    , r);
	pClass->Add_Value(CLASS_G    , g);
	pClass->Add_Value(CLASS_B    , b);

	return( true );
}

//---------------------------------------------------------
bool COpenCV_ML::_Get_Training(CSG_Matrix &Data, CSG_Table_Record *pClass, CSG_Shape_Polygon *pPolygon)
{
	int n = 0; double r = 0., g = 0., b = 0.; CSG_Vector Features(m_pFeatures->Get_Grid_Count()), z(1 + (sLong)m_pFeatures->Get_Grid_Count()); CSG_Grid_System System(m_pClasses->Get_System());

	for(int i=0; i<pPolygon->Get_Part_Count(); i++)
	{
		CSG_Shape_Polygon_Part *pPart = pPolygon->Get_Polygon_Part(i);

		if( pPart->Get_Extent().Intersects(System.Get_Extent()) )
		{
			int xMin = System.Get_xWorld_to_Grid(pPart->Get_Extent().Get_XMin()); if( xMin <  0               ) { xMin = 0                  ; }
			int xMax = System.Get_xWorld_to_Grid(pPart->Get_Extent().Get_XMax()); if( xMax >= System.Get_NX() ) { xMax = System.Get_NX() - 1; }
			int yMin = System.Get_yWorld_to_Grid(pPart->Get_Extent().Get_YMin()); if( yMin <  0               ) { yMin = 0                  ; }
			int yMax = System.Get_yWorld_to_Grid(pPart->Get_Extent().Get_YMax()); if( yMax >= System.Get_NY() ) { yMax = System.Get_NY() - 1; }

			for(int y=yMin; y<=yMax; y++) for(int x=xMin; x<=xMax; x++)
			{
				if( pPart->Contains(System.Get_Grid_to_World(x, y)) && _Get_Features(x, y, Features) )
				{
					z[m_pFeatures->Get_Grid_Count()] = pClass->asInt(CLASS_ID);

					for(int i=0; i<m_pFeatures->Get_Grid_Count(); i++)
					{
						z[i] = Features[i];
					}

					Data.Add_Row(z);

					if( m_pFeatures->Get_Grid_Count() >= 3 )
					{
						r += z[2]; g += z[1]; b += z[0];
					}

					n++;
				}
			}
		}
	}

	if( n > 0 )
	{
		pClass->Add_Value(CLASS_COUNT, n);
		pClass->Add_Value(CLASS_R    , r);
		pClass->Add_Value(CLASS_G    , g);
		pClass->Add_Value(CLASS_B    , b);

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Ptr<TrainData> COpenCV_ML::Get_Training(const CSG_Matrix &Data)
{
	Mat	Samples (Data.Get_NY(), Data.Get_NX() - 1, CV_32F);
	Mat	Response(Data.Get_NY(),                 1, CV_32S);

	for(int i=0; i<Data.Get_NY(); i++)
	{
		Response.at<int>(i)	= (int)Data[i][Data.Get_NX() - 1];

		for(int j=0; j<Samples.cols; j++)
		{
			Samples.at<float>(i, j)	= (float)Data[i][j];
		}
	}

	//-----------------------------------------------------
	return( TrainData::create(Samples, ROW_SAMPLE, Response) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool COpenCV_ML::On_Execute(void)
{
	//-----------------------------------------------------
	Process_Set_Text(_TL("initializing"));

	if( !_Initialize() )
	{
		return( false );
	}

	Ptr<StatModel> Model;

	//-----------------------------------------------------
	if( SG_File_Exists(Parameters("MODEL_LOAD")->asString()) )
	{
		Model = Get_Model(Parameters("MODEL_LOAD")->asString());
	}
	else
	{
		Process_Set_Text(_TL("preparing training"));

		CSG_Matrix Data;

		if( !_Get_Training(Data) )
		{
			return( false );
		}

		Ptr<TrainData> tData = Get_Training(Data); Data.Destroy();

		Process_Set_Text(_TL("training"));

		Model = Get_Model();

		try
		{
			Model->train(tData);
		}
		catch(...)
		{
			Error_Set(_TL("Failed to train the model."));

			return( false );
		}

		if( *Parameters("MODEL_SAVE")->asString() )
		{
			CSG_String File(Parameters("MODEL_SAVE")->asString());

			Model->save(File.b_str());
		}
	}

	//-----------------------------------------------------
	if( !Model->isTrained() )
	{
		Error_Set(_TL("Model is not trained"));

		return( false );
	}

	Process_Set_Text(_TL("prediction"));

	_Get_Prediction(Model);

	//-----------------------------------------------------
	return( _Finalize() );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // CV_MAJOR_VERSION >= 3


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
