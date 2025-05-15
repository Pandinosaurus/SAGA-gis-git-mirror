
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                  Library: SAGA_API                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    pointcloud.cpp                     //
//                                                       //
//          Copyright (C) 2009 by Olaf Conrad            //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'.                              //
//                                                       //
// This library is free software; you can redistribute   //
// it and/or modify it under the terms of the GNU Lesser //
// General Public License as published by the Free       //
// Software Foundation, either version 2.1 of the        //
// License, or (at your option) any later version.       //
//                                                       //
// This library is distributed in the hope that it will  //
// be useful, but WITHOUT ANY WARRANTY; without even the //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU Lesser General Public //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU Lesser     //
// General Public License along with this program; if    //
// not, see <http://www.gnu.org/licenses/>.              //
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
#include "pointcloud.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define PC_FILE_VERSION "SGPC01"

#define PC_SIZE_STRING       32
#define PC_SIZE_DATE         32
#define PC_SIZE_TYPE(type)   (type == SG_DATATYPE_String ? PC_SIZE_STRING : type == SG_DATATYPE_Date ? PC_SIZE_DATE : (int)SG_Data_Type_Get_Size(type))
#define PC_SIZE_FIELD(field) PC_SIZE_TYPE(m_Field_Info[field]->m_Type)


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_PointCloud * SG_Create_PointCloud(void)
{
	return( new CSG_PointCloud );
}

//---------------------------------------------------------
CSG_PointCloud * SG_Create_PointCloud(const CSG_PointCloud &PointCloud)
{
	return( new CSG_PointCloud(PointCloud) );
}

//---------------------------------------------------------
CSG_PointCloud * SG_Create_PointCloud(const char       *File) { return( SG_Create_PointCloud(CSG_String(File)) ); }
CSG_PointCloud * SG_Create_PointCloud(const wchar_t    *File) { return( SG_Create_PointCloud(CSG_String(File)) ); }
CSG_PointCloud * SG_Create_PointCloud(const CSG_String &File)
{
	CSG_PointCloud *pPoints = new CSG_PointCloud();

	if( pPoints->Create(File) )
	{
		return( pPoints );
	}

	delete(pPoints); return( NULL );
}

//---------------------------------------------------------
CSG_PointCloud * SG_Create_PointCloud(CSG_PointCloud *pStructure)
{
	return( new CSG_PointCloud(pStructure) );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_PointCloud::CSG_PointCloud(void)
	: CSG_Shapes()
{
	_On_Construction(); Create();
}

bool CSG_PointCloud::Create(void)
{
	Destroy();

	Add_Field("", SG_DATATYPE_Undefined); // add x, y, z fields

	return( true );
}

//---------------------------------------------------------
CSG_PointCloud::CSG_PointCloud(const CSG_PointCloud &PointCloud)
	: CSG_Shapes()
{
	_On_Construction(); Create(PointCloud);
}

bool CSG_PointCloud::Create(const CSG_PointCloud &PointCloud)
{
	return( Assign((CSG_Data_Object *)&PointCloud) );
}

//---------------------------------------------------------
CSG_PointCloud::CSG_PointCloud(const char       *File) : CSG_PointCloud(CSG_String(File)) {}
CSG_PointCloud::CSG_PointCloud(const wchar_t    *File) : CSG_PointCloud(CSG_String(File)) {}
CSG_PointCloud::CSG_PointCloud(const CSG_String &File)
	: CSG_Shapes()
{
	_On_Construction(); Create(File);
}

bool CSG_PointCloud::Create(const char       *File) { return( Create(CSG_String(File)) ); }
bool CSG_PointCloud::Create(const wchar_t    *File) { return( Create(CSG_String(File)) ); }
bool CSG_PointCloud::Create(const CSG_String &File)
{
	return( _Load(File) );
}

//---------------------------------------------------------
CSG_PointCloud::CSG_PointCloud(CSG_PointCloud *pTemplate)
	: CSG_Shapes()
{
	_On_Construction(); Create(pTemplate);
}

bool CSG_PointCloud::Create(CSG_PointCloud *pTemplate)
{
	Destroy();

	if( pTemplate && pTemplate->m_nFields > 0 )
	{
		for(int i=0; i<pTemplate->m_nFields; i++)
		{
			_Add_Field(pTemplate->m_Field_Info[i]->m_Name, pTemplate->m_Field_Info[i]->m_Type);
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
void CSG_PointCloud::_On_Construction(void)
{
	m_Type         = SHAPE_TYPE_Point;
	m_Vertex_Type  = SG_VERTEX_TYPE_XYZ;

	Set_NoData_Value(-999999);

	Set_Update_Flag();

	m_Shapes.Create(SHAPE_TYPE_Point, NULL, NULL, SG_VERTEX_TYPE_XYZ);
	m_Shapes.Set_Count(SG_OMP_Get_Max_Num_Procs());
	for(sLong i=0; i<m_Shapes.Get_Count(); i++)
	{
		m_Shapes[i].m_Index = -1;
	}

	m_Array_Points.Create(sizeof(char *), 0, TSG_Array_Growth::SG_ARRAY_GROWTH_3);
}

//---------------------------------------------------------
CSG_PointCloud::~CSG_PointCloud(void)
{
	Destroy();
}

bool CSG_PointCloud::Destroy(void)
{
	Del_Points();

	if( m_nFields > 0 )
	{
		for(int i=0; i<m_nFields; i++)
		{
			delete(m_Field_Info[i]);
		}

		SG_Free(m_Field_Info  ); m_Field_Info   = NULL;
		SG_Free(m_Field_Offset); m_Field_Offset = NULL;

		m_nFields = 0;

		_On_Construction();
	}

	CSG_Data_Object::Destroy();

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//                         File                          //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::_Load(const CSG_String &File)
{
	SG_UI_Msg_Add(CSG_String::Format("%s %s: %s...", _TL("Loading"), _TL("point cloud"), File.c_str()), true);

	bool bResult = false;

	if( SG_File_Cmp_Extension(File, "sg-pts-z") ) // POINTCLOUD_FILE_FORMAT_Compressed
	{
		CSG_Archive Stream(File, SG_FILE_R);

		CSG_String _File(SG_File_Get_Name(File, false) + ".");

		if( (bResult = Stream.Get_File(_File + "sg-pts")) == false )
		{
			for(size_t i=0; i<Stream.Get_File_Count(); i++)
			{
				if( SG_File_Cmp_Extension(Stream.Get_File_Name(i), "sg-pts") )
				{
					_File = SG_File_Get_Name(Stream.Get_File_Name(i), false) + ".";

					break;
				}
			}

			bResult = Stream.Get_File(_File + "sg-pts");
		}

		if( bResult && _Load(Stream) )
		{
			if( Stream.Get_File(_File + "sg-info") )
			{
				Load_MetaData(Stream);
			}

			if( Stream.Get_File(_File + "sg-prj") )
			{
				Get_Projection().Load(Stream);
			}
		}
	}
	else // if( SG_File_Cmp_Extension(File, "sg-pts"/"spc") ) // POINTCLOUD_FILE_FORMAT_Normal
	{
		CSG_File Stream(File, SG_FILE_R, true);

		if( (bResult = _Load(Stream)) == true )
		{
			Load_MetaData(File);

			Get_Projection().Load(SG_File_Make_Path("", File, "sg-prj"));
		}
	}

	//-----------------------------------------------------
	SG_UI_Process_Set_Ready();

	if( bResult )
	{
		Set_Modified(false);

		Set_File_Name(File, true);

		SG_UI_Msg_Add(_TL("okay"), false, SG_UI_MSG_STYLE_SUCCESS);

		return( true );
	}

	SG_UI_Msg_Add(_TL("failed"), false, SG_UI_MSG_STYLE_FAILURE);

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::Save(const CSG_String &_File, int Format)
{
	if( _File.is_Empty() )
	{
		return( *Get_File_Name(false) ? Save(Get_File_Name(false), Format) : false );
	}

	if( Format == POINTCLOUD_FILE_FORMAT_Undefined )
	{
		Format	= SG_File_Cmp_Extension(_File, "sg-pts-z")
			? POINTCLOUD_FILE_FORMAT_Compressed
			: POINTCLOUD_FILE_FORMAT_Normal;
	}

	bool bResult = false;

	CSG_String File(_File);

	switch( Format )
	{
	//-----------------------------------------------------
	case POINTCLOUD_FILE_FORMAT_Compressed: default:
		{
			SG_File_Set_Extension(File, "sg-pts-z");

			SG_UI_Msg_Add(CSG_String::Format("%s %s: %s...", _TL("Saving"), _TL("point cloud"), File.c_str()), true);

			CSG_Archive Stream(File, SG_FILE_W);

			CSG_String Name(SG_File_Get_Name(File, false) + ".");

			if( Stream.Add_File(Name + "sg-pts") && _Save(Stream) )
			{
				if( Stream.Add_File(Name + "sg-pts-hdr") )
				{
					CSG_MetaData Header = _Create_Header(); Header.Save(Stream);
				}

				if( Stream.Add_File(Name + "sg-info") )
				{
					Save_MetaData(Stream);
				}

				if( Get_Projection().is_Okay() && Stream.Add_File(Name + "sg-prj") )
				{
					Get_Projection().Save(Stream);
				}

				bResult = true;
			}
		}
		break;

	//-----------------------------------------------------
	case POINTCLOUD_FILE_FORMAT_Normal:
		{
			SG_File_Set_Extension(File, "sg-pts");

			SG_UI_Msg_Add(CSG_String::Format("%s %s: %s...", _TL("Saving"), _TL("point cloud"), File.c_str()), true);

			CSG_File Stream(File, SG_FILE_W, true);

			if( _Save(Stream) )
			{
				CSG_MetaData Header = _Create_Header(); Header.Save(SG_File_Make_Path("", File, "sg-pts-hdr"));

				Save_MetaData(File);

				if( Get_Projection().is_Okay() )
				{
					Get_Projection().Save(SG_File_Make_Path("", File, "sg-prj"));
				}

				bResult = true;
			}
		}
		break;
	}

	//-----------------------------------------------------
	SG_UI_Process_Set_Ready();

	if( bResult )
	{
		Set_Modified(false);

		Set_File_Name(File, true);

		SG_UI_Msg_Add(_TL("okay"), false, SG_UI_MSG_STYLE_SUCCESS);

		return( true );
	}

	SG_UI_Msg_Add(_TL("failed"), false, SG_UI_MSG_STYLE_FAILURE);

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::Get_Header_Content(const CSG_String &File, CSG_MetaData &Header)
{
	bool bResult = false;

	if( SG_File_Cmp_Extension(File, "sg-pts-z") ) // POINTCLOUD_FILE_FORMAT_Compressed
	{
		CSG_Archive Stream(File, SG_FILE_R);

		CSG_String _File(SG_File_Get_Name(File, false) + ".");

		if( Stream.Get_File(_File + "sg-pts-hdr") )
		{
			bResult = Header.Load(Stream);
		}
	}
	else // if( SG_File_Cmp_Extension(File, "sg-pts"/"spc") ) // POINTCLOUD_FILE_FORMAT_Normal
	{
		bResult = Header.Load(File, SG_T("sg-pts-hdr"));
	}

	return( bResult );
}

///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::_Load(CSG_File &Stream)
{
	if( !Stream.is_Reading() )
	{
		return( false );
	}

	//-----------------------------------------------------
	char ID[6];

	if( !Stream.Read(ID, 6) || strncmp(ID, PC_FILE_VERSION, 5) != 0 )
	{
		return( false );
	}

	int nPointBytes;

	if( !Stream.Read(&nPointBytes, sizeof(int)) || nPointBytes < (int)(3 * sizeof(float)) )
	{
		return( false );
	}

	int nFields;

	if( !Stream.Read(&nFields, sizeof(int)) || nFields < 3 )
	{
		return( false );
	}

	//-----------------------------------------------------
	Destroy();

	for(int iField=0; iField<nFields; iField++)
	{
		TSG_Data_Type Type; int iBuffer; char Name[1024];

		if( !Stream.Read(&Type   , sizeof(TSG_Data_Type))
		||  !Stream.Read(&iBuffer, sizeof(int)) || !(iBuffer > 0 && iBuffer < 1024)
		||  !Stream.Read(Name    , iBuffer) )
		{
			return( false );
		}

		if( ID[5] == '0' )	// Data Type Definition changed!!!
		{
			switch( Type )
			{
			default: Type = SG_DATATYPE_Undefined; break;
			case  1: Type = SG_DATATYPE_Char     ; break;
			case  2: Type = SG_DATATYPE_Short    ; break;
			case  3: Type = SG_DATATYPE_Int      ; break;
			case  4: Type = SG_DATATYPE_Long     ; break;
			case  5: Type = SG_DATATYPE_Float    ; break;
			case  6: Type = SG_DATATYPE_Double   ; break;
			}
		}

		Name[iBuffer]	= '\0';

		if( !_Add_Field(CSG_String((const char *)Name), Type) )
		{
			return( false );
		}
	}

	//-----------------------------------------------------
	sLong fLength = Stream.Length();

	while( _Inc_Array() && Stream.Read(m_Cursor + 1, nPointBytes) && SG_UI_Process_Set_Progress((double)Stream.Tell(), (double)fLength) )
	{
		// nop
	}

	_Dec_Array();

	return( true );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Save(CSG_File &Stream)
{
	if( !Stream.is_Writing() )
	{
		return( false );
	}

	//-----------------------------------------------------
	int iBuffer, nPointBytes = m_nPointBytes - 1;

	Stream.Write((void *)PC_FILE_VERSION, 6);
	Stream.Write(&nPointBytes, sizeof(int));
	Stream.Write(&m_nFields  , sizeof(int));

	for(int iField=0; iField<m_nFields; iField++)
	{
		Stream.Write(&m_Field_Info[iField]->m_Type, sizeof(TSG_Data_Type));

		iBuffer = (int)m_Field_Info[iField]->m_Name.Length(); if( iBuffer >= 1024 - 1 ) iBuffer	= 1024 - 1;
		Stream.Write(&iBuffer, sizeof(int));
		Stream.Write((void *)m_Field_Info[iField]->m_Name.b_str(), sizeof(char), iBuffer);
	}

	_Shape_Flush();

	for(sLong i=0; i<m_nRecords && SG_UI_Process_Set_Progress(i, m_nRecords); i++)
	{
		Stream.Write(m_Points[i] + 1, nPointBytes);
	}

	return( true );
}

//---------------------------------------------------------
CSG_MetaData CSG_PointCloud::_Create_Header(void) const
{
	CSG_MetaData Header;

	Header.Set_Name("PointCloudHeaderFile");
	Header.Add_Property("Version", "1.0");

	CSG_MetaData *pPoints = Header.Add_Child("Points"    );
	CSG_MetaData *pBBox   = Header.Add_Child("BBox"      );
	CSG_MetaData *pNoData = Header.Add_Child("NoData"    );
	CSG_MetaData *pAttr   = Header.Add_Child("Attributes");

	pPoints->Add_Property("Value", Get_Count());
	pBBox  ->Add_Property("XMin" , Get_Minimum(0));
	pBBox  ->Add_Property("YMin" , Get_Minimum(1));
	pBBox  ->Add_Property("ZMin" , Get_Minimum(2));
	pBBox  ->Add_Property("XMax" , Get_Maximum(0));
	pBBox  ->Add_Property("YMax" , Get_Maximum(1));
	pBBox  ->Add_Property("ZMax" , Get_Maximum(2));
	pNoData->Add_Property("Value", Get_NoData_Value());
	pAttr  ->Add_Property("Count", m_nFields);

	for(int iField=0; iField<m_nFields; iField++)
	{
		CSG_MetaData *pField = pAttr->Add_Child(CSG_String::Format("Field_%d", iField + 1));

		pField->Add_Property("Name", Get_Field_Name(iField));
		pField->Add_Property("Type", gSG_Data_Type_Identifier[Get_Field_Type(iField)]);
	}

	return( Header );
}


///////////////////////////////////////////////////////////
//                                                       //
//						Assign							 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::Assign(CSG_Data_Object *pObject, bool bProgress)
{
	if( pObject->asPointCloud() && CSG_Data_Object::Assign(pObject, bProgress) )
	{
		CSG_PointCloud *pPoints = pObject->asPointCloud();

		Create(pPoints);

		Get_Projection().Create(pPoints->Get_Projection());

		for(sLong i=0; i<pPoints->m_nRecords && (!bProgress || SG_UI_Process_Set_Progress(i, pPoints->m_nRecords)); i++)
		{
			if( _Inc_Array() )
			{
				memcpy(m_Points[i] + 1, pPoints->m_Points[i] + 1, m_nPointBytes - 1l);
			}
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
//						Checks							 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::is_Compatible(CSG_PointCloud *pPointCloud) const
{
	if( m_nFields == pPointCloud->m_nFields )
	{
		for(int iField=0; iField<m_nFields; iField++)
		{
			if( Get_Field_Type(iField) != pPointCloud->Get_Field_Type(iField) )
			{
				return( false );
			}
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
//						Fields							 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::Add_Field(const CSG_String &Name, TSG_Data_Type Type, int Field)
{
	if( m_nFields == 0 ) { _Add_Field(SG_T("X"), m_bXYZPrecDbl ? SG_DATATYPE_Double : SG_DATATYPE_Float); }
	if( m_nFields == 1 ) { _Add_Field(SG_T("Y"), m_bXYZPrecDbl ? SG_DATATYPE_Double : SG_DATATYPE_Float); }
	if( m_nFields == 2 ) { _Add_Field(SG_T("Z"), m_bXYZPrecDbl ? SG_DATATYPE_Double : SG_DATATYPE_Float); }

	return( m_nFields >= 3 && _Add_Field(Name, Type, Field) );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Add_Field(const SG_Char *Name, TSG_Data_Type Type, int Field)
{
	if( !Name || PC_SIZE_TYPE(Type) <= 0 )
	{
		return( false );
	}

	if( Field < 0 || Field > m_nFields )
	{
		Field = m_nFields;
	}

	if( Field < 3 && m_nFields >= 3 )
	{
		Field = 3; // (x, y, z) always have to stay in the first place
	}

	//-----------------------------------------------------
	int nFieldBytes = PC_SIZE_TYPE(Type);

	if( m_nFields < 1 )
	{
		m_nPointBytes = 1;
	}

	m_nFields++; m_nPointBytes += nFieldBytes;

	//-----------------------------------------------------
	m_Field_Offset = (int             *)SG_Realloc(m_Field_Offset, m_nFields * sizeof(int             ));
	m_Field_Info   = (CSG_Field_Info **)SG_Realloc(m_Field_Info  , m_nFields * sizeof(CSG_Field_Info *));

	for(int i=m_nFields-1; i>Field; i--)
	{
		m_Field_Info[i] = m_Field_Info[i - 1];
	}

	m_Field_Info[Field] = new CSG_Field_Info(Name, Type);

	for(int i=0, Offset=1; i<m_nFields; i++)
	{
		m_Field_Offset[i] = Offset; Offset += PC_SIZE_FIELD(i);
	}

	//-----------------------------------------------------
	int Offset = m_Field_Offset[Field], nMoveBytes = Field < m_nFields - 1 ? m_nPointBytes - m_Field_Offset[Field + 1] : 0;

	#pragma omp parallel for
	for(sLong i=0; i<m_nRecords; i++)
	{
		m_Points[i] = (char *)SG_Realloc(m_Points[i], m_nPointBytes * sizeof(char));

		if( nMoveBytes > 0 )
		{
			memmove(m_Points[i] + Offset + nFieldBytes, m_Points[i] + Offset, nMoveBytes);
		}

		memset(m_Points[i] + Offset, 0, nFieldBytes);
	}

	//-----------------------------------------------------
	m_Shapes.Add_Field(Name, Type, Field);

	Set_Modified();

	return( true );
}

//---------------------------------------------------------
bool CSG_PointCloud::Del_Field(int Field)
{
	if( Field < 3 || Field >= m_nFields )
	{
		return( false );
	}

	//-----------------------------------------------------
	int nFieldBytes = PC_SIZE_FIELD(Field);

	m_nFields--; m_nPointBytes -= nFieldBytes;

	//-----------------------------------------------------
	int Offset = m_Field_Offset[Field], nMoveBytes = Field < m_nFields ? (m_nPointBytes + nFieldBytes) - m_Field_Offset[Field + 1] : 0;

	#pragma omp parallel for
	for(sLong i=0; i<m_nRecords; i++)
	{
		if( nMoveBytes > 0 )
		{
			memmove(m_Points[i] + Offset, m_Points[i] + Offset + nFieldBytes, nMoveBytes);
		}

		m_Points[i] = (char *)SG_Realloc(m_Points[i], m_nPointBytes * sizeof(char));
	}

	//-----------------------------------------------------
	delete(m_Field_Info[Field]);

	for(int i=Field, Offset=m_Field_Offset[Field]; i<m_nFields; i++)
	{
		m_Field_Info  [i] = m_Field_Info[i + 1];
		m_Field_Offset[i] = Offset; Offset += PC_SIZE_FIELD(i);
	}

	m_Field_Info   = (CSG_Field_Info **)SG_Realloc(m_Field_Info  , m_nFields * sizeof(CSG_Field_Info *));
	m_Field_Offset = (int             *)SG_Realloc(m_Field_Offset, m_nFields * sizeof(int             ));

	//-----------------------------------------------------
	m_Shapes.Del_Field(Field);

	Set_Modified();

	return( true );
}

//---------------------------------------------------------
bool CSG_PointCloud::Mov_Field(int Field, int Position)
{
	if( Position < 0 )
	{
		Position = 0;
	}
	else if( Position >= m_nFields - 1 )
	{
		Position = m_nFields - 1;
	}

	if( Field < 3 || Field >= m_nFields || Field == Position )
	{
		return( false );
	}

	//-----------------------------------------------------
	if( Position > Field )
	{
		Position++;
	}

	if( !Add_Field(m_Field_Info[Field]->m_Name, m_Field_Info[Field]->m_Type, Position) )
	{
		return( false );
	}

	if( Position < Field )
	{
		Field++;
	}

	size_t Size = PC_SIZE_FIELD(Field);

	#pragma omp parallel for
	for(sLong i=0; i<m_nRecords; i++)
	{
		memcpy(m_Points[i] + m_Field_Offset[Position], m_Points[i] + m_Field_Offset[Field], Size);
	}

	if( !Del_Field(Field) )
	{
		return( false );
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
* Returns the maximum number of bytes reserved for data type
* string or the number of bytes used for all other data types.
*/
//---------------------------------------------------------
int CSG_PointCloud::Get_Field_Length(int Field, int Encoding) const
{
	size_t Length = 0;

	if( Field >= 0 && Field < m_nFields )
	{
		Length = (Field + 1 >= m_nFields ? m_nPointBytes : m_Field_Offset[Field + 1]) - m_Field_Offset[Field];
	}

	return( (int)Length );
}

//---------------------------------------------------------
bool CSG_PointCloud::Set_Field_Type(int iField, TSG_Data_Type Type)
{
	if( iField < 3 || iField >= m_nFields )
	{
		return( false );
	}

	if( Type == m_Field_Info[iField]->m_Type )
	{
		return( true );
	}

	//-----------------------------------------------------
	Add_Field(Get_Field_Name(iField), Type, iField);

	#pragma omp parallel for
	for(sLong i=0; i<m_nRecords; i++)
	{
		Set_Value(i, iField, Get_Value(i, iField + 1));
	}

	Del_Field(iField + 1);

	//-----------------------------------------------------
	m_Shapes.Set_Field_Type(iField, Type);

	Set_Modified();

	return( true );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Set_Field_Value(char *pPoint, int Field, double Value)
{
	if( pPoint && Field >= 0 && Field < m_nFields )
	{
		pPoint = ((char *)pPoint) + m_Field_Offset[Field];

		switch( m_Field_Info[Field]->m_Type )
		{
		case SG_DATATYPE_Byte  : *((BYTE   *)pPoint) = (BYTE  )Value ; break;
		case SG_DATATYPE_Char  : *((char   *)pPoint) = (char  )Value ; break;
		case SG_DATATYPE_Word  : *((WORD   *)pPoint) = (WORD  )Value ; break;
		case SG_DATATYPE_Short : *((short  *)pPoint) = (short )Value ; break;
		case SG_DATATYPE_DWord : *((DWORD  *)pPoint) = (DWORD )Value ; break;
		case SG_DATATYPE_Int   : *((int    *)pPoint) = (int   )Value ; break;
		case SG_DATATYPE_Long  : *((sLong  *)pPoint) = (sLong )Value ; break;
		case SG_DATATYPE_ULong : *((uLong  *)pPoint) = (uLong )Value ; break;
		case SG_DATATYPE_Float : *((float  *)pPoint) = (float )Value ; break;
		case SG_DATATYPE_Double: *((double *)pPoint) = (double)Value ; break;
		case SG_DATATYPE_Color : *((DWORD  *)pPoint) = (DWORD )Value ; break;
		case SG_DATATYPE_String: sprintf(    pPoint, "%f"    , Value); break;
		default                :                                       break;
		}

		m_Field_Info[Field]->Reset_Statistics();

		Set_Modified();

		if( Field < 3 )
		{
			Set_Update_Flag(); // extent might have changed
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
double CSG_PointCloud::_Get_Field_Value(char *pPoint, int Field) const
{
	if( pPoint && Field >= 0 && Field < m_nFields )
	{
		pPoint += m_Field_Offset[Field];

		switch( m_Field_Info[Field]->m_Type )
		{
		case SG_DATATYPE_Byte  : return( (double)*((BYTE   *)pPoint) );
		case SG_DATATYPE_Char  : return( (double)*((char   *)pPoint) );
		case SG_DATATYPE_Word  : return( (double)*((WORD   *)pPoint) );
		case SG_DATATYPE_Short : return( (double)*((short  *)pPoint) );
		case SG_DATATYPE_DWord : return( (double)*((DWORD  *)pPoint) );
		case SG_DATATYPE_Int   : return( (double)*((int    *)pPoint) );
		case SG_DATATYPE_Long  : return( (double)*((sLong  *)pPoint) );
		case SG_DATATYPE_ULong : return( (double)*((uLong  *)pPoint) );
		case SG_DATATYPE_Float : return( (double)*((float  *)pPoint) );
		case SG_DATATYPE_Double: return( (double)*((double *)pPoint) );
		case SG_DATATYPE_Color : return( (double)*((DWORD  *)pPoint) );
		case SG_DATATYPE_String: return( (double)atof(       pPoint) );
		default                : break;
		}
	}

	return( 0. );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Set_Field_Value(char *pPoint, int Field, const SG_Char *Value)
{
	if( pPoint && Field >= 0 && Field < m_nFields && Value )
	{
		CSG_String s(Value);

		switch( m_Field_Info[Field]->m_Type )
		{
		default: { double d; return( s.asDouble(d) && _Set_Field_Value(pPoint, Field, d) ); }

		case SG_DATATYPE_Date  :
		case SG_DATATYPE_String:
			pPoint	+= m_Field_Offset[Field];
			memset(pPoint, 0, PC_SIZE_STRING);
			memcpy(pPoint, s.b_str(), s.Length() > PC_SIZE_STRING ? PC_SIZE_STRING : s.Length());
			break;
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Get_Field_Value(char *pPoint, int Field, CSG_String &Value)	const
{
	if( pPoint && Field >= 0 && Field < m_nFields )
	{
		switch( m_Field_Info[Field]->m_Type )
		{
		default: Value.Printf("%f", _Get_Field_Value(pPoint, Field)); break;

		case SG_DATATYPE_Date  :
		case SG_DATATYPE_String:
			{
				char s[PC_SIZE_STRING + 1];

				memcpy(s, pPoint + m_Field_Offset[Field], PC_SIZE_STRING);

				s[PC_SIZE_STRING] = '\0';

				Value = s;
			}
			break;
		}

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
TSG_Point_3D CSG_PointCloud::Get_Point(void)	const
{
	TSG_Point_3D	p;

	if( m_Cursor )
	{
		p.x = _Get_Field_Value(m_Cursor, 0);
		p.y = _Get_Field_Value(m_Cursor, 1);
		p.z = _Get_Field_Value(m_Cursor, 2);
	}
	else
	{
		p.x = p.y = p.z = 0.;
	}

	return( p );
}

//---------------------------------------------------------
TSG_Point_3D CSG_PointCloud::Get_Point(sLong Index)	const
{
	TSG_Point_3D p;

	if( Index >= 0 && Index < m_nRecords )
	{
		char *pPoint = m_Points[Index];

		p.x = _Get_Field_Value(pPoint, 0);
		p.y = _Get_Field_Value(pPoint, 1);
		p.z = _Get_Field_Value(pPoint, 2);
	}
	else
	{
		p.x = p.y = p.z = 0.;
	}

	return( p );
}

//---------------------------------------------------------
bool CSG_PointCloud::Set_Point(const TSG_Point_3D &Point)
{
	return( _Set_Field_Value(m_Cursor, 0, Point.x)
		&&  _Set_Field_Value(m_Cursor, 1, Point.y)
		&&  _Set_Field_Value(m_Cursor, 2, Point.z)
	);
}

//---------------------------------------------------------
bool CSG_PointCloud::Set_Point(sLong Index, const TSG_Point_3D &Point)
{
	if( Index >= 0 && Index < m_nRecords )
	{
		return( _Set_Field_Value(m_Points[Index], 0, Point.x)
			&&  _Set_Field_Value(m_Points[Index], 1, Point.y)
			&&  _Set_Field_Value(m_Points[Index], 2, Point.z)
		);
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::Add_Point(double x, double y, double z)
{
	if( _Inc_Array() )
	{
		_Set_Field_Value(m_Cursor, 0, x);
		_Set_Field_Value(m_Cursor, 1, y);
		_Set_Field_Value(m_Cursor, 2, z);

		Set_Modified();
		Set_Update_Flag();
		_Stats_Invalidate();

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::Add_Point(const CSG_Point_3D &Point)
{
	return( Add_Point(Point.x, Point.y, Point.z) );
}

//---------------------------------------------------------
bool CSG_PointCloud::Del_Point(sLong Index)
{
	if( Index >= 0 && Index < m_nRecords )
	{
		if( is_Selected(Index) )
		{
			Select(Index, true);
		}

		m_Cursor = m_Points[Index];

		for(sLong i=Index, j=Index+1; j<m_nRecords; i++, j++)
		{
			m_Points[i]	= m_Points[j];
		}

		m_Points[m_nRecords - 1] = m_Cursor;

		m_Cursor = NULL;

		_Dec_Array();

		Set_Modified();
		Set_Update_Flag();
		_Stats_Invalidate();

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::Del_Points(void)
{
	for(sLong i=0; i<m_nRecords; i++)
	{
		SG_Free(m_Points[i]);
	}

	m_Array_Points.Set_Array(0);

	m_nRecords = 0;
	m_Points   = NULL;
	m_Cursor   = NULL;

	m_Selection.Set_Array(0);

	Set_Modified();
	Set_Update_Flag();
	_Stats_Invalidate();

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::_Inc_Array(void)
{
	if( m_nFields > 0 && m_Array_Points.Set_Array(m_nRecords + 1, (void **)&m_Points) )
	{
		m_Points[m_nRecords++]	= m_Cursor	= (char *)SG_Calloc(m_nPointBytes, sizeof(char));

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Dec_Array(void)
{
	if( m_nRecords > 0 )
	{
		m_nRecords	--;

		m_Cursor	= NULL;

		SG_Free(m_Points[m_nRecords]);

		m_Array_Points.Set_Array(m_nRecords, (void **)&m_Points);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//						Statistics						 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::On_Update(void)
{
	if( m_nFields >= 2 )
	{
		_Shape_Flush();

		for(int iField=0; iField<m_nFields; iField++)
		{
			m_Field_Info[iField]->Reset_Statistics();
		}

		m_Extent.Create(Get_Minimum(0), Get_Minimum(1), Get_Maximum(0), Get_Maximum(1));

		m_ZMin = Get_Minimum(2);
		m_ZMax = Get_Maximum(2);
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_PointCloud::_Stats_Update(int Field) const
{
	if( Field >= 3 ) // attributes, else => always get full statistics for x, y, z coordinates!
	{
		return( CSG_Table::_Stats_Update(Field) );
	}

	if( Field < 0 || m_nRecords < 1 )
	{
		return( false );
	}

	CSG_Simple_Statistics &Statistics = m_Field_Info[Field]->m_Statistics;

	if( Statistics.is_Evaluated() )
	{
		return( true );
	}

	char **pPoint = m_Points;

	for(sLong i=0; i<m_nRecords; i++, pPoint++)
	{
		Statistics += _Get_Field_Value(*pPoint, Field);
	}

	return( Statistics.Evaluate() ); // evaluate! prevent values to be added more than once!
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::On_Reload(void)
{
	return( Create(Get_File_Name(false)) );
}

//---------------------------------------------------------
bool CSG_PointCloud::On_Delete(void)
{
	return( SG_File_Delete(Get_File_Name(false)) );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Shape * CSG_PointCloud::_Shape_Get(sLong Index)
{
	SG_UI_Progress_Lock(true);

	CSG_Shape *pShape = m_Shapes.Get_Shape(SG_OMP_Get_Thread_Num());

	if( pShape->is_Modified() && pShape->m_Index >= 0 && pShape->m_Index < m_nRecords )
	{
		char *Point = m_Points[pShape->m_Index];

		for(int iField=0; iField<m_nFields; iField++)
		{
			switch( Get_Field_Type(iField) )
			{
			default                : _Set_Field_Value(Point, iField, pShape->asDouble(iField)); break;
			case SG_DATATYPE_Date  :
			case SG_DATATYPE_String: _Set_Field_Value(Point, iField, pShape->asString(iField)); break;
			}
		}

		Set_Value(0, pShape->Get_Point().x);
		Set_Value(1, pShape->Get_Point().y);
		Set_Value(2, pShape->Get_Z    ()  );
	}

	if( Index >= 0 && Index < m_nRecords )
	{
		char *Point = m_Points[Index];

		for(int iField=0; iField<m_nFields; iField++)
		{
			switch( Get_Field_Type(iField) )
			{
			default: pShape->Set_Value(iField, _Get_Field_Value(Point, iField)); break;

			case SG_DATATYPE_Date  :
			case SG_DATATYPE_String: {
				CSG_String s; _Get_Field_Value(Point, iField, s); pShape->Set_Value(iField, s);
				break; }
			}
		}

		pShape->Set_Point(Get_Point(Index));
		pShape->Set_Selected(is_Selected(Index));
		pShape->Set_Modified(false);

		pShape->m_Index = Index;
	}
	else
	{
		pShape->m_Index = -1;
	}

	SG_UI_Progress_Lock(false);

	return( pShape->m_Index >= 0 ? pShape : NULL );
}

//---------------------------------------------------------
void CSG_PointCloud::_Shape_Flush(void)
{
	for(sLong i=0; i<m_Shapes.Get_Count(); i++)
	{
		_Shape_Get(m_Shapes[i].m_Index); m_Shapes[i].m_Index = -1;
	}
}

//---------------------------------------------------------
CSG_Table_Record * CSG_PointCloud::Get_Record(sLong Index)	const
{
	return( ((CSG_PointCloud *)this)->_Shape_Get(Index) );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
* Returns the index of the nearest point cloud point
* within search distance (epsilon) to given point location.
*/
//---------------------------------------------------------
sLong CSG_PointCloud::Get_Point(const CSG_Point &Point, double Epsilon)
{
	sLong Index = -1; CSG_Rect r(Point.x - Epsilon, Point.y - Epsilon, Point.x + Epsilon, Point.y + Epsilon);

	if( r.Intersects(Get_Extent()) != INTERSECTION_None )
	{
		double Distance = -1.;

		for(sLong i=0; i<m_nRecords; i++)
		{
			double x = Get_X(i), y = Get_Y(i);

			if( r.Contains(x, y) )
			{
				if( Index < 0 || Distance > SG_Get_Distance(Point.x, Point.y, x, y) )
				{
					Index = i; Distance = SG_Get_Distance(Point.x, Point.y, x, y);
				}
			}
		}
	}

	return( Index );
}

//---------------------------------------------------------
CSG_Shape * CSG_PointCloud::Get_Shape(const CSG_Point &Point, double Epsilon)
{
	sLong Index = Get_Point(Point, Epsilon);

	return( Index < 0 ? NULL : CSG_Shapes::Get_Shape(Index) );
}

//---------------------------------------------------------
CSG_Table_Record * CSG_PointCloud::Ins_Record(sLong Index, CSG_Table_Record *pCopy)
{
	return( NULL );
}

//---------------------------------------------------------
CSG_Table_Record * CSG_PointCloud::Add_Record(CSG_Table_Record *pCopy)
{
	return( Add_Shape(pCopy, SHAPE_COPY) );
}

//---------------------------------------------------------
CSG_Shape * CSG_PointCloud::Add_Shape(CSG_Table_Record *pCopy, TSG_ADD_Shape_Copy_Mode mCopy)
{
	Add_Point(0., 0., 0.);

	if( pCopy && (mCopy == SHAPE_COPY_ATTR || mCopy == SHAPE_COPY) )
	{
		for(int iField=0; iField<m_nFields && iField<pCopy->Get_Table()->Get_Field_Count(); iField++)
		{
			if( Get_Field_Type(iField) == pCopy->Get_Table()->Get_Field_Type(iField) )
			{
				if( SG_Data_Type_is_Numeric(Get_Field_Type(iField)) )
				{
					Set_Value(iField, pCopy->asDouble(iField));
				}
				else
				{
					Set_Value(iField, pCopy->asString(iField));
				}
			}
		}
	}

	return( _Shape_Get(m_nRecords - 1) );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::Select(sLong Index, bool bInvert)
{
	if( !bInvert && Get_Selection_Count() > 0 )
	{
		for(sLong i=0; i<Get_Selection_Count(); i++)
		{
			m_Points[Get_Selection_Index(i)][0]	&= ~SG_TABLE_REC_FLAG_Selected;
		}

		m_Selection.Destroy();
	}

	if( Set_Cursor(Index) )
	{
		if( (m_Cursor[0] & SG_TABLE_REC_FLAG_Selected) == 0 )	// select
		{
			if( _Add_Selection(Index) )
			{
				m_Cursor[0]	|= SG_TABLE_REC_FLAG_Selected;

				return( true );
			}
		}
		else													// deselect
		{
			if( _Del_Selection(Index) )
			{
				m_Cursor[0]	&= ~SG_TABLE_REC_FLAG_Selected;

				return( true );
			}
		}
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PointCloud::Select(CSG_Table_Record *pShape, bool bInvert)
{
    return( CSG_Table::Select(pShape, bInvert) );
}

//---------------------------------------------------------
bool CSG_PointCloud::Select(const TSG_Rect &Extent, bool bInvert)
{
	if( !bInvert )	// clear selection
	{
		Select(-1, false);
	}

	if( Get_Extent().Intersects(Extent) != INTERSECTION_None )
	{
		for(sLong i=0; i<m_nRecords; i++)
		{
			Set_Cursor(i);

			if(	Extent.xMin <= Get_X() && Get_X() <= Extent.xMax
			&&	Extent.yMin <= Get_Y() && Get_Y() <= Extent.yMax )
			{
				Select(i, true);
			}
		}
	}

	return( Get_Selection_Count() > 0 );
}

//---------------------------------------------------------
bool CSG_PointCloud::Select(const TSG_Point &Point, bool bInvert)
{
	return( Select(CSG_Rect(Point.x, Point.y, Point.x, Point.y), bInvert) );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::is_Selected(sLong Index)	const
{
	return( Index >= 0 && Index < m_nRecords && (m_Points[Index][0] & SG_TABLE_REC_FLAG_Selected) != 0 );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Shape * CSG_PointCloud::Get_Selection(sLong Index)
{
	return( Index < Get_Selection_Count() ? _Shape_Get(Get_Selection_Index(Index)) : NULL );
}

//---------------------------------------------------------
const CSG_Rect & CSG_PointCloud::Get_Selection_Extent(void)
{
	if( Get_Selection_Count() > 0 && Set_Cursor((int)Get_Selection_Index(0)) )
	{
		TSG_Rect r;

		r.xMin = r.xMax = Get_X();
		r.yMin = r.yMax = Get_Y();

		for(sLong i=1; i<Get_Selection_Count(); i++)
		{
			if( Set_Cursor(Get_Selection_Index(i)) )
			{
				if( Get_X() < r.xMin ) { r.xMin = Get_X(); } else if( Get_X() > r.xMax ) { r.xMax = Get_X(); }
				if( Get_Y() < r.yMin ) { r.yMin = Get_Y(); } else if( Get_Y() > r.yMax ) { r.yMax = Get_Y(); }
			}
		}

		m_Extent_Selected.Assign(r);
	}
	else
	{
		m_Extent_Selected.Assign(0., 0., 0., 0.);
	}

	return( m_Extent_Selected );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
sLong CSG_PointCloud::Del_Selection(void)
{
	sLong n = 0;

	if( Get_Selection_Count() > 0 )
	{
		m_Selection.Set_Array(0);

		m_Cursor = NULL;

		for(sLong i=0; i<m_nRecords; i++)
		{
			if( (m_Points[i][0] & SG_TABLE_REC_FLAG_Selected) != 0 )
			{
				SG_Free(m_Points[i]);
			}
			else
			{
				if( n < i )
				{
					m_Points[n] = m_Points[i];
				}

				n++;
			}
		}

		m_Array_Points.Set_Array(m_nRecords = n, (void **)&m_Points);

		Set_Modified();
		Set_Update_Flag();
		_Stats_Invalidate();
	}

	return( n );
}

//---------------------------------------------------------
sLong CSG_PointCloud::Inv_Selection(void)
{
	if( m_Selection.Set_Array(m_nRecords - Get_Selection_Count()) )
	{
		char **pPoint = m_Points;

		for(sLong i=0, n=0; i<m_nRecords && n<Get_Selection_Count(); i++, pPoint++)
		{
			if( ((*pPoint)[0] & SG_TABLE_REC_FLAG_Selected) != 0 )
			{
				(*pPoint)[0] &= ~SG_TABLE_REC_FLAG_Selected;
			}
			else
			{
				(*pPoint)[0] |= SG_TABLE_REC_FLAG_Selected;

				_Set_Selection(i, n++);
			}
		}
	}

	return( Get_Selection_Count() );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PointCloud::Sort(const CSG_Index &Index)
{
	m_Points[m_nRecords++]	= (char *)SG_Calloc(m_nPointBytes, sizeof(char));

	if( Get_Count() > 0 && Get_Count() == Index.Get_Count() )
	{
		char **Points = (char **)SG_Malloc(m_nBuffer * sizeof(char *)); memcpy(Points, m_Points, Get_Count());

		for(sLong i=0; i<Get_Count(); i++)
		{
			m_Points[i] = Points[Index[i]];
		}

		SG_Free(Points);

		Del_Index(); m_Cursor = NULL;

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
