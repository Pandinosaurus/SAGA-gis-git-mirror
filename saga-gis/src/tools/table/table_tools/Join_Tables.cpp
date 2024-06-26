
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                     Tool Library                      //
//                     Table_Tools                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   Join_Tables.cpp                     //
//                                                       //
//                 Copyright (C) 2005 by                 //
//              Olaf Conrad  & Victor Olaya              //
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
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "Join_Tables.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CJoin_Table::CJoin_Table(void)
{
	Set_Name		(_TL("Join Attributes from a Table"));

	Set_Author		("V.Olaya (c) 2005, O.Conrad (c) 2011");

	Set_Description	(_TW(
		"Joins two tables using key attributes."
	));

	//-----------------------------------------------------
	Parameters.Add_Table("",
		"TABLE_A"	, _TL("Input Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field("TABLE_A",
		"ID_A"		, _TL("Input Join Field"),
		_TL("")
	);

	Parameters.Add_Table("",
		"TABLE_B"	, _TL("Join Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field("TABLE_B",
		"ID_B"		, _TL("Join Table Field"),
		_TL("")
	);

	Parameters.Add_Bool("TABLE_B",
		"FIELDS_ALL", _TL("Add All Fields"),
		_TL(""),
		true
	);

	Parameters.Add_Table_Fields("TABLE_B",
		"FIELDS"	, _TL("Fields"),
		_TL("")
	);

	Parameters.Add_Bool("",
		"KEEP_ALL"	, _TL("Keep All"),
		_TL(""),
		true
	);

	Parameters.Add_Bool("",
		"CMP_CASE"	, _TL("Case Sensitive String Comparison"),
		_TL(""),
		true
	);

	Parameters.Add_Table("",
		"UNJOINED"	, _TL("Unjoined Records"),
		_TL("Collect unjoined records from join table."),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Table ("", "RESULT_TABLE" , _TL("Result"), _TL(""), PARAMETER_OUTPUT_OPTIONAL);
	Parameters.Add_Shapes("", "RESULT_SHAPES", _TL("Result"), _TL(""), PARAMETER_OUTPUT_OPTIONAL);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CJoin_Table::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	pParameter->Cmp_Identifier("TABLE_A") )
	{
		if( pParameter->asDataObject() )
		{
			pParameters->Set_Enabled("RESULT_TABLE" , pParameter->asDataObject()->asShapes() == NULL);
			pParameters->Set_Enabled("RESULT_SHAPES", pParameter->asDataObject()->asShapes() != NULL);
		}
		else
		{
			pParameters->Set_Enabled("RESULT_TABLE" , false);
			pParameters->Set_Enabled("RESULT_SHAPES", false);
		}
	}

	if(	pParameter->Cmp_Identifier("FIELDS_ALL") )
	{
		pParameters->Set_Enabled("FIELDS", pParameter->asBool() == false);
	}

	return( CSG_Tool::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CJoin_Table::On_Execute(void)
{
	CSG_Table *pTable_A = Parameters("TABLE_A")->asTable(); int Key_A = Parameters("ID_A")->asInt();

	if(	pTable_A->Get_Field_Count() < 1 || pTable_A->Get_Count() < 1 )
	{
		Error_Fmt("%s [%s]", _TL("Table contains no records or fields"), pTable_A->Get_Name());

		return( false );
	}

	CSG_Table *pTable_B = Parameters("TABLE_B")->asTable(); int Key_B = Parameters("ID_B")->asInt();

	if(	pTable_B->Get_Field_Count() < 2 || pTable_B->Get_Count() < 1 ) // id/key field of join is redundant and should be ignored
	{
		Error_Fmt("%s [%s]", _TL("Join table contains no records or less than two fields"), pTable_B->Get_Name());

		return( false );
	}

	//-----------------------------------------------------
	if( pTable_A->asShapes() == NULL && Parameters("RESULT_TABLE")->asTable () )
	{
		pTable_A = Parameters("RESULT_TABLE")->asTable();

		((CSG_Table  *)pTable_A)->Create(*Parameters("TABLE_A")->asTable ());

		pTable_A->Fmt_Name("%s [%s]", pTable_A->Get_Name(), pTable_B->Get_Name());
	}

	if( pTable_A->asShapes() != NULL && Parameters("RESULT_SHAPES")->asShapes() )
	{
		pTable_A = Parameters("RESULT_SHAPES")->asShapes();

		((CSG_Shapes *)pTable_A)->Create(*Parameters("TABLE_A")->asShapes());

		pTable_A->Fmt_Name("%s [%s]", pTable_A->Get_Name(), pTable_B->Get_Name());
	}

	//-----------------------------------------------------
	CSG_Array_Int Joins; int Offset = pTable_A->Get_Field_Count();

	if( Parameters("FIELDS_ALL")->asBool() )
	{
		for(int i=0; i<pTable_B->Get_Field_Count(); i++)
		{
			if( i != Key_B )
			{
				pTable_A->Add_Field(pTable_B->Get_Field_Name(i), pTable_B->Get_Field_Type(i));

				Joins += i;
			}
		}
	}
	else
	{
		CSG_Parameter_Table_Fields *pFields = Parameters("FIELDS")->asTableFields();

		for(int j=0; j<pFields->Get_Count(); j++)
		{
			int i = pFields->Get_Index(j);

			pTable_A->Add_Field(pTable_B->Get_Field_Name(i), pTable_B->Get_Field_Type(i));

			Joins += i;
		}
	}

	if( Joins.Get_Size() < 1 )
	{
		Error_Set(_TL("no fields to add"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Table *pUnjoined = Parameters("UNJOINED")->asTable(); int nUnjoined[2]; nUnjoined[0] = nUnjoined[1] = 0;

	if( pUnjoined )
	{
		pUnjoined->Create(pTable_B);
		pUnjoined->Fmt_Name("%s [%s]", pTable_B->Get_Name(), _TL("Unjoined Records"));
	}

	//-----------------------------------------------------
	m_bCmpNoCase	= Parameters("CMP_CASE")->asBool() == false;

	m_bCmpNumeric	=  SG_Data_Type_is_Numeric(pTable_A->Get_Field_Type(Key_A))
					|| SG_Data_Type_is_Numeric(pTable_B->Get_Field_Type(Key_B));

	CSG_Table Delete; if( !Parameters("KEEP_ALL")->asBool() ) Delete.Add_Field("ID", SG_DATATYPE_Int);

	CSG_Index Index_A; pTable_A->Set_Index(Index_A, Key_A);
	CSG_Index Index_B; pTable_B->Set_Index(Index_B, Key_B);

	CSG_Table_Record *pRecord_B = pTable_B->Get_Record(Index_B[0]);

	for(sLong a=0, b=0, nJoined=0, Cmp; a<pTable_A->Get_Count() && Set_Progress(a, pTable_A->Get_Count()); a++)
	{
		CSG_Table_Record *pRecord_A = pTable_A->Get_Record(Index_A[a]);

		while( (Cmp = Cmp_Keys(pRecord_A->Get_Value(Key_A), pRecord_B ? pRecord_B->Get_Value(Key_B) : NULL)) < 0 )
		{
			if( nJoined < 1 )
			{
				nUnjoined[1]++;

				if( pUnjoined )
				{
					pUnjoined->Add_Record(pRecord_B);
				}
			}

			pRecord_B = pTable_B->Get_Record(Index_B[++b]); nJoined = 0;
		}

		if( Cmp == 0 )
		{
			nJoined++;

			for(int i=0; i<(int)Joins.Get_Size(); i++)
			{
				*pRecord_A->Get_Value(Offset + i) = *pRecord_B->Get_Value(Joins[i]);
			}
		}
		else
		{
			nUnjoined[0]++;

			if( Delete.Get_Field_Count() )
			{
				Delete.Add_Record()->Set_Value(0, Index_A[a]);
			}
			else for(int i=0; i<(int)Joins.Get_Size(); i++)
			{
				pRecord_A->Set_NoData(Offset + i);
			}
		}
	}

	//-----------------------------------------------------
	if( nUnjoined[0] >= pTable_A->Get_Count() )
	{
		Message_Fmt("\n%s", _TL("no record found a join"));
	}
	else
	{
		if( nUnjoined[0] )
		{
			Message_Fmt("\n%s: %d", _TL("number of unjoined input records"), nUnjoined[0]);
		}
		else
		{
			Message_Fmt("\n%s", _TL("all input records found a join"));
		}

		if( nUnjoined[1] )
		{
			Message_Fmt("\n%s: %d", _TL("number of unjoined join table records"), nUnjoined[1]);
		}
		else
		{
			Message_Fmt("\n%s", _TL("all join table records found at least one join"));
		}
	}

	//-----------------------------------------------------
	if( Delete.Get_Count() > 0 )
	{
		Delete.Set_Index(0, TABLE_INDEX_Descending);

		for(sLong i=0; i<Delete.Get_Count(); i++)
		{
			pTable_A->Del_Record(Delete[i].asInt(0));
		}
	}

	if( pTable_A == Parameters("TABLE_A")->asTable() )
	{
		DataObject_Update(pTable_A);
	}

	return( pTable_A->Get_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline int CJoin_Table::Cmp_Keys(CSG_Table_Value *pA, CSG_Table_Value *pB)
{
	if( pB == NULL )
	{
		return( 1 );
	}

	if( m_bCmpNumeric )
	{
		double	d	= pB->asDouble() - pA->asDouble();

		return( d < 0. ? -1 : d > 0. ? 1 : 0 );
	}

	CSG_String	Key(pB->asString());

	return( m_bCmpNoCase ? Key.CmpNoCase(pA->asString()) : Key.Cmp(pA->asString()) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CTable_Append_Cols::CTable_Append_Cols(void)
{
	Set_Name		(_TL("Append Fields from another Table"));

	Set_Author		("O.Conrad (c) 2012");

	Set_Description	(_TW(
		""
	));

	Parameters.Add_Table("",
		"INPUT"	, _TL("Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table("",
		"APPEND", _TL("Append Rows from ..."),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table("",
		"OUTPUT", _TL("Result"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CTable_Append_Cols::On_Execute(void)
{
	CSG_Table *pTable  = Parameters("INPUT" )->asTable();

	if( pTable->Get_Count() <= 0 )
	{
		Error_Set(_TL("no records in data set"));

		return( false );
	}

	CSG_Table *pAppend = Parameters("APPEND")->asTable();

	if( pAppend->Get_Count() <= 0 )
	{
		Error_Set(_TL("no records to append"));

		return( false );
	}

	//-----------------------------------------------------
	if( Parameters("OUTPUT")->asTable() && Parameters("OUTPUT")->asTable() != pTable )
	{
		CSG_Table *pOutput = Parameters("OUTPUT")->asTable();
		pOutput->Create  (*pTable);
		pOutput->Set_Name( pTable->Get_Name());
		pTable	= pOutput;
	}

	//-----------------------------------------------------
	sLong	nRecords = pTable->Get_Count() < pAppend->Get_Count() ? pTable->Get_Count() : pAppend->Get_Count();
	int		offField = pTable->Get_Field_Count();

	for(int iField=0; iField<pAppend->Get_Field_Count(); iField++)
	{
		pTable->Add_Field(pAppend->Get_Field_Name(iField), pAppend->Get_Field_Type(iField));
	}

	//-----------------------------------------------------
	for(sLong iRecord=0; iRecord<nRecords && Set_Progress(iRecord, nRecords); iRecord++)
	{
		CSG_Table_Record &Record = *pTable ->Get_Record(iRecord);
		CSG_Table_Record &Append = *pAppend->Get_Record(iRecord);

		for(int iField=0; iField<pAppend->Get_Field_Count(); iField++)
		{
			*Record.Get_Value(offField + iField) = *Append.Get_Value(iField);
		}
	}

	//-----------------------------------------------------
	if( pTable == Parameters("INPUT")->asTable() )
	{
		DataObject_Update(pTable);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
