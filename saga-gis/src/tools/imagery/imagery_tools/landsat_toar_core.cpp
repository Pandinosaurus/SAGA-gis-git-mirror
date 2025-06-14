#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "landsat_toar_core.h"

#include <saga_api/saga_api.h>


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
double	julian_int	(int year, int month, int day);
double	julian_char	(const char date[]);
double	earth_sun	(const char date[]);


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void G_debug	(int i, const char *s)
{
	SG_UI_Msg_Add_Execution('\n', false);
	SG_UI_Msg_Add_Execution(s   , false);
}

//---------------------------------------------------------
void G_warning	(const char *s)
{
	SG_UI_Msg_Add_Execution('\n', false);
	SG_UI_Msg_Add_Execution(s   , false, SG_UI_MSG_STYLE_FAILURE);
}


///////////////////////////////////////////////////////////
//														 //
//						landsat_met.c					 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define MAX_STR         127
#define METADATA_SIZE   65535  /* MTL.txt file size  65535 bytes */
#define TM5_MET_SIZE    28700  /* .met file size 28686 bytes */

inline void chrncpy(char *dest, const char src[], int n)
{
    if (src == NULL)
    {
        dest[0] = '\0';
    }
    else
    {
        int i;
        for (i = 0; i < n && src[i] != '\0' && src[i] != '\"'; i++) dest[i] = src[i];
        dest[i] = '\0';
    }
}


/****************************************************************************
 * PURPOSE:     Read values of Landsat MSS/TM from header (.met) file
 *****************************************************************************/
void get_metdata(const char metdata[], const char *text, char value[])
{
    const char *ptr	= strstr(metdata, text);

    if (ptr == NULL)
    {
        value[0] = 0;
        return;
    }

    ptr = strstr(ptr, " VALUE ");
    if (ptr == NULL) return;

    while (*ptr++ != '\"') ;
    int i = 0;
    while (*ptr != '\"' && i < MAX_STR) value[i++] = *ptr++;
    value[i] = '\0';

    return;
}

bool lsat_metdata(const char metdata[], lsat_data *lsat)
{
	char	value[MAX_STR];

    /* --------------------------------------- */
    get_metdata(metdata, "PLATFORMSHORTNAME", value);
    lsat->number = atoi(value + 8);

    get_metdata(metdata, "SENSORSHORTNAME", value);
    chrncpy(lsat->sensor, value + 1, 4);

    get_metdata(metdata, "CALENDARDATE", value);
    chrncpy(lsat->date, value, 10);

	get_metdata(metdata, "PRODUCTIONDATETIME", value);
	chrncpy(lsat->creation, value, 10);

	get_metdata(metdata, "SolarElevation", value);
	lsat->sun_elev = atof(value);

    /* Fill data with the sensor_XXX functions */
	switch( lsat->number )
	{
	default: return( false );
	case 1:	set_MSS1(lsat); break;
	case 2:	set_MSS2(lsat); break;
	case 3:	set_MSS3(lsat); break;
	case 4:	if( lsat->sensor[0] == 'M' ) set_MSS4(lsat); else set_TM4(lsat); break;
	case 5:	if( lsat->sensor[0] == 'M' ) set_MSS5(lsat); else set_TM5(lsat); break;
    }

	// "RADIANCE & QUANTIZE from band setting of the metadata file"
	for(int i=0; i<lsat->bands; i++)
	{
		CSG_String	key;
		
		key.Printf(SG_T("Band%dGainSetting"), lsat->band[i].code);
		get_metdata(metdata, key, value);
		if (value[0] == '\0')
		{
			G_warning(key);
			continue;
		}
		lsat->band[i].gain = atof(value);

		key.Printf(SG_T("Band%dBiasSetting"), lsat->band[i].code);
		get_metdata(metdata, key, value);
		if (value[0] == '\0')
		{
			G_warning(key);
			continue;
		}

		lsat->band[i].bias		= atof(value);
		lsat->band[i].qcalmax	= 255.;
		lsat->band[i].qcalmin	= 1.;
		lsat->band[i].lmin		= lsat->band[i].gain * lsat->band[i].qcalmin + lsat->band[i].bias;
		lsat->band[i].lmax		= lsat->band[i].gain * lsat->band[i].qcalmax + lsat->band[i].bias;
	}

    /* --------------------------------------- */
	lsat->flag	= METADATAFILE;

    return( *lsat->sensor ? true : false );
}


/****************************************************************************
 * PURPOSE:     Read values of Landsat from MTL metadata (MTL.txt) file
 *****************************************************************************/

//---------------------------------------------------------
bool Load_MetaData(const CSG_String &filename, CSG_MetaData &MetaData)
{
	CSG_String	sLine, sKey, sVal;
	CSG_File	f;

	MetaData.Destroy();

	if( !f.Open(filename, SG_FILE_R, false) )
	{
		return( false );
	}

	while( !f.is_EOF() && f.Read_Line(sLine) )
	{
		sKey	= sLine.BeforeFirst('='); sKey.Trim(false); sKey.Trim(true);

		if( !sKey.is_Empty() && sKey.CmpNoCase("GROUP") && sKey.CmpNoCase("END_GROUP") )
		{
			sVal	= sLine.AfterFirst ('='); sVal.Trim(false); sVal.Trim(true); sVal.Replace("\"", "");

			MetaData.Add_Child(sKey, sVal);
		}
	}

	return( true );
}

//---------------------------------------------------------
bool Get_MetaData(const CSG_MetaData &MetaData, const CSG_String &key, CSG_String &value)
{
	CSG_MetaData	*p	= MetaData.Get_Child(key);

	if( p )
	{
		value	= p->Get_Content();

		return( true );
	}

	value.Clear();

	return( false );
}

//---------------------------------------------------------
#define GET_METADATA(key)		if( Get_MetaData(m, key, s) == false ) return( false );
#define IF_GET_METADATA(key)	if( Get_MetaData(m, key, s) == true )

//---------------------------------------------------------
bool lsat_old_mtl(const CSG_MetaData &m, lsat_data *lsat)
{
	CSG_String	s;

	//-------------------------------------------
	IF_GET_METADATA("SPACECRAFT_ID"              )	lsat->number = CSG_String(s.Get_Char(7)).asInt();	else return( false );
	IF_GET_METADATA("SENSOR_ID"                  )	chrncpy(lsat->sensor  , s,  4);						else return( false );
	IF_GET_METADATA("ACQUISITION_DATE"           )	chrncpy(lsat->date    , s, 10);
	IF_GET_METADATA("PRODUCT_CREATION_TIME"      )	chrncpy(lsat->creation, s, 10);	else
	IF_GET_METADATA("ORTHO_PRODUCT_CREATION_TIME")	chrncpy(lsat->creation, s, 10);
	IF_GET_METADATA("SUN_ELEVATION"              )	lsat->sun_elev = s.asDouble();

	switch( lsat->number )	// Fill data with the sensor_XXX functions
	{
	default: return( false );
	case 1:	set_MSS1(lsat); break;
	case 2:	set_MSS2(lsat); break;
	case 3:	set_MSS3(lsat); break;
	case 4:	if( lsat->sensor[0] == 'M' ) set_MSS4(lsat); else set_TM4(lsat); break;
	case 5:	if( lsat->sensor[0] == 'M' ) set_MSS5(lsat); else set_TM5(lsat); break;
	case 8:	set_OLI(lsat); break;
	case 7:
		{
			char	gain[9];

			GET_METADATA("BAND1_GAIN" ); gain[0] = *s.b_str();
			GET_METADATA("BAND2_GAIN" ); gain[1] = *s.b_str();
			GET_METADATA("BAND3_GAIN" ); gain[2] = *s.b_str();
			GET_METADATA("BAND4_GAIN" ); gain[3] = *s.b_str();
			GET_METADATA("BAND5_GAIN" ); gain[4] = *s.b_str();
			GET_METADATA("BAND6_GAIN1"); gain[5] = *s.b_str();
			GET_METADATA("BAND6_GAIN2"); gain[6] = *s.b_str();
			GET_METADATA("BAND7_GAIN" ); gain[7] = *s.b_str();
			GET_METADATA("BAND8_GAIN" ); gain[8] = *s.b_str();

			set_ETM(lsat, gain);
		}
		break;
    }

	//-----------------------------------------------------
    for(int i=0; i<lsat->bands; i++)	// Update the information from metadata file
	{
		CSG_String	Code(CSG_String::Format(SG_T("%d"), lsat->band[i].code));

		IF_GET_METADATA("LMAX_BAND"    + Code) lsat->band[i].lmax    = s.asDouble();
		IF_GET_METADATA("LMIN_BAND"    + Code) lsat->band[i].lmin    = s.asDouble();
        IF_GET_METADATA("QCALMAX_BAND" + Code) lsat->band[i].qcalmax = s.asDouble();
        IF_GET_METADATA("QCALMIN_BAND" + Code) lsat->band[i].qcalmin = s.asDouble();
    }

	//-----------------------------------------------------
	lsat->flag	= METADATAFILE;

    return( *lsat->sensor ? true : false );
}

//---------------------------------------------------------
bool lsat_new_mtl(const CSG_MetaData &m, lsat_data * lsat)
{
	CSG_String	s;

	//-----------------------------------------------------
	GET_METADATA("SPACECRAFT_ID"); lsat->number		= CSG_String(s.Get_Char(8)).asInt();
	GET_METADATA("SENSOR_ID"    ); chrncpy(lsat->sensor  , s,  4);
	GET_METADATA("DATE_ACQUIRED"); chrncpy(lsat->date    , s, 10);
    GET_METADATA("FILE_DATE"    ); chrncpy(lsat->creation, s, 10);
	GET_METADATA("SUN_ELEVATION"); lsat->sun_elev	= s.asDouble();

	switch( lsat->number )	// Fill data with the sensor_XXX functions
	{
	default: return( false );
	case 1:	set_MSS1(lsat); break;
	case 2:	set_MSS2(lsat); break;
	case 3:	set_MSS3(lsat); break;
	case 4:	if( lsat->sensor[0] == 'M' ) set_MSS4(lsat); else set_TM4(lsat); break;
	case 5:	if( lsat->sensor[0] == 'M' ) set_MSS5(lsat); else set_TM5(lsat); break;
	case 8:	set_OLI(lsat); break;
	case 7:
		{
			char	gain[9];

			GET_METADATA("GAIN_BAND_1"       ); gain[0] = *s.b_str();
			GET_METADATA("GAIN_BAND_2"       ); gain[1] = *s.b_str();
			GET_METADATA("GAIN_BAND_3"       ); gain[2] = *s.b_str();
			GET_METADATA("GAIN_BAND_4"       ); gain[3] = *s.b_str();
			GET_METADATA("GAIN_BAND_5"       ); gain[4] = *s.b_str();
			GET_METADATA("GAIN_BAND_6_VCID_1"); gain[5] = *s.b_str();
			GET_METADATA("GAIN_BAND_6_VCID_2"); gain[6] = *s.b_str();
			GET_METADATA("GAIN_BAND_7"       ); gain[7] = *s.b_str();
			GET_METADATA("GAIN_BAND_8"       ); gain[8] = *s.b_str();

			set_ETM(lsat, gain);
		}
		break;
    }

	//-----------------------------------------------------
	// Update the information from metadata file

	// Other possible values in the metadata file
	IF_GET_METADATA("EARTH_SUN_DISTANCE") lsat->dist_es	= s.asDouble();	// Necessary after

	//-----------------------------------------------------
	// RADIANCE & QUANTIZE from MIN_MAX_(RADIANCE|PIXEL_VALUE)
	if( m.Get_Child("RADIANCE_MAXIMUM_BAND_" + SG_Get_String(lsat->band[0].code)) != NULL )
	{
		for(int i=0; i<lsat->bands; i++)
		{
			CSG_String	Code(lsat->number == 7 && lsat->band[i].thermal
				? CSG_String::Format(SG_T("_6_VCID_%d"), lsat->band[i].code - 60)
				: CSG_String::Format(SG_T(       "_%d"), lsat->band[i].code)
			);

			IF_GET_METADATA("RADIANCE_MAXIMUM_BAND" + Code) lsat->band[i].lmax    = s.asDouble();
			IF_GET_METADATA("RADIANCE_MINIMUM_BAND" + Code) lsat->band[i].lmin    = s.asDouble();
			IF_GET_METADATA("QUANTIZE_CAL_MAX_BAND" + Code) lsat->band[i].qcalmax = s.asDouble();
			IF_GET_METADATA("QUANTIZE_CAL_MIN_BAND" + Code) lsat->band[i].qcalmin = s.asDouble();

			// other possible values of each band
			if( lsat->band[i].thermal )
			{
				Code.Printf(SG_T("%d"), lsat->band[i].code);	// ??!! see grass implementation!!

				IF_GET_METADATA("K1_CONSTANT_BAND" + Code) lsat->band[i].K1	= s.asDouble();
				IF_GET_METADATA("K2_CONSTANT_BAND" + Code) lsat->band[i].K2	= s.asDouble();
			}
			else if( lsat->number == 8 )	// ESUN from  REFLECTANCE and RADIANCE ADD_BAND
			{
				IF_GET_METADATA("REFLECTANCE_MAXIMUM_BAND" + Code)
					lsat->band[i].esun	= (M_PI * lsat->dist_es * lsat->dist_es * lsat->band[i].lmax) / s.asDouble();
			}
		}

		if( lsat->number == 8 )
		{
			G_warning("ESUN evaluated from REFLECTANCE_MAXIMUM_BAND");
		}
	}

	//-----------------------------------------------------
	// RADIANCE & QUANTIZE from RADIOMETRIC_RESCALING
	else
	{
		for(int i=0; i<lsat->bands; i++)
		{
			CSG_String	Code(lsat->number == 7 && lsat->band[i].thermal
				? CSG_String::Format(SG_T("_6_VCID_%d"), lsat->band[i].code - 60)
				: CSG_String::Format(SG_T(       "_%d"), lsat->band[i].code)
			);

			IF_GET_METADATA("RADIANCE_MULT_BAND" + Code)	lsat->band[i].gain	= s.asDouble();
			IF_GET_METADATA("RADIANCE_ADD_BAND"  + Code)	lsat->band[i].bias	= s.asDouble();

			// reversing to calculate the values of Lmin and Lmax ...
			lsat->band[i].lmin	= lsat->band[i].gain * lsat->band[i].qcalmin + lsat->band[i].bias;
			lsat->band[i].lmax	= lsat->band[i].gain * lsat->band[i].qcalmax + lsat->band[i].bias;

			// ... qcalmax and qcalmin loaded in previous sensor_function
			if( lsat->number == 8 )
			{
				if( lsat->band[i].thermal )
				{
					IF_GET_METADATA("K1_CONSTANT_BAND"      + Code)	lsat->band[i].K1 = s.asDouble();
					IF_GET_METADATA("K2_CONSTANT_BAND"      + Code)	lsat->band[i].K2 = s.asDouble();
				}
				else	// ESUN from REFLECTANCE_ADD_BAND
				{
					IF_GET_METADATA("REFLECTANCE_MULT_BAND" + Code)	lsat->band[i].K1 = s.asDouble();
					IF_GET_METADATA("REFLECTANCE_ADD_BAND"  + Code)	lsat->band[i].K2 = s.asDouble();

					lsat->band[i].esun	= (M_PI * lsat->dist_es * lsat->dist_es * lsat->band[i].bias) / lsat->band[i].K2;

				//	double esun1	= (M_PI * lsat->dist_es * lsat->dist_es * lsat->band[i].bias) / lsat->band[i].K2;
				//	double esun2	= (M_PI * lsat->dist_es * lsat->dist_es * lsat->band[i].gain) / lsat->band[i].K1;
				//	lsat->band[i].esun	= (esun1 + esun2) / 2.;
				}
			}
		}

		G_warning("ESUN evaluated from REFLECTANCE_ADDITIVE_FACTOR_BAND");
	}

	//-----------------------------------------------------
	lsat->flag	= METADATAFILE;

    return( *lsat->sensor ? true : false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool lsat_metadata(const char *metafile, lsat_data *lsat)
{
	memset(lsat, 0, sizeof(lsat_data));

	//-----------------------------------------------------
	FILE	*f	= fopen(metafile, "r");

	if( f == NULL )
	{
		G_warning("Metadata file not found");

		return( false );
	}

	//-----------------------------------------------------
	char	mtldata[METADATA_SIZE];
	size_t	i	= fread(mtldata, METADATA_SIZE, 1, f);
	fclose(f);

	//-----------------------------------------------------
	if( strstr(mtldata, " VALUE ") != NULL )	// get_metformat
	{
		return( lsat_metdata(mtldata, lsat) );
	}

	//-----------------------------------------------------
	CSG_MetaData	m;

	if( !Load_MetaData(metafile, m) )
	{
		return( false );
	}

	if( m.Get_Child("QCALMAX_BAND1") != NULL )	// ver_mtl = (strstr(mtldata, "QCALMAX_BAND") != NULL) ? 0 : 1;
	{
		return( lsat_old_mtl(m, lsat) );	// old format
	}

	return( lsat_new_mtl(m, lsat) );	// new format
}


///////////////////////////////////////////////////////////
//														 //
//						landsat_set.c					 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void sensor_MSS(lsat_data * lsat)
{
	/* green, red, near infrared, near infrared */
	int		band[4] = {   1,   2,   3,   4 };
	int		code[4] = {   4,   5,   6,   7 };
	double	wmax[4] = { 0.6, 0.7, 0.8, 1.1 };
	double	wmin[4] = { 0.5, 0.6, 0.7, 0.8 };
	/* 68x83, 68x83, 68x83, 68x83 */

	strcpy(lsat->sensor, "MSS");

	lsat->bands		= 4;

	for(int i=0; i<lsat->bands; i++)
	{
		lsat->band[i].number	= band[i];
		lsat->band[i].code		= code[i];
		lsat->band[i].wavemax	= wmax[i];
		lsat->band[i].wavemin	= wmin[i];
		lsat->band[i].qcalmax	= 255.;
		lsat->band[i].qcalmin	=   0.;
		lsat->band[i].thermal	= 0;
	}

	return;
}

void sensor_TM(lsat_data * lsat)
{
	/* blue, green red, near infrared, shortwave IR, thermal IR, shortwave IR */
	int		band[7]	= {    1,    2,    3,    4,    5,    6,    7 };
	double	wmax[7]	= { 0.52, 0.60, 0.69, 0.90, 1.75, 12.5, 2.35 };
	double	wmin[7]	= { 0.45, 0.52, 0.63, 0.76, 1.55, 10.4, 2.08 };
	/* 30, 30, 30, 30, 30, 120, 30 */

	if( !lsat->sensor )
	{
		strcpy(lsat->sensor, "TM");
	}

	lsat->bands		= 7;

	for(int i=0; i<lsat->bands; i++)
	{
		lsat->band[i].number	= band[i];
		lsat->band[i].code		= band[i];
		lsat->band[i].wavemax	= wmax[i];
		lsat->band[i].wavemin	= wmin[i];
		lsat->band[i].qcalmax	= 255.0;
		lsat->band[i].qcalmin	=   0.0;	/* Modified in set_TM5 by date */
		lsat->band[i].thermal	= band[i] == 6 ? 1 : 0;
	}

	return;
}

void sensor_ETM(lsat_data * lsat)
{
	/* blue, green, red, near infrared, shortwave IR, thermal IR, shortwave IR, panchromatic */
	int		band[9]	= {     1,     2,     3,    4,    5,    6,    6,    7,    8 };
	int		code[9]	= {     1,     2,     3,    4,    5,   61,   62,    7,    8 };
	double	wmax[9]	= { 0.515, 0.605, 0.690, 0.90, 1.75, 12.5, 12.5, 2.35, 0.90 };
	double	wmin[9]	= { 0.450, 0.525, 0.630, 0.75, 1.55, 10.4, 10.4, 2.09, 0.52 };
	/* 30, 30, 30, 30, 30, 60, 30, 15 */

	strcpy(lsat->sensor, "ETM+");

	lsat->bands		= 9;

	for(int i=0; i<lsat->bands; i++)
	{
		lsat->band[i].number	= band[i];
		lsat->band[i].code		= code[i];
		lsat->band[i].wavemax	= wmax[i];
		lsat->band[i].wavemin	= wmin[i];
		lsat->band[i].qcalmax	= 255.0;
		lsat->band[i].qcalmin	=   1.0;
		lsat->band[i].thermal	= band[i] == 6 ? 1 : 0;
	}

	return;
}

void sensor_OLI(lsat_data * lsat)
{
    int i;

    /* coastal aerosol, blue, green, red, near infrared, shortwave IR (SWIR) 1, SWIR 2, panchromatic,
     * cirrus, thermal infrared (TIR) 1, TIR 2 */
    int band[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    int code[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    double wmin[] =	{ 0.433, 0.450, 0.525, 0.630, 0.845, 1.560, 2.100, 0.500, 1.360, 10.3, 11.5 };
    double wmax[] =	{ 0.453, 0.515, 0.600, 0.680, 0.885, 1.660, 2.300, 0.680, 1.390, 11.3, 12.5 };
    /* 30, 30, 30, 30, 30, 30, 30, 15, 30, 100, 100 */

    strcpy(lsat->sensor, "OLI/TIRS");

    lsat->bands = 11;
    for (i = 0; i < lsat->bands; i++) {
	lsat->band[i].number = *(band + i);
	lsat->band[i].code = *(code + i);
	lsat->band[i].wavemax = *(wmax + i);
	lsat->band[i].wavemin = *(wmin + i);
	lsat->band[i].qcalmax = 65535.;
	lsat->band[i].qcalmin = 1.;
	lsat->band[i].thermal = (lsat->band[i].number > 9 ? 1 : 0);
    }
    return;
}


/** **********************************************
 ** Before access to these functions ...
 ** store previously
 ** >>> acquisition date,
 ** >>> creation date, and
 ** >>> sun_elev
 ** **********************************************/

/****************************************************************************
 * PURPOSE:     Store values of Landsat-1 MSS
 *              July 23, 1972 to January 6, 1978
 *****************************************************************************/
void set_MSS1(lsat_data * lsat)
{
    int i, j;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double lmax[] = { 248., 200., 176., 153. };
    double lmin[] = { 0., 0., 0., 0. };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1824., 1570., 1249., 853.4 };

    lsat->number = 1;
    sensor_MSS(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
    }
    G_debug(1, "Landsat-1 MSS");
    return;
}

/****************************************************************************
 * PURPOSE:     Store values of Landsat-2 MSS
 *              January 22, 1975 to February 5, 1982
 *****************************************************************************/
void set_MSS2(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][4] = {
	{210., 156., 140., 138.},	/* before      July 16, 1975 */
	{263., 176., 152., 130.}	/* on or after July 16, 1975 */
    };
    double Lmin[][4] = {
	{10., 7., 7., 5.},
	{8., 6., 6., 4.}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1824., 1570., 1249., 853.4 };

    julian = julian_char(lsat->creation);
    if (julian < julian_char("1975-07-16"))
	i = 0;
    else
	i = 1;
    lmax = Lmax[i];
    lmin = Lmin[i];

    lsat->number = 2;
    sensor_MSS(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
    }
    G_debug(1, "Landsat-2 MSS");
    return;
}

/****************************************************************************
 * PURPOSE:     Store values of Landsat-3 MSS
 *              March 5, 1978 to March 31, 1983
 *
 *              tiene una banda 8 thermal
 *****************************************************************************/
void set_MSS3(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][4] = {
	{220., 175., 145., 147.},	/* before      June 1, 1978 */
	{259., 179., 149., 128.}	/* on or after June 1, 1978 */
    };
    double Lmin[][4] = {
	{4., 3., 3., 1.},
	{4., 3., 3., 1.}
    };
    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1824., 1570., 1249., 853.4 };

    julian = julian_char(lsat->creation);
    if (julian < julian_char("1978-06-01"))
	i = 0;
    else
	i = 1;
    lmax = Lmax[i];
    lmin = Lmin[i];

    lsat->number = 3;
    sensor_MSS(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
    }
    G_debug(1, "Landsat-3 MSS");
    return;
}

/****************************************************************************
 * PURPOSE:     Store values of Landsat-4 MSS/TM
 *              July 16, 1982 to June 15, 2001
 *****************************************************************************/
void set_MSS4(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][4] = {
	{250., 180., 150., 133.},	/* before      August 26, 1982 */
	{230., 180., 130., 133.},	/* between                     */
	{238., 164., 142., 116.}	/* on or after April 1, 1983   */
    };
    double Lmin[][4] = {
	{2., 4., 4., 3.},
	{2., 4., 4., 3.},
	{4., 4., 5., 4.}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1824., 1570., 1249., 853.4 };

    julian = julian_char(lsat->creation);
    if (julian < julian_char("1982-08-26"))
	    i = 0;
    else if (julian < julian_char("1983-03-31"))
	    i = 1;
    else
	    i = 2;
    lmax = Lmax[i];
    lmin = Lmin[i];

    lsat->number = 4;
    sensor_MSS(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
    }
    G_debug(1, "Landsat-4 MSS");
    return;
}

void set_TM4(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][7] = {
	{158.42, 308.17, 234.63, 224.32, 32.42, 15.64, 17.00},	/* before August 1983      */
	{142.86, 291.25, 225.00, 214.29, 30.00, 12.40, 15.93},	/* before January 15, 1984 */
	{171.00, 336.00, 254.00, 221.00, 31.40, 15.303, 16.60}	/* after  Jaunary 15, 1984 */
    };
    double Lmin[][7] = {
	{-1.52, -2.84, -1.17, -1.51, -0.37, 2.00, -0.15},
	{0.00, 0.00, 0.00, 0.00, 0.00, 4.84, 0.00},
	{-1.52, -2.84, -1.17, -1.51, -0.37, 1.2378, -0.15}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1957., 1825., 1557., 1033., 214.9, 0., 80.72 };

    /* Thermal band calibration constants: K1 = 671.62   K2 = 1284.30 */

    julian = julian_char(lsat->creation);
    if (julian < julian_char("1983-08-01"))
	    i = 0;
    else if (julian < julian_char("1984-01-15"))
	    i = 1;
    else
	    i = 2;
    lmax = Lmax[i];
    lmin = Lmin[i];

    lsat->number = 4;
    sensor_TM(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
	if (lsat->band[i].thermal) {
	    lsat->band[i].K1 = 671.62;
	    lsat->band[i].K2 = 1284.30;
	}
    }
    G_debug(1, "Landsat-4 TM");
    return;
}


/****************************************************************************
 * PURPOSE:     Store values of Landsat-5 MSS/TM
 *              March 1, 1984 to today
 *****************************************************************************/
void set_MSS5(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][4] = {
	{240., 170., 150., 127.},	/* before   April 6, 1984    */
	{268., 179., 159., 123.},	/* between                  */
	{268., 179., 148., 123.}	/* after    November 9, 1984 */
    };
    double Lmin[][4] = {
	{4., 3., 4., 2.},
	{3., 3., 4., 3.},
	{3., 3., 5., 3.}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1824., 1570., 1249., 853.4 };

    julian = julian_char(lsat->creation);
    if (julian < julian_char("1984-04-06"))
	i = 0;
    else if (julian < julian_char("1984-11-08"))
	i = 1;
    else
	i = 2;
    lmax = Lmax[i];
    lmin = Lmin[i];

    lsat->number = 5;
    sensor_MSS(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
    }
    G_debug(1, "Landsat-5 MSS");
    return;
}

void set_TM5(lsat_data * lsat)
{
    int i, j;
    double julian, *lmax, *lmin, jbuf;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    double Lmax[][7] = {
	{152.10, 296.81, 204.30, 206.20, 27.19, 15.303, 14.38},	/* before May 4, 2003 */
	{193.00, 365.00, 264.00, 221.00, 30.20, 15.303, 16.50},	/* after May 4, 2003 */
	{169.00, 333.00, 264.00, 221.00, 30.20, 15.303, 16.50}	/* after April 2, 2007 */
    };
    double Lmin[][7] = {
	{-1.52, -2.84, -1.17, -1.51, -0.37, 1.2378, -0.15},
	{-1.52, -2.84, -1.17, -1.51, -0.37, 1.2378, -0.15},
	{-1.52, -2.84, -1.17, -1.51, -0.37, 1.2378, -0.15}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1957., 1826., 1554., 1036., 215.0, 0., 80.67 };

    /* Thermal band calibration constants: K1 = 607.76   K2 = 1260.56 */

    julian = julian_char(lsat->creation);
    if (julian < julian_char("2003-05-04"))
	    i = 0;
    else if (julian < julian_char("2007-04-02"))
	    i = 1;
    else
	    i = 2;
    lmax = Lmax[i];
    lmin = Lmin[i];
    if (i == 2) {		/* in Chander, Markham and Barsi 2007 */
	julian = julian_char(lsat->date);	/* Yes, here acquisition date */
	if (julian >= julian_char("1992-01-01")) {
	    lmax[0] = 193.0;
	    lmax[1] = 365.0;
	}
    }

    jbuf = julian_char("2004-04-04");
    if (julian >= jbuf && !(lsat->flag & METADATAFILE) )
    {
        G_warning("Using QCalMin=1.0 as a NLAPS product processed after 04/04/2004");
    }
    lsat->number = 5;
    sensor_TM(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for (i = 0; i < lsat->bands; i++) {
	j = lsat->band[i].number - 1;
	if (julian >= jbuf)
	    lsat->band[i].qcalmin = 1.;
	lsat->band[i].esun = *(esun + j);
	lsat->band[i].lmax = *(lmax + j);
	lsat->band[i].lmin = *(lmin + j);
	if (lsat->band[i].thermal) {
	    lsat->band[i].K1 = 607.76;
	    lsat->band[i].K2 = 1260.56;
	}
    }
    G_debug(1, "Landsat-5 TM");
    return;
}


/****************************************************************************
 * PURPOSE:     Store values of Landsat-7 ETM+
 *              April 15, 1999 to May 31, 2003 (SLC failure)
 *****************************************************************************/
void set_ETM(lsat_data * lsat, const char gain[])
{
    double	*lmax, *lmin;

    /** USGS Calibration Parameter Files 2012 */

    /* Spectral radiances at detector */
    /* - LOW GAIN - */
    double LmaxL[][8] = {
	{297.5, 303.4, 235.5, 235.0, 47.70, 17.04, 16.60, 244.0},	/* before      July 1, 2000 */
	{293.7, 300.9, 234.4, 241.1, 47.57, 17.04, 16.54, 243.1}	/* on or after July 1, 2000 */
    };
    double LminL[][8] = {
	{-6.2, -6.0, -4.5, -4.5, -1.0, 0.0, -0.35, -5.0},
	{-6.2, -6.4, -5.0, -5.1, -1.0, 0.0, -0.35, -4.7}
    };
    /* - HIGH GAIN - */
    double LmaxH[][8] = {
	{194.3, 202.4, 158.6, 157.5, 31.76, 12.65, 10.932, 158.4},
	{191.6, 196.5, 152.9, 157.4, 31.06, 12.65, 10.80, 158.3}
    };
    double LminH[][8] = {
	{-6.2, -6.0, -4.5, -4.5, -1.0, 3.2, -0.35, -5.0},
	{-6.2, -6.4, -5.0, -5.1, -1.0, 3.2, -0.35, -4.7}
    };

    /* Solar exoatmospheric spectral irradiances */
    double esun[] = { 1969., 1840., 1551., 1044., 225.7, 0., 82.07, 1368. };

    /*  Thermal band calibration constants: K1 = 666.09   K2 = 1282.71 */

    int	k	= (julian_char(lsat->creation) < julian_char("2000-07-01")) ? 0 : 1;

    lsat->number = 7;
    sensor_ETM(lsat);

    lsat->dist_es = earth_sun(lsat->date);

    for(int i=0; i<lsat->bands; i++)
	{
		int	j	= lsat->band[i].number - 1;

		lsat->band[i].esun	= esun[j];

		if( gain[i] == 'H' || gain[i] == 'h' )
		{
			lmax = LmaxH[k];
			lmin = LminH[k];
		}
		else
		{
			lmax = LmaxL[k];
			lmin = LminL[k];
		}

		lsat->band[i].lmax = *(lmax + j);
		lsat->band[i].lmin = *(lmin + j);

		if (lsat->band[i].thermal)
		{
			lsat->band[i].K1 = 666.09;
			lsat->band[i].K2 = 1282.71;
		}
    }
    G_debug(1, "Landsat-7 ETM+");
    return;
}

/****************************************************************************
 * PURPOSE:     Store values of Landsat-8 OLI/TIRS
 *              February 14, 2013
 *****************************************************************************/
void set_OLI(lsat_data * lsat)
{
	int i, j;
	double *lmax, *lmin;

	/* Spectral radiances at detector */
	/* estimates */
	double Lmax[][11] = {	{	755.8, 770.7, 705.7, 597.7, 362.7, 91.4, 29.7, 673.3, 149.0, 22.0, 22.0	}	};
	double Lmin[][11] = {	{	-62.4, -63.6, -58.3, -49.4, -30.0, -7.5, -2.5, -55.6, -12.3, 0.1, 0.1	}	};

	/* Solar exoatmospheric spectral irradiances estimates */
	double esun[] =
	{ 2026.8, 2066.8, 1892.5, 1602.8, 972.6, 245.0, 79.7, 1805.5, 399.7, 0., 0. };

	lmax = Lmax[0];
	lmin = Lmin[0];

	lsat->number = 8;
	sensor_OLI(lsat);

	lsat->dist_es = earth_sun(lsat->date);

	for (i = 0; i < lsat->bands; i++)
	{
		j = lsat->band[i].number - 1;
		lsat->band[i].esun = *(esun + j);
		lsat->band[i].lmax = *(lmax + j);
		lsat->band[i].lmin = *(lmin + j);
		if (lsat->band[i].thermal)
		{
			lsat->band[i].K1 = (lsat->band[i].number == 10 ? 774.89 : 480.89);
			lsat->band[i].K2 = (lsat->band[i].number == 10 ? 1321.08 : 1201.14);
		}
	}
	G_debug(1, "Landsat-8 OLI/TIRS");
	return;
}


///////////////////////////////////////////////////////////
//														 //
//						landsat.c						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define PI   3.1415926535897932384626433832795
#define R2D 57.295779513082320877
#define D2R  0.017453292519943295769

/****************************************************************************
 * PURPOSE: Calibrated Digital Number to at-satellite Radiance
 *****************************************************************************/
double lsat_qcal2rad(double qcal, band_data * band)
{
    return (double)(qcal * band->gain + band->bias);
}

/****************************************************************************
 * PURPOSE: Radiance of non-thermal band to at-satellite Reflectance
 *****************************************************************************/
double lsat_rad2ref(double rad, band_data * band)
{
    return (double)(rad / band->K2);
}

/****************************************************************************
 * PURPOSE: Radiance of thermal band to at-satellite Temperature
 *****************************************************************************/
double lsat_rad2temp(double rad, band_data * band)
{
    return (double)(band->K2 / log((band->K1 / rad) + 1.0));
}

/****************************************************************************
 * PURPOSE: Some band constants
 *
 *      zenith = 90 - sun_elevation
 *      sin( sun_elevation ) = cos( sun_zenith )
 *
 *      lsat : satellite data
 *         i : band number
 *    method : level of atmospheric correction
 *   percent : percent of solar irradiance in path radiance
 *       dos : digital number of dark object for DOS
  *****************************************************************************/

#define abs(x)	(((x)>0)?(x):(-x))

void lsat_bandctes(lsat_data * lsat, int i, char method,
		   double percent, int dos, double rayleigh)
{
    double pi_d2, sin_e, cos_v, rad_sun;

    /* TAUv  = at. transmittance surface-sensor */
    /* TAUz  = at. transmittance sun-surface    */
    /* Edown = diffuse sky spectral irradiance  */
    double TAUv, TAUz, Edown;

    pi_d2 = (double)(PI * lsat->dist_es * lsat->dist_es);
    sin_e = (double)(sin(D2R * lsat->sun_elev));
    cos_v = (double)(cos(D2R * (lsat->number < 4 ? 9.2 : 8.2)));

	/** Global irradiance on the sensor.
		Radiance to reflectance coefficient, only NO thermal bands.
		K1 and K2 variables are also utilized as thermal constants
	*/
    if (lsat->band[i].thermal == 0) {
	switch (method) {
	case DOS2:
	    {
		TAUv = 1.;
		TAUz = (lsat->band[i].wavemax < 1.) ? sin_e : 1.;
		Edown = 0.;
		break;
	    }
	case DOS2b:
	    {
		TAUv = (lsat->band[i].wavemax < 1.) ? cos_v : 1.;
		TAUz = (lsat->band[i].wavemax < 1.) ? sin_e : 1.;
		Edown = 0.;
		break;
	    }
	case DOS3:
	    {
		double t;

		t = 2. / (lsat->band[i].wavemax + lsat->band[i].wavemin);
		t = 0.008569 * t * t * t * t * (1 + 0.0113 * t * t +
						0.000013 * t * t * t * t);
		TAUv = exp(-t / cos_v);
		TAUz = exp(-t / sin_e);
		Edown = rayleigh;
		break;
	    }
	case DOS4:
	    {
		double Ro =
		    (lsat->band[i].lmax - lsat->band[i].lmin) * (dos -
								 lsat->band
								 [i].qcalmin)
		    / (lsat->band[i].qcalmax - lsat->band[i].qcalmin) +
		    lsat->band[i].lmin;
		double Tv = 1.;
		double Tz = 1.;
		double Lp = 0.;

		do {
		    TAUz = Tz;
		    TAUv = Tv;
		    Lp = Ro -
			percent * TAUv * (lsat->band[i].esun * sin_e * TAUz +
					  PI * Lp) / pi_d2;
		    Tz = 1 - (4 * pi_d2 * Lp) / (lsat->band[i].esun * sin_e);
		    Tv = exp(sin_e * log(Tz) / cos_v);
		    /* G_message("TAUv = %.5f (%.5f), TAUz = %.5f (%.5f) and Edown = %.5f\n", TAUv, Tv, TAUz, Tz, PI * Lp ); */
		    /* } while( abs(TAUv - Tv) > 0.0000001 || abs(TAUz - Tz) > 0.0000001); */
		} while (TAUv != Tv && TAUz != Tz);
		TAUz = (Tz < 1. ? Tz : 1.);
		TAUv = (Tv < 1. ? Tv : 1.);
		Edown = (Lp < 0. ? 0. : PI * Lp);
		break;
	    }
	default:		/* DOS1 and Without atmospheric-correction */
	    TAUv = 1.;
	    TAUz = 1.;
	    Edown = 0.;
	    break;
	}
	rad_sun = TAUv * (lsat->band[i].esun * sin_e * TAUz + Edown) / pi_d2;
//	G_verbose_message("... TAUv = %.5f, TAUz = %.5f, Edown = %.5f\n", TAUv, TAUz, Edown);

	lsat->band[i].K1 = 0.;
	lsat->band[i].K2 = rad_sun;
    }

	/** Digital number to radiance coefficients.
		Without atmospheric calibration for thermal bands.
	*/
    lsat->band[i].gain = ((lsat->band[i].lmax - lsat->band[i].lmin) /
			  (lsat->band[i].qcalmax - lsat->band[i].qcalmin));

    if (method == UNCORRECTED || lsat->band[i].thermal) {
	/* L = G * (DN - Qmin) + Lmin
	   -> bias = Lmin - G * Qmin    */
	lsat->band[i].bias =
	    (lsat->band[i].lmin - lsat->band[i].gain * lsat->band[i].qcalmin);
    }
    else {
	if (method == CORRECTED) {
	    /* L = G * (DN - Qmin) + Lmin - Lmin
	       -> bias = - G * Qmin */
	    lsat->band[i].bias =
		-(lsat->band[i].gain * lsat->band[i].qcalmin);
	    /* Another possibility is cut when rad < 0 */
	}
	else if (method > DOS) {
	    /* L = Lsat - Lpath =
	       G * DNsat + B - (G * dark + B - p * rad_sun) =
	       G * DNsat - G * dark + p * rad_sun
	       -> bias = p * rad_sun - G * dark */
	    lsat->band[i].bias = percent * rad_sun - lsat->band[i].gain * dos;
	}
    }
}


///////////////////////////////////////////////////////////
//														 //
//						earth_sun.c						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
/*
 *  Modified from LIBNOVA-0.12
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *  Some functions in this file use the VSOP87 solution by
 *  Messrs. Bretagnon and Francou.
 *
 *  Copyright (C) 2000 - 2005 Liam Girdwood
 *  Modified to GRASS (C) 2006 E. Jorge Tizado
 */

#define RADIUS_R0 523
#define RADIUS_R1 290
#define RADIUS_R2 134
#define RADIUS_R3 20
#define RADIUS_R4 9
#define RADIUS_R5 2

struct ln_vsop
{
    double A;
    double B;
    double C;
};

double ln_calc_series(const struct ln_vsop *data, int terms, double t);

static const struct ln_vsop earth_radius_r0[RADIUS_R0] = {
    {1.00013988784, 0.00000000000, 0.00000000000},
    {0.01670699632, 3.09846350258, 6283.07584999140},
    {0.00013956024, 3.05524609456, 12566.15169998280},
    {0.00003083720, 5.19846674381, 77713.77146812050},
    {0.00001628463, 1.17387558054, 5753.38488489680},
    {0.00001575572, 2.84685214877, 7860.41939243920},
    {0.00000924799, 5.45292236722, 11506.76976979360},
    {0.00000542439, 4.56409151453, 3930.20969621960},
    {0.00000472110, 3.66100022149, 5884.92684658320},
    {0.00000328780, 5.89983686142, 5223.69391980220},
    {0.00000345969, 0.96368627272, 5507.55323866740},
    {0.00000306784, 0.29867139512, 5573.14280143310},
    {0.00000174844, 3.01193636733, 18849.22754997420},
    {0.00000243181, 4.27349530790, 11790.62908865880},
    {0.00000211836, 5.84714461348, 1577.34354244780},
    {0.00000185740, 5.02199710705, 10977.07880469900},
    {0.00000109835, 5.05510635860, 5486.77784317500},
    {0.00000098316, 0.88681311278, 6069.77675455340},
    {0.00000086500, 5.68956418946, 15720.83878487840},
    {0.00000085831, 1.27079125277, 161000.68573767410},
    {0.00000062917, 0.92177053978, 529.69096509460},
    {0.00000057056, 2.01374292245, 83996.84731811189},
    {0.00000064908, 0.27251341435, 17260.15465469040},
    {0.00000049384, 3.24501240359, 2544.31441988340},
    {0.00000055736, 5.24159799170, 71430.69561812909},
    {0.00000042520, 6.01110257982, 6275.96230299060},
    {0.00000046966, 2.57799853213, 775.52261132400},
    {0.00000038963, 5.36063832897, 4694.00295470760},
    {0.00000044666, 5.53715663816, 9437.76293488700},
    {0.00000035661, 1.67447135798, 12036.46073488820},
    {0.00000031922, 0.18368299942, 5088.62883976680},
    {0.00000031846, 1.77775642078, 398.14900340820},
    {0.00000033193, 0.24370221704, 7084.89678111520},
    {0.00000038245, 2.39255343973, 8827.39026987480},
    {0.00000028468, 1.21344887533, 6286.59896834040},
    {0.00000037486, 0.82961281844, 19651.04848109800},
    {0.00000036957, 4.90107587287, 12139.55350910680},
    {0.00000034537, 1.84270693281, 2942.46342329160},
    {0.00000026275, 4.58896863104, 10447.38783960440},
    {0.00000024596, 3.78660838036, 8429.24126646660},
    {0.00000023587, 0.26866098169, 796.29800681640},
    {0.00000027795, 1.89934427832, 6279.55273164240},
    {0.00000023927, 4.99598548145, 5856.47765911540},
    {0.00000020345, 4.65282190725, 2146.16541647520},
    {0.00000023287, 2.80783632869, 14143.49524243060},
    {0.00000022099, 1.95002636847, 3154.68708489560},
    {0.00000019509, 5.38233922479, 2352.86615377180},
    {0.00000017958, 0.19871369960, 6812.76681508600},
    {0.00000017178, 4.43322156854, 10213.28554621100},
    {0.00000016190, 5.23159323213, 17789.84561978500},
    {0.00000017315, 6.15224075188, 16730.46368959580},
    {0.00000013814, 5.18962074032, 8031.09226305840},
    {0.00000018834, 0.67280058021, 149854.40013480789},
    {0.00000018330, 2.25348717053, 23581.25817731760},
    {0.00000013639, 3.68511810757, 4705.73230754360},
    {0.00000013142, 0.65267698994, 13367.97263110660},
    {0.00000010414, 4.33285688501, 11769.85369316640},
    {0.00000009978, 4.20126336356, 6309.37416979120},
    {0.00000010170, 1.59366684542, 4690.47983635860},
    {0.00000007564, 2.62560597391, 6256.77753019160},
    {0.00000009654, 3.67583728703, 27511.46787353720},
    {0.00000006743, 0.56269927047, 3340.61242669980},
    {0.00000008743, 6.06359123461, 1748.01641306700},
    {0.00000007786, 3.67371235367, 12168.00269657460},
    {0.00000006633, 5.66149277789, 11371.70468975820},
    {0.00000007712, 0.31242577788, 7632.94325965020},
    {0.00000006586, 3.13580054586, 801.82093112380},
    {0.00000007460, 5.64758066660, 11926.25441366880},
    {0.00000006933, 2.92384586372, 6681.22485339960},
    {0.00000006805, 1.42327153767, 23013.53953958720},
    {0.00000006118, 5.13395999022, 1194.44701022460},
    {0.00000006477, 2.64986648493, 19804.82729158280},
    {0.00000005233, 4.62432817299, 6438.49624942560},
    {0.00000006147, 3.02863936662, 233141.31440436149},
    {0.00000004608, 1.72194702724, 7234.79425624200},
    {0.00000004221, 1.55697533726, 7238.67559160000},
    {0.00000005310, 2.40821524293, 11499.65622279280},
    {0.00000005128, 5.32398965690, 11513.88331679440},
    {0.00000004770, 0.25554311730, 11856.21865142450},
    {0.00000005519, 2.09089153789, 17298.18232732620},
    {0.00000005625, 4.34052903053, 90955.55169449610},
    {0.00000004578, 4.46569641570, 5746.27133789600},
    {0.00000003788, 4.90728294810, 4164.31198961300},
    {0.00000005337, 5.09957905103, 31441.67756975680},
    {0.00000003967, 1.20054555175, 1349.86740965880},
    {0.00000004005, 3.02853885902, 1059.38193018920},
    {0.00000003480, 0.76066308841, 10973.55568635000},
    {0.00000004232, 1.05485713117, 5760.49843189760},
    {0.00000004582, 3.76570026763, 6386.16862421000},
    {0.00000003335, 3.13829943354, 6836.64525283380},
    {0.00000003420, 3.00043974511, 4292.33083295040},
    {0.00000003595, 5.70703236079, 5643.17856367740},
    {0.00000003236, 4.16387400645, 9917.69687450980},
    {0.00000004154, 2.59940749519, 7058.59846131540},
    {0.00000003362, 4.54577164994, 4732.03062734340},
    {0.00000002978, 1.30561268820, 6283.14316029419},
    {0.00000002765, 0.51311975671, 26.29831979980},
    {0.00000002807, 5.66230537649, 8635.94200376320},
    {0.00000002927, 5.73787834080, 16200.77272450120},
    {0.00000003167, 1.69181759900, 11015.10647733480},
    {0.00000002598, 2.96244118358, 25132.30339996560},
    {0.00000003519, 3.62639325753, 244287.60000722769},
    {0.00000002676, 4.20727719487, 18073.70493865020},
    {0.00000002978, 1.74971565805, 6283.00853968860},
    {0.00000002287, 1.06976449088, 14314.16811304980},
    {0.00000002863, 5.92838917309, 14712.31711645800},
    {0.00000003071, 0.23793217000, 35371.88726597640},
    {0.00000002656, 0.89959301615, 12352.85260454480},
    {0.00000002415, 2.79975176800, 709.93304855830},
    {0.00000002811, 3.51513864541, 21228.39202354580},
    {0.00000001977, 2.61358297551, 951.71840625060},
    {0.00000002548, 2.47684686575, 6208.29425142410},
    {0.00000001999, 0.56090396506, 7079.37385680780},
    {0.00000002305, 1.05376463592, 22483.84857449259},
    {0.00000001855, 2.86093570752, 5216.58037280140},
    {0.00000002157, 1.31395211105, 154717.60988768269},
    {0.00000001970, 4.36931551625, 167283.76158766549},
    {0.00000001754, 2.14452400686, 6290.18939699220},
    {0.00000001628, 5.85704450617, 10984.19235169980},
    {0.00000002154, 6.03828353794, 10873.98603048040},
    {0.00000001714, 3.70158195222, 1592.59601363280},
    {0.00000001541, 6.21599512982, 23543.23050468179},
    {0.00000001602, 1.99860679677, 10969.96525769820},
    {0.00000001712, 1.34295218697, 3128.38876509580},
    {0.00000001647, 5.54948299069, 6496.37494542940},
    {0.00000001495, 5.43980459648, 155.42039943420},
    {0.00000001827, 5.91227480351, 3738.76143010800},
    {0.00000001726, 2.16765465036, 10575.40668294180},
    {0.00000001532, 5.35683107063, 13521.75144159140},
    {0.00000001824, 1.66056145084, 39302.09696219600},
    {0.00000001605, 1.90930973224, 6133.51265285680},
    {0.00000001282, 2.46013372544, 13916.01910964160},
    {0.00000001211, 4.41360631550, 3894.18182954220},
    {0.00000001394, 1.77801929250, 9225.53927328300},
    {0.00000001571, 4.95512957606, 25158.60171976540},
    {0.00000001205, 1.19212756308, 3.52311834900},
    {0.00000001132, 2.69830084955, 6040.34724601740},
    {0.00000001504, 5.77577388271, 18209.33026366019},
    {0.00000001393, 1.62625077326, 5120.60114558360},
    {0.00000001081, 2.93726744446, 17256.63153634140},
    {0.00000001232, 0.71651766504, 143571.32428481648},
    {0.00000001087, 0.99769687961, 955.59974160860},
    {0.00000001068, 5.28472576591, 65147.61976813770},
    {0.00000001169, 3.11663802316, 14945.31617355440},
    {0.00000000975, 5.10887260780, 6172.86952877200},
    {0.00000001202, 4.02992510403, 553.56940284240},
    {0.00000000979, 2.00000879106, 15110.46611986620},
    {0.00000000962, 4.02380771400, 6282.09552892320},
    {0.00000000999, 3.62643002790, 6262.30045449900},
    {0.00000001030, 5.84987815239, 213.29909543800},
    {0.00000001014, 2.84227679965, 8662.24032356300},
    {0.00000001185, 1.51330629149, 17654.78053974960},
    {0.00000000967, 2.67081017562, 5650.29211067820},
    {0.00000001222, 2.65423784904, 88860.05707098669},
    {0.00000000986, 2.36212814824, 6206.80977871580},
    {0.00000001034, 0.13634950642, 11712.95531823080},
    {0.00000001103, 3.08477302937, 43232.30665841560},
    {0.00000000781, 2.53374971725, 16496.36139620240},
    {0.00000001019, 3.04569392376, 6037.24420376200},
    {0.00000000795, 5.80662989126, 5230.80746680300},
    {0.00000000813, 3.57702871938, 10177.25767953360},
    {0.00000000962, 5.31470594766, 6284.05617105960},
    {0.00000000717, 5.95797471837, 12559.03815298200},
    {0.00000000967, 2.74413738053, 6244.94281435360},
    {0.00000000921, 0.10160160830, 29088.81141598500},
    {0.00000000719, 5.91788189939, 4136.91043351620},
    {0.00000000688, 3.89489045092, 1589.07289528380},
    {0.00000000772, 4.05505380285, 6127.65545055720},
    {0.00000000706, 5.49323197725, 22003.91463486980},
    {0.00000000665, 1.60002747134, 11087.28512591840},
    {0.00000000690, 4.50539825729, 426.59819087600},
    {0.00000000854, 3.26104645060, 20426.57109242200},
    {0.00000000656, 4.32410182940, 16858.48253293320},
    {0.00000000840, 2.59572585212, 28766.92442448400},
    {0.00000000686, 0.61944033771, 11403.67699557500},
    {0.00000000700, 3.40901412473, 7.11354700080},
    {0.00000000728, 0.04050185963, 5481.25491886760},
    {0.00000000653, 1.03869451230, 6062.66320755260},
    {0.00000000559, 4.79221805695, 20199.09495963300},
    {0.00000000633, 5.70229959167, 45892.73043315699},
    {0.00000000591, 6.10986487621, 9623.68827669120},
    {0.00000000520, 3.62310356479, 5333.90024102160},
    {0.00000000602, 5.58381898589, 10344.29506538580},
    {0.00000000496, 2.21027756314, 1990.74501704100},
    {0.00000000691, 1.96733114988, 12416.58850284820},
    {0.00000000640, 1.59062417043, 18319.53658487960},
    {0.00000000625, 3.82358168221, 13517.87010623340},
    {0.00000000475, 1.17025904180, 12569.67481833180},
    {0.00000000660, 5.08498512995, 283.85931886520},
    {0.00000000664, 4.50029469969, 47162.51635463520},
    {0.00000000569, 0.16318535463, 17267.26820169119},
    {0.00000000568, 3.86100969474, 6076.89030155420},
    {0.00000000462, 0.26368763517, 4590.91018048900},
    {0.00000000535, 4.83225423196, 18422.62935909819},
    {0.00000000466, 0.75873879417, 7342.45778018060},
    {0.00000000541, 3.07212190556, 226858.23855437008},
    {0.00000000610, 1.53597089605, 33019.02111220460},
    {0.00000000617, 2.62356328726, 11190.37790013700},
    {0.00000000548, 4.55798855803, 18875.52586977400},
    {0.00000000633, 4.60110281228, 66567.48586525429},
    {0.00000000587, 5.78087907808, 632.78373931320},
    {0.00000000603, 5.38458554802, 316428.22867391503},
    {0.00000000525, 5.01522072363, 12132.43996210600},
    {0.00000000469, 0.59975173763, 21954.15760939799},
    {0.00000000548, 3.50627043672, 17253.04110768959},
    {0.00000000502, 0.98804327589, 11609.86254401220},
    {0.00000000568, 1.98497313089, 7668.63742494250},
    {0.00000000482, 1.62460405687, 12146.66705610760},
    {0.00000000391, 3.68718382972, 18052.92954315780},
    {0.00000000457, 3.77214896610, 156137.47598479928},
    {0.00000000401, 5.29221540240, 15671.08175940660},
    {0.00000000469, 1.80963351735, 12562.62858163380},
    {0.00000000514, 3.37031288919, 20597.24396304120},
    {0.00000000452, 5.66811219778, 10454.50138660520},
    {0.00000000375, 4.98528185039, 9779.10867612540},
    {0.00000000523, 0.97215560834, 155427.54293624099},
    {0.00000000403, 5.13948189770, 1551.04522264800},
    {0.00000000372, 3.69883738807, 9388.00590941520},
    {0.00000000367, 4.43875659833, 4535.05943692440},
    {0.00000000406, 4.20863156497, 12592.45001978260},
    {0.00000000362, 2.55099560446, 242.72860397400},
    {0.00000000471, 4.61907324819, 5436.99301524020},
    {0.00000000388, 4.96020928400, 24356.78078864160},
    {0.00000000441, 5.83872966262, 3496.03282613400},
    {0.00000000349, 6.16307810648, 19800.94595622480},
    {0.00000000356, 0.23819081240, 5429.87946823940},
    {0.00000000346, 5.60809622572, 2379.16447357160},
    {0.00000000380, 2.72105213132, 11933.36796066960},
    {0.00000000432, 0.24215988572, 17996.03116822220},
    {0.00000000378, 5.22516848076, 7477.52286021600},
    {0.00000000337, 5.10885555836, 5849.36411211460},
    {0.00000000315, 0.57827745123, 10557.59416082380},
    {0.00000000318, 4.49949007320, 3634.62102451840},
    {0.00000000323, 1.55850824803, 10440.27429260360},
    {0.00000000314, 5.77154773334, 20.77539549240},
    {0.00000000303, 2.34615580398, 4686.88940770680},
    {0.00000000414, 5.93237602310, 51092.72605085480},
    {0.00000000362, 2.17561997119, 28237.23345938940},
    {0.00000000288, 0.18377405421, 13095.84266507740},
    {0.00000000277, 5.12952205030, 13119.72110282519},
    {0.00000000325, 6.18608287927, 6268.84875598980},
    {0.00000000273, 0.30522428863, 23141.55838292460},
    {0.00000000267, 5.76152585786, 5966.68398033480},
    {0.00000000345, 2.94246040875, 36949.23080842420},
    {0.00000000253, 5.20994580359, 24072.92146977640},
    {0.00000000342, 5.76212804329, 16460.33352952499},
    {0.00000000307, 6.01039067183, 22805.73556599360},
    {0.00000000261, 2.00304796059, 6148.01076995600},
    {0.00000000238, 5.08241964961, 6915.85958930460},
    {0.00000000249, 2.94762789744, 135.06508003540},
    {0.00000000306, 3.89765478921, 10988.80815753500},
    {0.00000000308, 0.05451027736, 4701.11650170840},
    {0.00000000319, 2.95712862064, 163096.18036118349},
    {0.00000000272, 2.07967681309, 4804.20927592700},
    {0.00000000209, 4.43768461442, 6546.15977336420},
    {0.00000000217, 0.73691592312, 6303.85124548380},
    {0.00000000203, 0.32033085531, 25934.12433108940},
    {0.00000000205, 5.22936478995, 20995.39296644940},
    {0.00000000213, 0.20671418919, 28286.99048486120},
    {0.00000000197, 0.48286131290, 16737.57723659660},
    {0.00000000230, 6.06567392849, 6287.00800325450},
    {0.00000000219, 1.29194216300, 5326.78669402080},
    {0.00000000201, 1.74700937253, 22743.40937951640},
    {0.00000000207, 4.45440927276, 6279.48542133960},
    {0.00000000269, 6.05640445030, 64471.99124174489},
    {0.00000000190, 0.99261116842, 29296.61538957860},
    {0.00000000194, 3.82656562755, 419.48464387520},
    {0.00000000262, 5.26961924126, 522.57741809380},
    {0.00000000210, 4.68618183158, 6254.62666252360},
    {0.00000000197, 2.80624554186, 4933.20844033260},
    {0.00000000252, 4.36220154620, 40879.44050464380},
    {0.00000000261, 1.07241516738, 55022.93574707440},
    {0.00000000233, 5.41751014958, 39609.65458316560},
    {0.00000000185, 4.14324541379, 5642.19824260920},
    {0.00000000247, 3.44855612987, 6702.56049386660},
    {0.00000000205, 4.04424043226, 536.80451209540},
    {0.00000000191, 3.15807087926, 16723.35014259500},
    {0.00000000222, 5.16259496507, 23539.70738633280},
    {0.00000000180, 4.56214752149, 6489.26139842860},
    {0.00000000227, 0.60156339452, 5905.70224207560},
    {0.00000000170, 0.93185903228, 16062.18452611680},
    {0.00000000159, 0.92751013112, 23937.85638974100},
    {0.00000000157, 4.69607868164, 6805.65326808520},
    {0.00000000218, 0.85533373430, 16627.37091537720},
    {0.00000000169, 0.94641052064, 3097.88382272579},
    {0.00000000207, 4.88410451334, 6286.66627864320},
    {0.00000000160, 4.95943826819, 10021.83728009940},
    {0.00000000175, 6.12762824563, 239424.39025435288},
    {0.00000000173, 3.13887234973, 6179.98307577280},
    {0.00000000157, 3.62822057807, 18451.07854656599},
    {0.00000000206, 5.74617821138, 3646.35037735440},
    {0.00000000157, 4.67695912207, 6709.67404086740},
    {0.00000000146, 3.09506069745, 4907.30205014560},
    {0.00000000165, 2.27139128760, 10660.68693504240},
    {0.00000000144, 3.96947747592, 6019.99192661860},
    {0.00000000171, 5.91302216729, 6058.73105428950},
    {0.00000000144, 2.13155655120, 26084.02180621620},
    {0.00000000151, 0.67417383565, 2388.89402044920},
    {0.00000000196, 1.67718461229, 2107.03450754240},
    {0.00000000146, 5.10373877968, 10770.89325626180},
    {0.00000000187, 1.23915444627, 19402.79695281660},
    {0.00000000137, 1.26247412216, 12566.21901028560},
    {0.00000000191, 5.03547476279, 263.08392337280},
    {0.00000000137, 3.52825454595, 639.89728631400},
    {0.00000000135, 0.73840670927, 5017.50837136500},
    {0.00000000164, 2.39195095081, 6357.85744855870},
    {0.00000000168, 0.05515907462, 9380.95967271720},
    {0.00000000161, 1.15721259392, 26735.94526221320},
    {0.00000000144, 1.76097645199, 5888.44996493220},
    {0.00000000131, 2.51859277344, 6599.46771964800},
    {0.00000000142, 2.43802911123, 5881.40372823420},
    {0.00000000159, 5.90325893762, 6281.59137728310},
    {0.00000000151, 3.72338532519, 12669.24447420140},
    {0.00000000132, 2.38417741883, 6525.80445396540},
    {0.00000000127, 0.00254936441, 10027.90319572920},
    {0.00000000148, 2.85102145528, 6418.14093002680},
    {0.00000000143, 5.74460279560, 26087.90314157420},
    {0.00000000172, 0.41289962240, 174242.46596404970},
    {0.00000000136, 4.15497742275, 6311.52503745920},
    {0.00000000170, 5.98194913129, 327574.51427678125},
    {0.00000000136, 2.48430537541, 13341.67431130680},
    {0.00000000149, 0.33002271275, 245.83164622940},
    {0.00000000165, 2.49667924600, 58953.14544329400},
    {0.00000000123, 1.67328384813, 32217.20018108080},
    {0.00000000123, 3.45660563754, 6277.55292568400},
    {0.00000000117, 0.86065134175, 6245.04817735560},
    {0.00000000149, 5.61358281003, 5729.50644714900},
    {0.00000000128, 0.71204006448, 103.09277421860},
    {0.00000000159, 2.43166592149, 221995.02880149524},
    {0.00000000137, 1.70657709200, 12566.08438968000},
    {0.00000000129, 2.80667872683, 6016.46880826960},
    {0.00000000113, 3.58302904101, 25685.87280280800},
    {0.00000000109, 3.26403795962, 6819.88036208680},
    {0.00000000122, 0.34120688202, 1162.47470440780},
    {0.00000000106, 1.59721172719, 17782.73207278420},
    {0.00000000144, 2.28891651774, 12489.88562870720},
    {0.00000000137, 5.82029768354, 44809.65020086340},
    {0.00000000134, 1.26539983018, 5331.35744374080},
    {0.00000000103, 5.96518130595, 6321.10352262720},
    {0.00000000109, 0.33808549034, 11300.58422135640},
    {0.00000000129, 5.89187277190, 12029.34718788740},
    {0.00000000122, 5.77325634636, 11919.14086666800},
    {0.00000000107, 6.24998989350, 77690.75950573849},
    {0.00000000107, 1.00535580713, 77736.78343050249},
    {0.00000000115, 5.86963518266, 12721.57209941700},
    {0.00000000102, 5.66283467269, 5540.08578945880},
    {0.00000000143, 0.24122178432, 4214.06901508480},
    {0.00000000143, 0.88529649733, 7576.56007357400},
    {0.00000000107, 2.92124030351, 31415.37924995700},
    {0.00000000100, 5.99485644501, 4061.21921539440},
    {0.00000000103, 2.41941934525, 5547.19933645960},
    {0.00000000104, 4.44106051277, 2118.76386037840},
    {0.00000000110, 0.37559635174, 5863.59120611620},
    {0.00000000124, 2.55619029611, 12539.85338018300},
    {0.00000000110, 3.66952094465, 238004.52415723629},
    {0.00000000112, 4.32512422724, 97238.62754448749},
    {0.00000000120, 1.26895630075, 12043.57428188900},
    {0.00000000097, 5.42612959752, 7834.12107263940},
    {0.00000000094, 2.56461130309, 19004.64794940840},
    {0.00000000105, 5.68272475301, 16522.65971600220},
    {0.00000000117, 3.65425622684, 34520.30930938080},
    {0.00000000108, 1.24206843948, 84672.47584450469},
    {0.00000000098, 0.13589994287, 11080.17157891760},
    {0.00000000097, 2.46722096722, 71980.63357473118},
    {0.00000000095, 5.36958330451, 6288.59877429880},
    {0.00000000096, 0.20796618776, 18139.29450141590},
    {0.00000000111, 5.01961920313, 11823.16163945020},
    {0.00000000090, 2.72355843779, 26880.31981303260},
    {0.00000000099, 0.90164266199, 18635.92845453620},
    {0.00000000126, 4.78722177847, 305281.94307104882},
    {0.00000000124, 5.00979495566, 172146.97134054029},
    {0.00000000090, 4.50544881196, 40077.61957352000},
    {0.00000000104, 5.63679680710, 2787.04302385740},
    {0.00000000091, 5.43564326147, 6272.03014972750},
    {0.00000000100, 2.00639461597, 12323.42309600880},
    {0.00000000117, 2.35555589778, 83286.91426955358},
    {0.00000000105, 2.59824000109, 30666.15495843280},
    {0.00000000090, 2.35779490026, 12491.37010141550},
    {0.00000000089, 3.57152453732, 11720.06886523160},
    {0.00000000095, 5.67015349858, 14919.01785375460},
    {0.00000000087, 1.86043406047, 27707.54249429480},
    {0.00000000106, 3.04150600352, 22345.26037610820},
    {0.00000000082, 5.58298993353, 10241.20229116720},
    {0.00000000083, 3.10607039533, 36147.40987730040},
    {0.00000000094, 5.47749711149, 9924.81042151060},
    {0.00000000082, 4.71988314145, 15141.39079431200},
    {0.00000000096, 3.89073946348, 6379.05507720920},
    {0.00000000110, 4.92131611151, 5621.84292321040},
    {0.00000000110, 4.89978492291, 72140.62866668739},
    {0.00000000097, 5.20764563059, 6303.43116939020},
    {0.00000000085, 1.61269222311, 33326.57873317420},
    {0.00000000093, 1.32651591333, 23020.65308658799},
    {0.00000000090, 0.57733016380, 26482.17080962440},
    {0.00000000078, 3.99588630754, 11293.47067435560},
    {0.00000000106, 3.92012705073, 62883.35513951360},
    {0.00000000098, 2.94397773524, 316.39186965660},
    {0.00000000076, 3.96310417608, 29026.48522950779},
    {0.00000000098, 0.95914722366, 48739.85989708300},
    {0.00000000078, 1.97068528043, 90279.92316810328},
    {0.00000000076, 0.23027966596, 21424.46664430340},
    {0.00000000079, 1.46227790922, 8982.81066930900},
    {0.00000000078, 2.28840998832, 266.60704172180},
    {0.00000000071, 1.51940765590, 33794.54372352860},
    {0.00000000076, 0.22880641443, 57375.80190084620},
    {0.00000000097, 0.39449562097, 24279.10701821359},
    {0.00000000075, 2.77638584795, 12964.30070339100},
    {0.00000000077, 5.18846946344, 11520.99686379520},
    {0.00000000068, 0.50006599129, 4274.51831083240},
    {0.00000000075, 2.07323762803, 15664.03552270859},
    {0.00000000077, 0.46665178780, 16207.88627150200},
    {0.00000000081, 4.10452219483, 161710.61878623239},
    {0.00000000071, 3.91415328513, 7875.67186362420},
    {0.00000000081, 0.91938383406, 74.78159856730},
    {0.00000000083, 4.69916218791, 23006.42599258639},
    {0.00000000069, 0.98999300277, 6393.28217121080},
    {0.00000000065, 5.41938745446, 28628.33622609960},
    {0.00000000073, 2.45564765251, 15508.61512327440},
    {0.00000000065, 3.02336771694, 5959.57043333400},
    {0.00000000064, 0.18375587635, 1066.49547719000},
    {0.00000000080, 5.81239171612, 12341.80690428090},
    {0.00000000066, 2.15105504851, 38.02767263580},
    {0.00000000067, 5.14047250153, 9814.60410029120},
    {0.00000000062, 2.43313614978, 10138.10951694860},
    {0.00000000068, 2.24442548639, 24383.07910844140},
    {0.00000000078, 1.39649333997, 9411.46461508720},
    {0.00000000059, 4.95362151577, 35707.71008290740},
    {0.00000000073, 1.35229143121, 5327.47610838280},
    {0.00000000057, 3.16018882154, 5490.30096152400},
    {0.00000000072, 5.91833527334, 10881.09957748120},
    {0.00000000067, 0.66414713064, 29864.33402730900},
    {0.00000000065, 0.30352816135, 7018.95236352320},
    {0.00000000059, 5.36231868425, 10239.58386601080},
    {0.00000000056, 3.22196331515, 2636.72547263700},
    {0.00000000068, 5.32086226658, 3116.65941225980},
    {0.00000000059, 1.63156134967, 61306.01159706580},
    {0.00000000054, 4.29491690425, 21947.11137270000},
    {0.00000000070, 0.29271565928, 6528.90749622080},
    {0.00000000057, 5.89190132575, 34513.26307268280},
    {0.00000000054, 2.51856815404, 6279.19451463340},
    {0.00000000074, 1.38235845304, 9967.45389998160},
    {0.00000000054, 0.92276712152, 6286.95718534940},
    {0.00000000070, 5.00933012248, 6453.74872061060},
    {0.00000000053, 3.86543309344, 32370.97899156560},
    {0.00000000055, 4.51794544854, 34911.41207609100},
    {0.00000000063, 5.41479412056, 11502.83761653050},
    {0.00000000063, 2.34416220742, 11510.70192305670},
    {0.00000000056, 0.91310629913, 9910.58332750900},
    {0.00000000067, 4.03308763854, 34596.36465465240},
    {0.00000000060, 5.57024703495, 5756.90800324580},
    {0.00000000072, 2.80863088166, 10866.87248347960},
    {0.00000000066, 6.12047940728, 12074.48840752400},
    {0.00000000051, 2.59519527563, 11396.56344857420},
    {0.00000000062, 5.14746754396, 25287.72379939980},
    {0.00000000054, 2.50994032776, 5999.21653112620},
    {0.00000000051, 4.51195861837, 29822.78323632420},
    {0.00000000059, 0.44167237876, 250570.67585721909},
    {0.00000000051, 3.68849066760, 6262.72053059260},
    {0.00000000049, 0.54704693048, 22594.05489571199},
    {0.00000000065, 2.38423614501, 52670.06959330260},
    {0.00000000069, 5.34363738671, 66813.56483573320},
    {0.00000000056, 2.67216180349, 17892.93839400359},
    {0.00000000049, 4.18361320516, 18606.49894600020},
    {0.00000000055, 0.83886167974, 20452.86941222180},
    {0.00000000050, 1.46327331958, 37455.72649597440},
    {0.00000000058, 3.34847975377, 33990.61834428620},
    {0.00000000065, 1.45522693982, 76251.32777062019},
    {0.00000000056, 2.35650664200, 37724.75341974820},
    {0.00000000048, 1.80689447612, 206.18554843720},
    {0.00000000056, 3.84224878744, 5483.25472482600},
    {0.00000000053, 0.17334326094, 77717.29458646949},
    {0.00000000053, 0.79879700631, 77710.24834977149},
    {0.00000000047, 0.43240779709, 735.87651353180},
    {0.00000000053, 4.58786566028, 11616.97609101300},
    {0.00000000048, 6.20230111054, 4171.42553661380},
    {0.00000000052, 2.91719053030, 6993.00889854970},
    {0.00000000057, 3.42008310383, 50317.20343953080},
    {0.00000000048, 0.12356889012, 13362.44970679920},
    {0.00000000060, 5.52056066934, 949.17560896980},
    {0.00000000045, 3.37963782356, 10763.77970926100},
    {0.00000000047, 5.50958184902, 12779.45079542080},
    {0.00000000052, 5.42770349015, 310145.15282392364},
    {0.00000000061, 2.93237974631, 5791.41255753260},
    {0.00000000044, 2.87440620802, 8584.66166590080},
    {0.00000000046, 4.03141796560, 10667.80048204320},
    {0.00000000044, 1.21579107625, 6272.43918464160},
    {0.00000000047, 2.57670800912, 11492.54267579200},
    {0.00000000044, 3.62570223167, 63658.87775083760},
    {0.00000000051, 0.84531181151, 12345.73905754400},
    {0.00000000046, 1.17584556517, 149.56319713460},
    {0.00000000043, 0.01524970172, 37853.87549938260},
    {0.00000000043, 0.79038834934, 640.87760738220},
    {0.00000000044, 2.22554419931, 6293.71251534120},
    {0.00000000049, 1.01528394907, 149144.46708624958},
    {0.00000000041, 3.27146326065, 8858.31494432060},
    {0.00000000045, 3.03765521215, 65236.22129328540},
    {0.00000000058, 5.45843180927, 1975.49254585600},
    {0.00000000041, 1.32190847146, 2547.83753823240},
    {0.00000000047, 3.67626039848, 28313.28880466100},
    {0.00000000047, 6.21438985953, 10991.30589870060},
    {0.00000000040, 2.37237751212, 8273.82086703240},
    {0.00000000056, 1.09773690181, 77376.20102240759},
    {0.00000000040, 2.35698541041, 2699.73481931760},
    {0.00000000043, 5.28030897946, 17796.95916678580},
    {0.00000000054, 2.59175932091, 22910.44676536859},
    {0.00000000055, 0.07988985505, 83467.15635301729},
    {0.00000000041, 4.47510694062, 5618.31980486140},
    {0.00000000040, 1.35670430524, 27177.85152920020},
    {0.00000000041, 2.48011323946, 6549.68289171320},
    {0.00000000050, 2.56387920528, 82576.98122099529},
    {0.00000000042, 4.78798367468, 7856.89627409019},
    {0.00000000047, 2.75482175292, 18202.21671665939},
    {0.00000000039, 1.97008298629, 24491.42579258340},
    {0.00000000042, 4.04346599946, 7863.94251078820},
    {0.00000000039, 3.01033936420, 853.19638175200},
    {0.00000000038, 0.49178679251, 38650.17350619900},
    {0.00000000044, 1.35931241699, 21393.54196985760},
    {0.00000000036, 4.86047906533, 4157.19844261220},
    {0.00000000043, 5.64354880978, 1062.90504853820},
    {0.00000000039, 3.92736779879, 3903.91137641980},
    {0.00000000040, 5.39694918320, 9498.21223063460},
    {0.00000000043, 2.40863861919, 29424.63423291600},
    {0.00000000046, 2.08022244271, 12573.26524698360},
    {0.00000000050, 6.15760345261, 78051.34191383338},
};

static const struct ln_vsop earth_radius_r1[RADIUS_R1] = {
    {0.00103018607, 1.10748968172, 6283.07584999140},
    {0.00001721238, 1.06442300386, 12566.15169998280},
    {0.00000702217, 3.14159265359, 0.00000000000},
    {0.00000032345, 1.02168583254, 18849.22754997420},
    {0.00000030801, 2.84358443952, 5507.55323866740},
    {0.00000024978, 1.31906570344, 5223.69391980220},
    {0.00000018487, 1.42428709076, 1577.34354244780},
    {0.00000010077, 5.91385248388, 10977.07880469900},
    {0.00000008635, 0.27158192945, 5486.77784317500},
    {0.00000008654, 1.42046854427, 6275.96230299060},
    {0.00000005069, 1.68613408916, 5088.62883976680},
    {0.00000004985, 6.01402338185, 6286.59896834040},
    {0.00000004667, 5.98749245692, 529.69096509460},
    {0.00000004395, 0.51800423445, 4694.00295470760},
    {0.00000003870, 4.74932206877, 2544.31441988340},
    {0.00000003755, 5.07053801166, 796.29800681640},
    {0.00000004100, 1.08424801084, 9437.76293488700},
    {0.00000003518, 0.02290216978, 83996.84731811189},
    {0.00000003436, 0.94937503872, 71430.69561812909},
    {0.00000003221, 6.15628775321, 2146.16541647520},
    {0.00000003418, 5.41151581880, 775.52261132400},
    {0.00000002863, 5.48433323746, 10447.38783960440},
    {0.00000002525, 0.24296913555, 398.14900340820},
    {0.00000002205, 4.94892172085, 6812.76681508600},
    {0.00000002186, 0.41991932164, 8031.09226305840},
    {0.00000002828, 3.41986300734, 2352.86615377180},
    {0.00000002554, 6.13241770582, 6438.49624942560},
    {0.00000001932, 5.31374587091, 8429.24126646660},
    {0.00000002427, 3.09118902115, 4690.47983635860},
    {0.00000001730, 1.53685999718, 4705.73230754360},
    {0.00000002250, 3.68836395620, 7084.89678111520},
    {0.00000002094, 1.28169060400, 1748.01641306700},
    {0.00000001483, 3.22226346483, 7234.79425624200},
    {0.00000001434, 0.81293662216, 14143.49524243060},
    {0.00000001754, 3.22883705112, 6279.55273164240},
    {0.00000001583, 4.09815978783, 11499.65622279280},
    {0.00000001575, 5.53890314149, 3154.68708489560},
    {0.00000001847, 1.82041234937, 7632.94325965020},
    {0.00000001499, 3.63177937611, 11513.88331679440},
    {0.00000001337, 4.64442556061, 6836.64525283380},
    {0.00000001275, 2.69329661394, 1349.86740965880},
    {0.00000001348, 6.15284035323, 5746.27133789600},
    {0.00000001126, 3.35676107739, 17789.84561978500},
    {0.00000001470, 3.65282991735, 1194.44701022460},
    {0.00000001101, 4.49747427670, 4292.33083295040},
    {0.00000001168, 2.58033028504, 13367.97263110660},
    {0.00000001236, 5.64980098028, 5760.49843189760},
    {0.00000000985, 0.65326301914, 5856.47765911540},
    {0.00000000928, 2.32555018290, 10213.28554621100},
    {0.00000001073, 5.82672338169, 12036.46073488820},
    {0.00000000918, 0.76907130762, 16730.46368959580},
    {0.00000000876, 1.50335727807, 11926.25441366880},
    {0.00000001023, 5.62071200879, 6256.77753019160},
    {0.00000000853, 0.65678134630, 155.42039943420},
    {0.00000000802, 4.10519132094, 951.71840625060},
    {0.00000000859, 1.42880883564, 5753.38488489680},
    {0.00000000992, 1.14238001610, 1059.38193018920},
    {0.00000000814, 1.63584008733, 6681.22485339960},
    {0.00000000664, 4.55039663226, 5216.58037280140},
    {0.00000000627, 1.50782904323, 5643.17856367740},
    {0.00000000644, 4.19480024859, 6040.34724601740},
    {0.00000000590, 6.18371704849, 4164.31198961300},
    {0.00000000635, 0.52423584770, 6290.18939699220},
    {0.00000000650, 0.97935492869, 25132.30339996560},
    {0.00000000568, 2.30121525349, 10973.55568635000},
    {0.00000000549, 5.26737827342, 3340.61242669980},
    {0.00000000547, 2.20143332641, 1592.59601363280},
    {0.00000000526, 0.92464258271, 11371.70468975820},
    {0.00000000493, 5.91036281399, 3894.18182954220},
    {0.00000000483, 1.66005711540, 12168.00269657460},
    {0.00000000514, 3.59683072524, 10969.96525769820},
    {0.00000000516, 3.97164781773, 17298.18232732620},
    {0.00000000529, 5.03538677680, 9917.69687450980},
    {0.00000000487, 2.50544745305, 6127.65545055720},
    {0.00000000419, 4.05235655996, 10984.19235169980},
    {0.00000000538, 5.54081539813, 553.56940284240},
    {0.00000000402, 2.16859478359, 7860.41939243920},
    {0.00000000552, 2.32219865498, 11506.76976979360},
    {0.00000000367, 3.39145698451, 6496.37494542940},
    {0.00000000360, 5.34467204596, 7079.37385680780},
    {0.00000000334, 3.61346365667, 11790.62908865880},
    {0.00000000454, 0.28755421898, 801.82093112380},
    {0.00000000419, 3.69613970002, 10575.40668294180},
    {0.00000000319, 0.30793759304, 16200.77272450120},
    {0.00000000376, 5.81560210508, 7058.59846131540},
    {0.00000000364, 1.08425056923, 6309.37416979120},
    {0.00000000294, 4.54798604178, 11856.21865142450},
    {0.00000000290, 1.26451946335, 8635.94200376320},
    {0.00000000394, 4.15683669084, 26.29831979980},
    {0.00000000260, 5.09424572996, 10177.25767953360},
    {0.00000000241, 2.25766000302, 11712.95531823080},
    {0.00000000239, 1.06936978753, 242.72860397400},
    {0.00000000276, 3.44260568764, 5884.92684658320},
    {0.00000000255, 5.38496803122, 21228.39202354580},
    {0.00000000307, 4.24313885601, 3738.76143010800},
    {0.00000000213, 3.44661200485, 213.29909543800},
    {0.00000000198, 0.69427265195, 1990.74501704100},
    {0.00000000195, 5.16563409007, 12352.85260454480},
    {0.00000000213, 3.89937836808, 13916.01910964160},
    {0.00000000214, 4.00445200772, 5230.80746680300},
    {0.00000000184, 5.59805976614, 6283.14316029419},
    {0.00000000184, 2.85275392124, 7238.67559160000},
    {0.00000000179, 2.54259058252, 14314.16811304980},
    {0.00000000236, 5.58826125715, 6069.77675455340},
    {0.00000000189, 2.72689937708, 6062.66320755260},
    {0.00000000184, 6.04216273598, 6283.00853968860},
    {0.00000000225, 1.66128561344, 4732.03062734340},
    {0.00000000230, 3.62591335086, 6284.05617105960},
    {0.00000000172, 0.97566476085, 3930.20969621960},
    {0.00000000162, 2.19467339429, 18073.70493865020},
    {0.00000000215, 1.04672844028, 3496.03282613400},
    {0.00000000182, 5.17782354566, 17253.04110768959},
    {0.00000000167, 2.17754938066, 6076.89030155420},
    {0.00000000167, 4.75672473773, 17267.26820169119},
    {0.00000000149, 0.80944185798, 709.93304855830},
    {0.00000000149, 0.17584214812, 9779.10867612540},
    {0.00000000192, 5.00680790235, 11015.10647733480},
    {0.00000000141, 4.38420380014, 4136.91043351620},
    {0.00000000158, 4.60969054283, 9623.68827669120},
    {0.00000000133, 3.30507062245, 154717.60988768269},
    {0.00000000166, 6.13191098325, 3.52311834900},
    {0.00000000181, 1.60715321141, 7.11354700080},
    {0.00000000150, 5.28136702046, 13517.87010623340},
    {0.00000000142, 0.49788089569, 25158.60171976540},
    {0.00000000124, 6.03440459813, 9225.53927328300},
    {0.00000000124, 0.99251562639, 65147.61976813770},
    {0.00000000128, 1.92032744711, 22483.84857449259},
    {0.00000000124, 3.99739675184, 4686.88940770680},
    {0.00000000121, 2.37814805239, 167283.76158766549},
    {0.00000000123, 5.62315112940, 5642.19824260920},
    {0.00000000117, 5.81755956156, 12569.67481833180},
    {0.00000000157, 3.40236948518, 16496.36139620240},
    {0.00000000130, 2.10499918142, 1589.07289528380},
    {0.00000000116, 0.55839966736, 5849.36411211460},
    {0.00000000123, 5.81645568991, 6282.09552892320},
    {0.00000000110, 0.42176497674, 6172.86952877200},
    {0.00000000150, 4.26279600865, 3128.38876509580},
    {0.00000000106, 2.27436561182, 5429.87946823940},
    {0.00000000114, 1.52894564202, 12559.03815298200},
    {0.00000000121, 0.39459045915, 12132.43996210600},
    {0.00000000104, 2.41845930933, 426.59819087600},
    {0.00000000109, 5.82786999856, 16858.48253293320},
    {0.00000000102, 4.46626484910, 23543.23050468179},
    {0.00000000100, 2.93812275274, 4535.05943692440},
    {0.00000000097, 3.97935904984, 6133.51265285680},
    {0.00000000098, 0.87616810121, 6525.80445396540},
    {0.00000000110, 6.22339014386, 12146.66705610760},
    {0.00000000098, 3.17344332543, 10440.27429260360},
    {0.00000000096, 2.44128701699, 3097.88382272579},
    {0.00000000099, 5.75642493267, 7342.45778018060},
    {0.00000000090, 0.18984343165, 13119.72110282519},
    {0.00000000099, 5.58884724219, 2388.89402044920},
    {0.00000000091, 6.04278320182, 20426.57109242200},
    {0.00000000080, 1.29028142103, 5650.29211067820},
    {0.00000000086, 3.94529200528, 10454.50138660520},
    {0.00000000085, 1.92836879835, 29088.81141598500},
    {0.00000000076, 2.70726317966, 143571.32428481648},
    {0.00000000091, 5.63859073351, 8827.39026987480},
    {0.00000000076, 1.80783856698, 28286.99048486120},
    {0.00000000075, 3.40858032804, 5481.25491886760},
    {0.00000000070, 4.53719487231, 17256.63153634140},
    {0.00000000089, 1.10064490942, 11769.85369316640},
    {0.00000000066, 2.78384937771, 536.80451209540},
    {0.00000000068, 3.88199295043, 17260.15465469040},
    {0.00000000088, 3.88075269535, 7477.52286021600},
    {0.00000000061, 6.17558202197, 11087.28512591840},
    {0.00000000060, 4.34824715818, 6206.80977871580},
    {0.00000000082, 4.59843208943, 9388.00590941520},
    {0.00000000079, 1.63139280394, 4933.20844033260},
    {0.00000000081, 1.55550779371, 9380.95967271720},
    {0.00000000078, 4.20905757519, 5729.50644714900},
    {0.00000000058, 5.76889633224, 3634.62102451840},
    {0.00000000060, 0.93813100594, 12721.57209941700},
    {0.00000000071, 6.11408885148, 8662.24032356300},
    {0.00000000057, 5.48112524468, 18319.53658487960},
    {0.00000000070, 0.01749174864, 14945.31617355440},
    {0.00000000074, 1.09976045820, 16460.33352952499},
    {0.00000000056, 1.63036186739, 15720.83878487840},
    {0.00000000055, 4.86788348404, 13095.84266507740},
    {0.00000000060, 5.93729841267, 12539.85338018300},
    {0.00000000054, 0.22608242982, 15110.46611986620},
    {0.00000000054, 2.30250047594, 16062.18452611680},
    {0.00000000064, 2.13513754101, 7875.67186362420},
    {0.00000000059, 5.87963500139, 5331.35744374080},
    {0.00000000058, 2.30546168615, 955.59974160860},
    {0.00000000049, 1.93839278478, 5333.90024102160},
    {0.00000000054, 5.80331607119, 12043.57428188900},
    {0.00000000054, 4.44671053809, 4701.11650170840},
    {0.00000000049, 0.30241161485, 6805.65326808520},
    {0.00000000046, 2.76898193028, 6709.67404086740},
    {0.00000000046, 3.98449608961, 98068.53671630539},
    {0.00000000049, 3.72022009896, 12323.42309600880},
    {0.00000000045, 3.30065998328, 22003.91463486980},
    {0.00000000048, 0.71071357303, 6303.43116939020},
    {0.00000000061, 1.66030429494, 6262.30045449900},
    {0.00000000047, 1.26317154881, 11919.14086666800},
    {0.00000000051, 1.08020906825, 10988.80815753500},
    {0.00000000045, 0.89150445122, 51868.24866217880},
    {0.00000000043, 0.57756724285, 24356.78078864160},
    {0.00000000043, 1.61526242998, 6277.55292568400},
    {0.00000000045, 2.96132920534, 8982.81066930900},
    {0.00000000043, 5.74295325645, 11403.67699557500},
    {0.00000000055, 3.14274403422, 33019.02111220460},
    {0.00000000057, 0.06379726305, 15671.08175940660},
    {0.00000000041, 2.53761820726, 6262.72053059260},
    {0.00000000040, 1.53130436944, 18451.07854656599},
    {0.00000000052, 1.71451922581, 1551.04522264800},
    {0.00000000055, 0.89439119424, 11933.36796066960},
    {0.00000000045, 3.88495384656, 60530.48898574180},
    {0.00000000040, 4.75740908001, 38526.57435087200},
    {0.00000000040, 3.77498297348, 26087.90314157420},
    {0.00000000039, 2.97113832621, 2118.76386037840},
    {0.00000000040, 3.36050962605, 10021.83728009940},
    {0.00000000047, 1.67051113434, 6303.85124548380},
    {0.00000000052, 5.21827368711, 77713.77146812050},
    {0.00000000047, 4.26356628717, 21424.46664430340},
    {0.00000000037, 1.66712389942, 6819.88036208680},
    {0.00000000037, 0.65746800933, 12029.34718788740},
    {0.00000000035, 3.36255650927, 24072.92146977640},
    {0.00000000036, 0.11087914947, 10344.29506538580},
    {0.00000000040, 4.14725582115, 2787.04302385740},
    {0.00000000035, 5.93650887012, 31570.79964939120},
    {0.00000000036, 2.15108874765, 30774.50164257480},
    {0.00000000036, 1.75078825382, 16207.88627150200},
    {0.00000000034, 2.75708224536, 12139.55350910680},
    {0.00000000034, 6.16891378800, 24491.42579258340},
    {0.00000000034, 2.31528650443, 55798.45835839840},
    {0.00000000032, 4.21446357042, 15664.03552270859},
    {0.00000000034, 3.19783054699, 32217.20018108080},
    {0.00000000039, 1.24979117796, 6418.14093002680},
    {0.00000000038, 5.89832942685, 640.87760738220},
    {0.00000000033, 4.80200120107, 16723.35014259500},
    {0.00000000032, 1.72442327688, 27433.88921587499},
    {0.00000000035, 4.44608896525, 18202.21671665939},
    {0.00000000031, 4.52790731280, 6702.56049386660},
    {0.00000000034, 3.96287980676, 18216.44381066100},
    {0.00000000030, 5.06259854444, 226858.23855437008},
    {0.00000000034, 1.43910280005, 49515.38250840700},
    {0.00000000030, 0.29303163371, 13521.75144159140},
    {0.00000000029, 2.02633840220, 11609.86254401220},
    {0.00000000030, 2.54923230240, 9924.81042151060},
    {0.00000000032, 4.91793198558, 11300.58422135640},
    {0.00000000030, 0.23284423547, 23581.25817731760},
    {0.00000000029, 1.62807736495, 639.89728631400},
    {0.00000000028, 3.84568936822, 2699.73481931760},
    {0.00000000029, 1.83149729794, 29822.78323632420},
    {0.00000000033, 4.60320094415, 19004.64794940840},
    {0.00000000027, 1.86151121799, 6288.59877429880},
    {0.00000000030, 4.46494072240, 36147.40987730040},
    {0.00000000028, 5.19684492912, 5863.59120611620},
    {0.00000000035, 4.52695674113, 36949.23080842420},
    {0.00000000027, 3.52528177609, 10770.89325626180},
    {0.00000000026, 1.48499438453, 11080.17157891760},
    {0.00000000035, 2.82154380962, 19402.79695281660},
    {0.00000000025, 2.46339998836, 6279.48542133960},
    {0.00000000026, 4.97688894643, 16737.57723659660},
    {0.00000000027, 0.40827112500, 12964.30070339100},
    {0.00000000029, 4.15148654061, 45892.73043315699},
    {0.00000000026, 4.56404104286, 17796.95916678580},
    {0.00000000025, 2.89309528854, 6286.66627864320},
    {0.00000000026, 4.82914580957, 1066.49547719000},
    {0.00000000031, 3.93096113738, 29864.33402730900},
    {0.00000000024, 6.14987193584, 18606.49894600020},
    {0.00000000024, 3.74225964547, 29026.48522950779},
    {0.00000000025, 5.70460621565, 27707.54249429480},
    {0.00000000025, 5.33928840652, 15141.39079431200},
    {0.00000000023, 2.37624087345, 17996.03116822220},
    {0.00000000026, 1.34231351782, 18875.52586977400},
    {0.00000000022, 5.50791626120, 6245.04817735560},
    {0.00000000024, 1.33998410121, 19800.94595622480},
    {0.00000000023, 0.22512280890, 6279.78949257360},
    {0.00000000022, 1.17576471775, 11925.27409260060},
    {0.00000000022, 3.58603606640, 6915.85958930460},
    {0.00000000023, 3.21621246666, 6286.36220740920},
    {0.00000000029, 2.09564449439, 15265.88651930040},
    {0.00000000022, 4.74660932338, 28230.18722269139},
    {0.00000000021, 2.30688751432, 5999.21653112620},
    {0.00000000028, 3.92087592807, 18208.34994259200},
    {0.00000000021, 3.22643339385, 25934.12433108940},
    {0.00000000021, 3.04956726238, 6566.93516885660},
    {0.00000000027, 5.35645770522, 33794.54372352860},
    {0.00000000025, 5.91542362188, 6489.26139842860},
    {0.00000000020, 1.52296293311, 135.06508003540},
    {0.00000000019, 1.78134428631, 156137.47598479928},
    {0.00000000019, 0.34388684087, 5327.47610838280},
    {0.00000000026, 3.41701003233, 25287.72379939980},
    {0.00000000019, 2.86664271911, 18422.62935909819},
    {0.00000000019, 4.71432851499, 77690.75950573849},
    {0.00000000019, 2.54227398241, 77736.78343050249},
    {0.00000000020, 5.91915117116, 48739.85989708300},
};

static const struct ln_vsop earth_radius_r2[RADIUS_R2] = {
    {0.00004359385, 5.78455133808, 6283.07584999140},
    {0.00000123633, 5.57935427994, 12566.15169998280},
    {0.00000012342, 3.14159265359, 0.00000000000},
    {0.00000008792, 3.62777893099, 77713.77146812050},
    {0.00000005689, 1.86958905084, 5573.14280143310},
    {0.00000003302, 5.47034879713, 18849.22754997420},
    {0.00000001471, 4.47964125007, 5507.55323866740},
    {0.00000001013, 2.81323115556, 5223.69391980220},
    {0.00000000854, 3.10776566900, 1577.34354244780},
    {0.00000001102, 2.84173992403, 161000.68573767410},
    {0.00000000648, 5.47348203398, 775.52261132400},
    {0.00000000608, 1.37894173533, 6438.49624942560},
    {0.00000000499, 4.41649242250, 6286.59896834040},
    {0.00000000416, 0.90332697974, 10977.07880469900},
    {0.00000000404, 3.20567269530, 5088.62883976680},
    {0.00000000351, 1.81081728907, 5486.77784317500},
    {0.00000000466, 3.65086758149, 7084.89678111520},
    {0.00000000458, 5.38585314743, 149854.40013480789},
    {0.00000000304, 3.51015066341, 796.29800681640},
    {0.00000000266, 6.17413982699, 6836.64525283380},
    {0.00000000281, 1.83874672540, 4694.00295470760},
    {0.00000000262, 1.41420110644, 2146.16541647520},
    {0.00000000264, 3.14103683911, 71430.69561812909},
    {0.00000000319, 5.35037932146, 3154.68708489560},
    {0.00000000238, 2.17695432424, 155.42039943420},
    {0.00000000229, 4.75969588070, 7234.79425624200},
    {0.00000000291, 4.61776401638, 4690.47983635860},
    {0.00000000211, 0.21864885298, 4705.73230754360},
    {0.00000000204, 4.22895113488, 1349.86740965880},
    {0.00000000195, 4.58550676556, 529.69096509460},
    {0.00000000255, 2.81442711144, 1748.01641306700},
    {0.00000000182, 5.70454011389, 6040.34724601740},
    {0.00000000180, 6.02147727878, 4292.33083295040},
    {0.00000000186, 1.58690991244, 6309.37416979120},
    {0.00000000167, 2.88802733052, 9437.76293488700},
    {0.00000000166, 1.99990574734, 8031.09226305840},
    {0.00000000160, 0.04412738495, 2544.31441988340},
    {0.00000000197, 2.01089431842, 1194.44701022460},
    {0.00000000165, 5.78372596774, 83996.84731811189},
    {0.00000000214, 3.38300910371, 7632.94325965020},
    {0.00000000140, 0.36669664351, 10447.38783960440},
    {0.00000000151, 0.95519595275, 6127.65545055720},
    {0.00000000136, 1.48417295645, 2352.86615377180},
    {0.00000000128, 5.48057748834, 951.71840625060},
    {0.00000000126, 5.26866506592, 6279.55273164240},
    {0.00000000127, 3.77552907014, 6812.76681508600},
    {0.00000000103, 4.95897533789, 398.14900340820},
    {0.00000000104, 0.70183576826, 1592.59601363280},
    {0.00000000101, 1.14481598642, 3894.18182954220},
    {0.00000000131, 0.76624310306, 553.56940284240},
    {0.00000000109, 5.41063597567, 6256.77753019160},
    {0.00000000078, 5.84775340741, 242.72860397400},
    {0.00000000097, 1.94685257714, 11856.21865142450},
    {0.00000000100, 5.19725292131, 244287.60000722769},
    {0.00000000076, 0.70480774041, 8429.24126646660},
    {0.00000000080, 6.18430772683, 1059.38193018920},
    {0.00000000068, 5.29561709093, 14143.49524243060},
    {0.00000000085, 5.39487308005, 25132.30339996560},
    {0.00000000055, 5.16874637579, 7058.59846131540},
    {0.00000000063, 0.48494730699, 801.82093112380},
    {0.00000000058, 4.07254840265, 13367.97263110660},
    {0.00000000051, 3.89696552232, 12036.46073488820},
    {0.00000000051, 5.56335232286, 1990.74501704100},
    {0.00000000060, 2.25046596710, 8635.94200376320},
    {0.00000000049, 5.58163417371, 6290.18939699220},
    {0.00000000051, 3.87240194908, 26.29831979980},
    {0.00000000051, 4.19300909995, 7860.41939243920},
    {0.00000000041, 3.97169191582, 10973.55568635000},
    {0.00000000041, 3.57080919230, 7079.37385680780},
    {0.00000000056, 2.76959005761, 90955.55169449610},
    {0.00000000042, 1.91461189163, 7477.52286021600},
    {0.00000000042, 0.42775891995, 10213.28554621100},
    {0.00000000042, 1.06925480488, 709.93304855830},
    {0.00000000038, 6.17935925345, 9917.69687450980},
    {0.00000000050, 0.81691517401, 11506.76976979360},
    {0.00000000053, 1.45828359397, 233141.31440436149},
    {0.00000000038, 3.32444534628, 5643.17856367740},
    {0.00000000047, 6.21543665927, 6681.22485339960},
    {0.00000000037, 0.36359309980, 10177.25767953360},
    {0.00000000045, 5.29587706357, 10575.40668294180},
    {0.00000000034, 5.63446915337, 6525.80445396540},
    {0.00000000034, 5.36385158519, 4933.20844033260},
    {0.00000000035, 5.36152295839, 25158.60171976540},
    {0.00000000042, 5.08837645072, 11015.10647733480},
    {0.00000000042, 4.22496037505, 88860.05707098669},
    {0.00000000039, 1.99171699618, 6284.05617105960},
    {0.00000000029, 3.19088628170, 11926.25441366880},
    {0.00000000029, 0.14996158324, 12168.00269657460},
    {0.00000000030, 1.58346276808, 9779.10867612540},
    {0.00000000026, 4.16210340581, 12569.67481833180},
    {0.00000000036, 2.74684637873, 3738.76143010800},
    {0.00000000026, 0.72824915320, 1589.07289528380},
    {0.00000000031, 5.34906371821, 143571.32428481648},
    {0.00000000025, 0.10240267494, 22483.84857449259},
    {0.00000000030, 3.47110495524, 14945.31617355440},
    {0.00000000026, 3.89359701125, 5753.38488489680},
    {0.00000000024, 1.18744224678, 4535.05943692440},
    {0.00000000033, 2.99317143244, 3930.20969621960},
    {0.00000000024, 1.57253767584, 6496.37494542940},
    {0.00000000024, 3.47434797542, 4136.91043351620},
    {0.00000000022, 3.91230073719, 6275.96230299060},
    {0.00000000025, 4.02978941287, 3128.38876509580},
    {0.00000000023, 1.07724492065, 12721.57209941700},
    {0.00000000021, 1.89591807148, 16730.46368959580},
    {0.00000000025, 2.42198937013, 5729.50644714900},
    {0.00000000020, 1.78163489101, 17789.84561978500},
    {0.00000000021, 0.49258939822, 29088.81141598500},
    {0.00000000026, 4.14947806747, 2388.89402044920},
    {0.00000000027, 2.54785812264, 3496.03282613400},
    {0.00000000020, 4.29944129273, 16858.48253293320},
    {0.00000000021, 5.97796936723, 7.11354700080},
    {0.00000000019, 0.80292033311, 16062.18452611680},
    {0.00000000024, 4.89894141052, 17260.15465469040},
    {0.00000000025, 1.37003752175, 6282.09552892320},
    {0.00000000022, 4.92663152168, 18875.52586977400},
    {0.00000000023, 5.68902059771, 16460.33352952499},
    {0.00000000023, 3.03021283729, 66567.48586525429},
    {0.00000000016, 3.89713736666, 5331.35744374080},
    {0.00000000016, 5.68562539832, 12559.03815298200},
    {0.00000000016, 3.95085099736, 3097.88382272579},
    {0.00000000016, 3.99041783945, 6283.14316029419},
    {0.00000000020, 6.10643919100, 167283.76158766549},
    {0.00000000015, 4.09775914607, 11712.95531823080},
    {0.00000000016, 5.71769940700, 17298.18232732620},
    {0.00000000016, 3.28894009404, 5884.92684658320},
    {0.00000000015, 4.42564243680, 13517.87010623340},
    {0.00000000016, 4.43452080930, 6283.00853968860},
    {0.00000000014, 1.44384279999, 4164.31198961300},
    {0.00000000014, 4.47380919159, 11790.62908865880},
    {0.00000000014, 4.77646531825, 7342.45778018060},
    {0.00000000011, 2.56768522896, 5481.25491886760},
    {0.00000000011, 1.51443332200, 16200.77272450120},
    {0.00000000011, 0.88708889185, 21228.39202354580},
    {0.00000000014, 4.50116508534, 640.87760738220},
};

static const struct ln_vsop earth_radius_r3[RADIUS_R3] = {
    {0.00000144595, 4.27319433901, 6283.07584999140},
    {0.00000006729, 3.91706261708, 12566.15169998280},
    {0.00000000774, 0.00000000000, 0.00000000000},
    {0.00000000247, 3.73021571217, 18849.22754997420},
    {0.00000000036, 2.80081409050, 6286.59896834040},
    {0.00000000033, 5.62990083112, 6127.65545055720},
    {0.00000000018, 3.72826142555, 6438.49624942560},
    {0.00000000016, 4.26011484232, 6525.80445396540},
    {0.00000000014, 3.47817116396, 6256.77753019160},
    {0.00000000012, 3.55747379482, 25132.30339996560},
    {0.00000000010, 4.43995693209, 4705.73230754360},
    {0.00000000010, 4.28045255470, 83996.84731811189},
    {0.00000000009, 5.36457057335, 6040.34724601740},
    {0.00000000008, 1.78458957263, 5507.55323866740},
    {0.00000000009, 0.47275199930, 6279.55273164240},
    {0.00000000009, 1.34741231639, 6309.37416979120},
    {0.00000000009, 0.77092900708, 5729.50644714900},
    {0.00000000007, 3.50146897332, 7058.59846131540},
    {0.00000000005, 2.89071061700, 775.52261132400},
    {0.00000000006, 2.36514111314, 6836.64525283380},
};

static const struct ln_vsop earth_radius_r4[RADIUS_R4] = {
    {0.00000003858, 2.56389016346, 6283.07584999140},
    {0.00000000306, 2.26911740541, 12566.15169998280},
    {0.00000000053, 3.44031471924, 5573.14280143310},
    {0.00000000015, 2.03136359366, 18849.22754997420},
    {0.00000000013, 2.05688873673, 77713.77146812050},
    {0.00000000007, 4.41218854480, 161000.68573767410},
    {0.00000000004, 5.33854414781, 6438.49624942560},
    {0.00000000006, 3.81514213664, 149854.40013480789},
    {0.00000000004, 4.26602478239, 6127.65545055720},
};

static const struct ln_vsop earth_radius_r5[RADIUS_R5] = {
    {0.00000000086, 1.21805304895, 6283.07584999140},
    {0.00000000012, 0.65572878044, 12566.15169998280},
};

/* Use in earth-sun function */
double ln_calc_series(const struct ln_vsop *data, int terms, double t)
{
    double value = 0;
    int i;

    for (i = 0; i < terms; i++) {
	value += data->A * cos(data->B + data->C * t);
	data++;
    }

    return value;
}

/* Get Julian day from Gregorian year, month and day */
double julian_int(int year, int month, int day)
{
    int a, b;

    if (month < 3) {
	year--;
	month += 12;
    }

    /* check for Julian or Gregorian calendar (starts Oct 4th 1582) */
    b = 0;
    if (year > 1582 ||
	(year == 1582 && (month > 10 || (month == 10 && day >= 4)))) {
	a = year / 100;
	b = 2 - a + (a / 4);
    }

    return ((int)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) +
	    day + b - 1524.5);
}

/* Get Julian day form Gregorian string yyyy-mm-dd */
double julian_char(const char date[])
{
    int day, month, year;

    year = atoi(date);
    month = atoi(date + 5);
    day = atoi(date + 8);

    return julian_int(year, month, day);
}

/* Earth-Sun distance in astronomical units */
double earth_sun(const char *date)
{
    double t;
    double R0, R1, R2, R3, R4, R5;

    t = (julian_char(date) - 2451545.0) / 365250.0;

    R0 = ln_calc_series(earth_radius_r0, RADIUS_R0, t);
    R1 = ln_calc_series(earth_radius_r1, RADIUS_R1, t);
    R2 = ln_calc_series(earth_radius_r2, RADIUS_R2, t);
    R3 = ln_calc_series(earth_radius_r3, RADIUS_R3, t);
    R4 = ln_calc_series(earth_radius_r4, RADIUS_R4, t);
    R5 = ln_calc_series(earth_radius_r5, RADIUS_R5, t);

    return (R0 + R1 * t + R2 * t * t + R3 * t * t * t + R4 * t * t * t * t +
	    R5 * t * t * t * t * t);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
