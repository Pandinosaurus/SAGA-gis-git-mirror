
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
//                        tool.h                         //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
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
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef HEADER_INCLUDED__SAGA_API__tool_H
#define HEADER_INCLUDED__SAGA_API__tool_H


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/** \file tool.h
* The definition of the base classes for any SAGA tool.
* @see CSG_Tool
* @see CSG_Tool_Grid
* @see CSG_Tool_Interactive
* @see CSG_Tool_Library
* @see CSG_Tool_Library_Manager
*/


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "parameters.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
typedef enum
{
	SG_SUMMARY_FMT_FLAT		= 0,
	SG_SUMMARY_FMT_HTML,
	SG_SUMMARY_FMT_XML
}
TSG_Summary_Format;

//---------------------------------------------------------
typedef enum
{
	TOOL_TYPE_Base			= 0,
	TOOL_TYPE_Interactive,
	TOOL_TYPE_Grid,
	TOOL_TYPE_Grid_Interactive,
	TOOL_TYPE_Chain
}
TSG_Tool_Type;

//---------------------------------------------------------
typedef enum
{
	TOOL_ERROR_Unknown		= 0,
	TOOL_ERROR_Calculation
}
TSG_Tool_Error;


///////////////////////////////////////////////////////////
//                                                       //
//						CSG_Tool						 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
  * CSG_Tool is the base class for all executable SAGA tools.
  * @see CSG_Parameters
  * @see CSG_Tool_Interactive
  * @see CSG_Tool_Grid
  * @see CSG_Tool_Grid_Interactive
*/
//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool
{
	friend class CSG_Tool_Interactive_Base;
	friend class CSG_Tool_Library_Interface;
	friend class CSG_Tool_Library_Manager;
	friend class CSG_Tool_Chain;

public:

	CSG_Tool(void);
	virtual ~CSG_Tool(void);

	virtual void				Destroy						(void);

	virtual TSG_Tool_Type		Get_Type					(void)	const	{	return( TOOL_TYPE_Base );	}

	const CSG_String &			Get_ID						(void)	const	{	return( m_ID );	}

	const CSG_String &			Get_Library					(void)	const;
	const CSG_String &			Get_File_Name				(void)	const;	// Returns the file name of the tool's library or, if this is a tool chain, the associated XML file.
	const CSG_String &			Get_Name					(void)	const;
	const CSG_String &			Get_Author					(void)	const;
	const CSG_String &			Get_Version					(void)	const;
	const CSG_String &			Get_Description				(void)	const;
	const CSG_Strings &			Get_References				(void)	const;
	const SG_Char *				Get_Icon					(void)	{	return( NULL );	}
	CSG_String					Get_Summary					(bool bParameters = true, const CSG_String &Menu = "", const CSG_String &Description = "", int Format = SG_SUMMARY_FMT_HTML);

	virtual CSG_String			Get_MenuPath				(void)	{	return( SG_T("") );	}
	virtual CSG_String			Get_MenuPath				(bool bSolved);

	CSG_Parameters *			Get_Parameters				(void)	{	return( &Parameters );	}

	int							Get_Parameters_Count		(void)  const { return( (int)m_pParameters.Get_Size() ); }
	CSG_Parameters *			Get_Parameters				(int i) const { return( i >= 0 && i < Get_Parameters_Count() ? (CSG_Parameters *)m_pParameters[i] : NULL ); }
	CSG_Parameters *			Get_Parameters				(int i)       { return( i >= 0 && i < Get_Parameters_Count() ? (CSG_Parameters *)m_pParameters[i] : NULL ); }
	CSG_Parameters *			Get_Parameters				(const CSG_String &Identifier) const;

	CSG_Parameter *				Get_Parameter				(const CSG_String &ID)	const	{	return( Parameters(ID) );	}
	CSG_Parameter *				Get_Parameter				(const char       *ID)	const	{	return( Parameters(ID) );	}
	CSG_Parameter *				Get_Parameter				(const wchar_t    *ID)	const	{	return( Parameters(ID) );	}
	bool						Set_Parameter				(const CSG_String &ID, CSG_Parameter   *pValue);
	bool						Set_Parameter				(const char       *ID, CSG_Parameter   *pValue);
	bool						Set_Parameter				(const wchar_t    *ID, CSG_Parameter   *pValue);
	bool						Set_Parameter				(const CSG_String &ID, int               Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, int               Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, int               Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const CSG_String &ID, double            Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, double            Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, double            Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const CSG_String &ID, void             *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, void             *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, void             *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const CSG_String &ID, const CSG_String &Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, const CSG_String &Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, const CSG_String &Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const CSG_String &ID, const char       *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, const char       *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, const char       *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const CSG_String &ID, const wchar_t    *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const char       *ID, const wchar_t    *Value, int Type = PARAMETER_TYPE_Undefined);
	bool						Set_Parameter				(const wchar_t    *ID, const wchar_t    *Value, int Type = PARAMETER_TYPE_Undefined);

	bool						Reset						(bool bManager = true);
	bool						Reset_Manager				(void);
	bool						Reset_Grid_System			(void);

	bool						Update_Parameter_States		(void);

	void						Set_Callback				(bool bActive = true);

	bool						Set_Manager					(class CSG_Data_Manager *pManager);
	class CSG_Data_Manager *	Get_Manager					(void)	const;
	class CSG_Data_Manager *	Create_Manager				(void);
	bool						Delete_Manager				(bool bDetachData = false, bool bReset = true);

	bool						Set_Grid_System				(const CSG_Grid_System &System);
	CSG_Grid_System *			Get_Grid_System				(void)	const;

	bool						Settings_Push				(class CSG_Data_Manager *pManager = NULL);
	bool						Settings_Pop				(void);

	virtual bool				do_Sync_Projections			(void)	const	{	return( true  );	}

	virtual bool				needs_GUI					(void)	const	{	return( false );	}
	bool						has_GUI						(void)	const;
	bool						has_CMD						(void)	const;

	virtual bool				is_Grid						(void)	const	{	return( false );	}
	virtual bool				is_Interactive				(void)	const	{	return( false );	}
	bool						is_Progress					(void)	const	{	return( SG_UI_Process_Get_Okay(false) );	}
	bool						is_Executing				(void)	const	{	return( m_bExecutes );	}

	void						Set_Show_Progress			(bool bOn = true);

	virtual bool				On_Before_Execution			(void)	{	return( true );	}
	virtual bool				On_After_Execution			(void)	{	return( true );	}

	bool						Execute						(bool bAddHistory = false);

	const SG_Char *				Get_Execution_Info			(void)	const	{	return( m_Execution_Info );	}

	CSG_MetaData				Get_History					(int Depth = -1);
	bool						Set_History					(CSG_Data_Object *pDataObject, int Depth = -1);


	//-----------------------------------------------------
	enum class Script_Format
	{
		CMD_Shell, CMD_Batch, CMD_Usage,
		Toolchain,
		Python,
		Python_Wrapper_Func_Name, Python_Wrapper_Func_ID,
		Python_Wrapper_Call_Name, Python_Wrapper_Call_ID
	};

	CSG_String					Get_Script					(Script_Format Format, bool bHeader = true, int Arguments = 0, bool bWrapArgs = true);


protected:

	CSG_Parameters				Parameters;

	CSG_MetaData				History_Supplement;


	//-----------------------------------------------------
	void						Set_Name					(const CSG_String &String);
	void						Set_Author					(const CSG_String &String);
	void						Set_Version					(const CSG_String &String);
	void						Set_Description				(const CSG_String &String);

	void						Add_Reference				(const CSG_String &Authors, const CSG_String &Year, const CSG_String &Title, const CSG_String &Where, const SG_Char *Link = NULL, const SG_Char *Link_Text = NULL);
	void						Add_Reference				(const CSG_String &Link, const SG_Char *Link_Text = NULL);

	//-----------------------------------------------------
	virtual bool				On_Execute					(void)	= 0;

	virtual int					On_Parameter_Changed		(CSG_Parameters *pParameters, CSG_Parameter *pParameter);
	virtual int					On_Parameters_Enable		(CSG_Parameters *pParameters, CSG_Parameter *pParameter);

	TSG_PFNC_Parameter_Changed	Get_Parameter_Changed		(void)	{	return( _On_Parameter_Changed );	}

	//-----------------------------------------------------
	CSG_Parameters *			Add_Parameters				(const CSG_String &Identifier, const CSG_String &Name, const CSG_String &Description);
	bool						Dlg_Parameters				(const CSG_String &Identifier); // deprecated

	bool						Dlg_Parameters				(CSG_Parameters *pParameters, const CSG_String &Caption = "");
	bool						Dlg_Parameters				(CSG_Parameters  &Parameters, const CSG_String &Caption = "");

	//-----------------------------------------------------
	virtual bool				Set_Progress				(int    Position, int    Range     )	const;
	virtual bool				Set_Progress				(sLong  Position, sLong  Range     )	const;
	virtual bool				Set_Progress				(double Position, double Range = 1.)	const;

	bool						Stop_Execution				(bool bDialog = true);

	void						Message_Dlg					(const CSG_String &Text, const SG_Char *Caption = NULL);
	bool						Message_Dlg_Confirm			(const CSG_String &Text, const SG_Char *Caption = NULL);

	void						Message_Add					(const CSG_String &Text, bool bNewLine = true);
	void						Message_Fmt					(const char    *Format, ...);
	void						Message_Fmt					(const wchar_t *Format, ...);

	bool						Error_Set					(TSG_Tool_Error Error_ID = TOOL_ERROR_Unknown);
	bool						Error_Set					(const CSG_String &Text);
	bool						Error_Fmt					(const char    *Format, ...);
	bool						Error_Fmt					(const wchar_t *Format, ...);

	//-----------------------------------------------------
	bool						DataObject_Add				(CSG_Data_Object *pDataObject, bool bUpdate = false);

	bool						DataObject_Update_All		(void);

	bool						DataObject_Set_History		(CSG_Parameter *pParameter, CSG_MetaData *pHistory = NULL);

	bool						Get_Projection				(CSG_Projection &Projection) const;


protected: // static functions...

	static int					_On_Parameter_Changed				(CSG_Parameter *pParameter, int Flags);

	static bool					Process_Get_Okay					(bool bBlink = false);
	static void					Process_Set_Text					(const CSG_String &Text);
	static void					Process_Set_Text					(const char    *Format, ...);
	static void					Process_Set_Text					(const wchar_t *Format, ...);

	static bool					DataObject_Update					(CSG_Data_Object *pDataObject                                , int Show = SG_UI_DATAOBJECT_UPDATE);
	static bool					DataObject_Update					(CSG_Data_Object *pDataObject, double Minimum, double Maximum, int Show = SG_UI_DATAOBJECT_UPDATE);

	static bool					DataObject_Get_Colors				(CSG_Data_Object *pDataObject, CSG_Colors &Colors);
	static bool					DataObject_Set_Colors				(CSG_Data_Object *pDataObject, const CSG_Colors &Colors);
	static bool					DataObject_Set_Colors				(CSG_Data_Object *pDataObject, int nColors, int Colors = SG_COLORS_DEFAULT, bool bRevert = false);

	static bool					DataObject_Get_Parameters			(CSG_Data_Object *pDataObject, CSG_Parameters &Parameters);
	static bool					DataObject_Set_Parameters			(CSG_Data_Object *pDataObject, CSG_Parameters &Parameters);
	static bool					DataObject_Set_Parameters			(CSG_Data_Object *pDataObject, CSG_Data_Object *pCopy);

	static CSG_Parameter *		DataObject_Get_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, CSG_Parameter *pParameter);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, CSG_Data_Object *pCopy, const CSG_String &ID);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID, int            Value);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID, double         Value);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID, void          *Value);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID, const SG_Char *Value);
	static bool					DataObject_Set_Parameter			(CSG_Data_Object *pDataObject, const CSG_String &ID, double loVal, double hiVal); // Range Parameter

	static bool					DataObject_Set_Stretch_Linear		(CSG_Data_Object *pDataObject, int Field = -1, double Minimum = 5., double Maximum = 95.    , int Interval_Mode = 0, double Interval_Log = 1., int Colors = -1, bool bUpdateNow = false);
	static bool					DataObject_Set_Stretch_StdDev		(CSG_Data_Object *pDataObject, int Field = -1, double StdDev = 2., bool bKeepInRange = false, int Interval_Mode = 0, double Interval_Log = 1., int Colors = -1, bool bUpdateNow = false);
	static bool					DataObject_Set_Stretch_Percentile	(CSG_Data_Object *pDataObject, int Field = -1, double Minimum = 2., double Maximum = 98.    , int Interval_Mode = 0, double Interval_Log = 1., int Colors = -1, bool bUpdateNow = false);
	static bool					DataObject_Set_Stretch				(CSG_Data_Object *pDataObject, int Field     , double Minimum     , double Maximum          , int Interval_Mode = 0, double Interval_Log = 1., int Colors = -1, bool bUpdateNow = false);

	static bool					DataObject_Classify_Unique			(CSG_Data_Object *pDataObject, int Field = -1, int Classes_Max = 1024                                   , int Colors = -1);
	static bool					DataObject_Classify_Equal			(CSG_Data_Object *pDataObject, int Field = -1, int Classes = 10                                         , int Colors = -1);
	static bool					DataObject_Classify_Defined			(CSG_Data_Object *pDataObject, int Field     , double Interval, bool bOffset = false, double Offset = 0., int Colors = -1);
	static bool					DataObject_Classify_Quantile		(CSG_Data_Object *pDataObject, int Field = -1, int Classes = 10, bool bHistogram = true                 , int Colors = -1);
	static bool					DataObject_Classify_Geometric		(CSG_Data_Object *pDataObject, int Field = -1, int Classes = 10, bool bIncreasing = true                , int Colors = -1);
	static bool					DataObject_Classify_Natural			(CSG_Data_Object *pDataObject, int Field = -1, int Classes = 10                                         , int Colors = -1);
	static bool					DataObject_Classify_StdDev			(CSG_Data_Object *pDataObject, int Field = -1, double StdDev = 0.5, double StdDev_Max = 4.5             , int Colors = -1);


private:

	bool						m_bExecutes, m_bError_Ignore, m_bShow_Progress, m_bGUI, m_bCMD;

	CSG_Array_Pointer			m_pParameters;

	CSG_String					m_ID, m_Library, m_Library_Menu, m_File_Name, m_Author, m_Version, m_Execution_Info;


	bool						_Synchronize_DataObjects	(void);

	CSG_String					_Get_Script_CMD				(bool bHeader                                   , bool bAllArguments, bool bWrapArgs, bool bBatch);
	void						_Get_Script_CMD				(CSG_String &Script, CSG_Parameters *pParameters, bool bAllArguments, bool bWrapArgs, bool bBatch);

	CSG_String					_Get_Script_CMD_Usage		(void);
	void						_Get_Script_CMD_Usage		(CSG_Parameters *pParameters, class wxCmdLineParser &Parser);

	CSG_String					_Get_Script_Python			(bool bHeader, bool bAllArguments);
	void						_Get_Script_Python			(CSG_String &Script, CSG_Parameters *pParameters, bool bAllArguments, const CSG_String &Prefix = "");

	CSG_String					_Get_Script_Python_Wrap		(bool bHeader, bool bName, bool bCall, int AllArguments, bool bWrapArgs);
	bool						_Get_Script_Python_Wrap		(const CSG_Parameter &Parameter, int Constraint, CSG_String &Arguments, CSG_String &Description, CSG_String &Code, bool bCall, bool bOnlyNonDefaults, bool bWrapArgs, const CSG_String &Prefix = "");

	void						_Set_Output_History			(void);

	void						_Update_Parameter_States	(CSG_Parameters *pParameters);

};


///////////////////////////////////////////////////////////
//                                                       //
//					CSG_Tool_Grid						 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
  * CSG_Tool_Grid.
*/
//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool_Grid : public CSG_Tool
{
public:
	CSG_Tool_Grid(void);
	virtual ~CSG_Tool_Grid(void);

	virtual TSG_Tool_Type		Get_Type				(void)	const	{	return( TOOL_TYPE_Grid );	}

	const CSG_Grid_System &		Get_System				(void)	const	{	return( *Parameters.Get_Grid_System() );	}
	bool						Set_System				(const CSG_Grid_System &System);

	virtual bool				is_Grid					(void)	const	{	return( true );	}


protected:

	virtual bool				Set_Progress_Cells		(sLong Cell)					const;
	virtual bool				Set_Progress_Rows		(int    Row)					const;

	//-----------------------------------------------------
	int							Get_NX					(void)						const	{	return( Get_System().Get_NX      () );	}
	int							Get_NY					(void)						const	{	return( Get_System().Get_NY      () );	}
	sLong						Get_NCells				(void)						const	{	return( Get_System().Get_NCells  () );	}
	double						Get_XMin				(void)						const	{	return( Get_System().Get_XMin    () );	}
	double						Get_YMin				(void)						const	{	return( Get_System().Get_YMin    () );	}
	double						Get_XMax				(void)						const	{	return( Get_System().Get_XMax    () );	}
	double						Get_YMax				(void)						const	{	return( Get_System().Get_YMax    () );	}
	double						Get_Cellsize			(void)						const	{	return( Get_System().Get_Cellsize() );	}
	double						Get_Cellarea			(void)						const	{	return( Get_System().Get_Cellarea() );	}

	double						Get_Length				(int i)						const	{	return( Get_System().Get_Length    (i)     );	}
	double						Get_UnitLength			(int i)						const	{	return( Get_System().Get_UnitLength(i)     );	}
	bool						is_InGrid				(int x, int y)				const	{	return(	Get_System().is_InGrid(x, y)       );	}
	bool						is_InGrid				(int x, int y, int Rand)	const	{	return(	Get_System().is_InGrid(x, y, Rand) );	}

	static int					Get_xTo					(int i, int x = 0)	{	return( CSG_Grid_System::Get_xTo  (i, x) );	}
	static int					Get_yTo					(int i, int y = 0)	{	return( CSG_Grid_System::Get_yTo  (i, y) );	}
	static int					Get_xFrom				(int i, int x = 0)	{	return( CSG_Grid_System::Get_xFrom(i, x) );	}
	static int					Get_yFrom				(int i, int y = 0)	{	return( CSG_Grid_System::Get_yFrom(i, y) );	}

	//-----------------------------------------------------
	void						Lock_Create				(void);
	void						Lock_Destroy			(void);

	bool						is_Locked				(int x, int y)	{	return( Lock_Get(x, y) != 0 );	}
	char						Lock_Get				(int x, int y)	{	return( m_pLock && x >= 0 && x < Get_NX() && y >= 0 && y < Get_NY() ? m_pLock->asChar(x, y) : 0 );	}

	void						Lock_Set				(int x, int y, char Value = 1)
	{
		if( m_pLock && x >= 0 && x < Get_NX() && y >= 0 && y < Get_NY() )
		{
			m_pLock->Set_Value(x, y, Value);
		}
	}


private:

	CSG_Grid					*m_pLock;

};


///////////////////////////////////////////////////////////
//                                                       //
//				CSG_Tool_Interactive_Base				 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
typedef enum
{
	TOOL_INTERACTIVE_UNDEFINED		= 0,
	TOOL_INTERACTIVE_LDOWN,
	TOOL_INTERACTIVE_LUP,
	TOOL_INTERACTIVE_LDCLICK,
	TOOL_INTERACTIVE_MDOWN,
	TOOL_INTERACTIVE_MUP,
	TOOL_INTERACTIVE_MDCLICK,
	TOOL_INTERACTIVE_RDOWN,
	TOOL_INTERACTIVE_RUP,
	TOOL_INTERACTIVE_RDCLICK,
	TOOL_INTERACTIVE_MOVE,
	TOOL_INTERACTIVE_MOVE_LDOWN,
	TOOL_INTERACTIVE_MOVE_MDOWN,
	TOOL_INTERACTIVE_MOVE_RDOWN
}
TSG_Tool_Interactive_Mode;

//---------------------------------------------------------
typedef enum
{
	TOOL_INTERACTIVE_DRAG_NONE		= 0,
	TOOL_INTERACTIVE_DRAG_LINE,
	TOOL_INTERACTIVE_DRAG_BOX,
	TOOL_INTERACTIVE_DRAG_CIRCLE
}
TSG_Tool_Interactive_DragMode;

//---------------------------------------------------------
#define TOOL_INTERACTIVE_KEY_LEFT	0x01
#define TOOL_INTERACTIVE_KEY_MIDDLE	0x02
#define TOOL_INTERACTIVE_KEY_RIGHT	0x04
#define TOOL_INTERACTIVE_KEY_SHIFT	0x08
#define TOOL_INTERACTIVE_KEY_ALT	0x10
#define TOOL_INTERACTIVE_KEY_CTRL	0x20

//---------------------------------------------------------
/**
  * CSG_Tool_Interactive_Base.
*/
//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool_Interactive_Base
{
	friend class CSG_Tool_Interactive;
	friend class CSG_Tool_Grid_Interactive;

public:
	CSG_Tool_Interactive_Base(void);
	virtual ~CSG_Tool_Interactive_Base(void);

	bool						Set_Projection			(const CSG_Projection &Projection);

	bool						Execute_Position		(CSG_Point ptWorld, TSG_Tool_Interactive_Mode Mode, int Keys);
	bool						Execute_Keyboard		(int Character, int Keys);
	bool						Execute_Finish			(void);

	int							Get_Drag_Mode			(void)	const	{	return( m_Drag_Mode );	}


protected:

	const CSG_Projection &		Get_Projection			(void)	const	{	return( m_Projection );	}

	virtual bool				On_Execute_Position		(CSG_Point ptWorld, TSG_Tool_Interactive_Mode Mode);
	virtual bool				On_Execute_Keyboard		(int Character);
	virtual bool				On_Execute_Finish		(void);

	const CSG_Point &			Get_Position			(void)	const	{	return( m_Point        );	}
	double						Get_xPosition			(void)	const	{	return( m_Point     .x );	}
	double						Get_yPosition			(void)	const	{	return( m_Point     .y );	}

	const CSG_Point &			Get_Position_Last		(void)	const	{	return( m_Point_Last   );	}
	double						Get_xPosition_Last		(void)	const	{	return( m_Point_Last.x );	}
	double						Get_yPosition_Last		(void)	const	{	return( m_Point_Last.y );	}

	bool						is_Shift				(void)	const	{	return( (m_Keys & TOOL_INTERACTIVE_KEY_SHIFT) != 0 );	}
	bool						is_Alt					(void)	const	{	return( (m_Keys & TOOL_INTERACTIVE_KEY_ALT  ) != 0 );	}
	bool						is_Ctrl					(void)	const	{	return( (m_Keys & TOOL_INTERACTIVE_KEY_CTRL ) != 0 );	}

	void						Set_Drag_Mode			(int Drag_Mode);


private:

	int							m_Keys, m_Drag_Mode;

	CSG_Point					m_Point, m_Point_Last;

	CSG_Projection				m_Projection;

	CSG_Tool					*m_pTool;

};


///////////////////////////////////////////////////////////
//                                                       //
//				CSG_Tool_Interactive					 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
  * CSG_Tool_Interactive.
*/
//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool_Interactive : public CSG_Tool_Interactive_Base, public CSG_Tool
{
public:
	CSG_Tool_Interactive(void);
	virtual ~CSG_Tool_Interactive(void);

	virtual TSG_Tool_Type		Get_Type				(void)	const	{	return( TOOL_TYPE_Interactive );	}

	virtual bool				needs_GUI				(void)	const	{	return( true );	}

	virtual bool				is_Interactive			(void)	const	{	return( true );	}

};


///////////////////////////////////////////////////////////
//                                                       //
//				CSG_Tool_Grid_Interactive				 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/**
  * CSG_Tool_Grid_Interactive.
*/
//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool_Grid_Interactive : public CSG_Tool_Interactive_Base, public CSG_Tool_Grid
{
public:
	CSG_Tool_Grid_Interactive(void);
	virtual ~CSG_Tool_Grid_Interactive(void);

	virtual TSG_Tool_Type		Get_Type				(void)	const	{	return( TOOL_TYPE_Grid_Interactive );	}

	virtual bool				needs_GUI				(void)	const	{	return( true );	}

	virtual bool				is_Interactive			(void)	const	{	return( true );	}


protected:

	bool						Get_Grid_Pos			(int &x, int &y)	const;

	int							Get_xGrid				(void)	const;
	int							Get_yGrid				(void)	const;

};


///////////////////////////////////////////////////////////
//                                                       //
//			Tool Library Interface Definitions			 //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
typedef enum
{
	TLB_INFO_Name	= 0,
	TLB_INFO_Description,
	TLB_INFO_Author,
	TLB_INFO_Version,
	TLB_INFO_Menu_Path,
	TLB_INFO_Category,
	TLB_INFO_User,
	TLB_INFO_File,
	TLB_INFO_Library,
	TLB_INFO_SAGA_Version,
	TLB_INFO_Count
}
TSG_TLB_Info;

//---------------------------------------------------------
typedef CSG_Tool *	(* TSG_PFNC_TLB_Create_Tool)	(int i);
typedef CSG_String	(* TSG_PFNC_TLB_Get_Info   )	(int i);

//---------------------------------------------------------
class SAGA_API_DLL_EXPORT CSG_Tool_Library_Interface
{
public:
	CSG_Tool_Library_Interface(void);
	virtual ~CSG_Tool_Library_Interface(void);

	bool						Create					(const CSG_String &Version, const CSG_String &TLB_Path, TSG_PFNC_TLB_Get_Info Fnc_Info, TSG_PFNC_TLB_Create_Tool Fnc_Create_Tool);
	bool						Destroy					(void);

	const CSG_String &			Get_Info				(int ID);

	int							Get_Count				(void);
	CSG_Tool *					Get_Tool				(int i);

	CSG_Tool *					Create_Tool				(int i, bool bWithGUI = false, bool bWithCMD =true);
	bool						Delete_Tool				(CSG_Tool *pTool);
	bool						Delete_Tools			(void);


private:

	CSG_Strings					m_Info;

	CSG_Array_Pointer			m_Tools, m_xTools;

	TSG_PFNC_TLB_Create_Tool	m_Fnc_Create_Tool;

};

//---------------------------------------------------------
#define SYMBOL_TLB_Initialize			"TLB_Initialize"
typedef bool							(* TSG_PFNC_TLB_Initialize)		(const SG_Char *);

#define SYMBOL_TLB_Finalize				"TLB_Finalize"
typedef bool							(* TSG_PFNC_TLB_Finalize)		(void);

#define SYMBOL_TLB_Get_Interface		"TLB_Get_Interface"
typedef CSG_Tool_Library_Interface *	(* TSG_PFNC_TLB_Get_Interface)	(void);

//---------------------------------------------------------
#define TLB_INTERFACE_SKIP_TOOL			((CSG_Tool *)0x1)

//---------------------------------------------------------
#define TLB_INTERFACE_ESTABLISH CSG_Tool_Library_Interface TLB_Interface;\
\
extern "C" _SAGA_DLL_EXPORT CSG_Tool_Library_Interface *	TLB_Get_Interface   (void)\
{\
	return( &TLB_Interface );\
}\

//---------------------------------------------------------
#define TLB_INTERFACE_INITIALIZE extern "C" _SAGA_DLL_EXPORT bool TLB_Initialize(const SG_Char *TLB_Path)\
{\
	return( TLB_Interface.Create(SAGA_VERSION, TLB_Path, Get_Info, Create_Tool) );\
}\

//---------------------------------------------------------
#define TLB_INTERFACE_FINALIZE   extern "C" _SAGA_DLL_EXPORT bool TLB_Finalize  (void)\
{\
	return( true );\
}\

//---------------------------------------------------------
#define TLB_INTERFACE	TLB_INTERFACE_ESTABLISH TLB_INTERFACE_INITIALIZE TLB_INTERFACE_FINALIZE

//---------------------------------------------------------
#ifndef SWIG

extern CSG_Tool_Library_Interface	TLB_Interface;

#endif	// #ifdef SWIG


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__SAGA_API__tool_H
