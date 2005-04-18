/***************************************************************************
 *   Copyright (C) 2004 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>

#include <terror.h>
#include <tsys.h>
#include <tkernel.h>
#include <tmessage.h>
#include <ttiparam.h>
#include <tcontrollers.h>

#include "diamond.h"

//============ Modul info! =====================================================
#define MOD_ID      "diamond"
#define MOD_NAME    "Diamond system data acquisition controller"
#define MOD_TYPE    "Controller"
#define VER_TYPE    VER_CNTR
#define VERSION     "0.0.1"
#define AUTORS      "Roman Savochenko"
#define DESCRIPTION "Allow Diamond system boards' data acquisition controller. Include support of Athena board."
#define LICENSE     "GPL"
//==============================================================================

extern "C"
{
    TModule::SAt module( int n_mod )
    {
	TModule::SAt AtMod;

	if(n_mod==0)
	{
	    AtMod.id	= MOD_ID;
	    AtMod.type  = MOD_TYPE;
	    AtMod.t_ver = VER_TYPE;
	}
	else
    	    AtMod.id	= "";

	return( AtMod );
    }

    TModule *attach( const TModule::SAt &AtMod, const string &source )
    {
	Diamond::TTpContr *self_addr = NULL;

    	if( AtMod.id == MOD_ID && AtMod.type == MOD_TYPE && AtMod.t_ver == VER_TYPE )
	    self_addr = new Diamond::TTpContr( source );

	return ( self_addr );
    }
}

using namespace Diamond;

//======================================================================
//==== TTpContr ======================================================== 
//======================================================================
TTpContr::TTpContr( string name ) : 
    m_init(false), elem_ai("AI"), elem_ao("AO"), elem_di("DI"), elem_do("DO")
{
    mId 	= MOD_ID;
    mName       = MOD_NAME;
    mType  	= MOD_TYPE;
    Vers      	= VERSION;
    Autors    	= AUTORS;
    DescrMod  	= DESCRIPTION;
    License   	= LICENSE;
    Source    	= name;    
}

TTpContr::~TTpContr()
{    
    BYTE result;
    ERRPARAMS errparams;
    
    if( m_init )    
	if((result = dscFree()) != DE_NONE)
	{
	    dscGetLastError(&errparams);
	    mPut("CONTR:Diamond",MESS_WARNING,"dscFree failed: %s (%s)",
                dscGetErrorString(result), errparams.errstring);
	}
}

void TTpContr::modConnect( )
{    
    BYTE result;
    ERRPARAMS errparams;
    
    TModule::modConnect( );
    
    //==== Controler's bd structure ====
    fldAdd( new TFld("BOARD",I18N("Diamond system board"),T_DEC|T_SELECT,"3","0",
	"0;1;2;3;4;5;6;7;8;9;10;11;12;12;13;14;15;16;17;18;19;19;20;21;22;22;23;24;24;24",
	"DMM16;RMM;TMM;OPMM;DMM;SMM;GMM;QMM;ZMM;PMM;OMM;RMM416;DMM32;DMM32AT;EMMDIO;RMM1612;DMMAT;DMM16AT;IR104;EMM8;PROM;ATHENA;HERCEBX;CPT;DMM48;DMM48AT;OMMDIO;DIO82C55;MRC;EMMOPTO") );
    fldAdd( new TFld("PRM_BD_A",I18N("Analog parameters' table"),T_STRING,"30","diamond_prm_a") );
    fldAdd( new TFld("PRM_BD_D",I18N("Digital parameters' table"),T_STRING,"30","diamond_prm_d") );    
    fldAdd( new TFld("ADDR",I18N("Base board address"),T_HEX,"3","768") );
    fldAdd( new TFld("INT",I18N("Interrupt vector"),T_DEC,"2","5") );
    fldAdd( new TFld("DMA",I18N("DMA number"),T_DEC,"1","1") );
    
    //==== Parameter type bd structure ====
    //---- Analog ----
    int t_prm = tpParmAdd("a_prm","PRM_BD_A",I18N("Analog parameter"));
    tpPrmAt(t_prm).fldAdd( new TFld("TYPE",I18N("Analog parameter type"),T_DEC|T_SELECT|F_NOVAL|F_PREV,"1","0","0;1","Input;Output") );
    tpPrmAt(t_prm).fldAdd( new TFld("CNL",I18N("Channel number"),T_DEC|F_NOVAL|F_PREV,"2","0") );
    tpPrmAt(t_prm).fldAdd( new TFld("GAIN",I18N("A/D converter gain"),T_DEC|T_SELECT|F_NOVAL|F_PREV,"1","0","0;1;2;3","x1;x2;x4;x8") );
    tpPrmAt(t_prm).fldAdd( new TFld("POLAR",I18N("Polarity"),T_BOOL|T_SELECT|F_NOVAL|F_PREV,"1","false","false;true","Bipolar;Unipolar") );
    //---- Digit ----
    t_prm = tpParmAdd("d_prm","PRM_BD_D",I18N("Digital parameter"));
    tpPrmAt(t_prm).fldAdd( new TFld("TYPE",I18N("Digital parameter type"),T_DEC|T_SELECT|F_NOVAL,"1","0","0;1","Input;Output") );
    tpPrmAt(t_prm).fldAdd( new TFld("CNL",I18N("Channel number"),T_DEC|F_NOVAL,"2","0") );

    //==== Init value elements =====
    //---- Analog input ----
    elem_ai.fldAdd( new TFld("value","Value %",T_REAL|F_NWR|F_DRD,"6.2","0") );
    elem_ai.fldAdd( new TFld("voltage","Voltage V",T_REAL|F_NWR|F_DRD,"7.4","0") );
    elem_ai.fldAdd( new TFld("code","A/D code",T_HEX|F_NWR|F_DRD,"4","0") );
    //---- Analog output ----
    elem_ao.fldAdd( new TFld("value","Value %",T_REAL|F_DWR,"6.2","0") );
    elem_ao.fldAdd( new TFld("voltage","Voltage V",T_REAL|F_DWR,"7.4","0") );
    //---- Digit input ----
    elem_di.fldAdd( new TFld("value","Value",T_BOOL|F_NWR|F_DRD,"1","0") );
    //---- Digit output ----
    elem_do.fldAdd( new TFld("value","Value",T_BOOL|F_DWR,"1","0") );    

    //==== Init DSC ====
    if((result = dscInit(DSC_VERSION)) != DE_NONE)
    {
        dscGetLastError(&errparams);
	mPut("CONTR:Diamond",MESS_WARNING,"dscInit failed: %s (%s)",
	    dscGetErrorString(result), errparams.errstring);
    }
    else m_init = true;
}

TController *TTpContr::ContrAttach( const string &name, const TBDS::SName &bd)
{
    return( new TMdContr(name,bd,this,this));    
}

//======================================================================
//==== TMdContr 
//======================================================================
TMdContr::TMdContr( string name_c, const TBDS::SName &bd, ::TTipController *tcntr, ::TElem *cfgelem) :
	::TController(name_c,bd,tcntr,cfgelem) 
{

}

TMdContr::~TMdContr()
{
    
}

TParamContr *TMdContr::ParamAttach( const string &name, int type )
{    
    return(new TMdPrm(name,&owner().tpPrmAt(type),this));
}

void TMdContr::load_( )
{

}

void TMdContr::save_( )
{

}

void TMdContr::start_( )
{
    BYTE result;
    ERRPARAMS errparams;
    
    //Check inited of Diamond API    
    if( !((TTpContr &)owner()).initStat() )
        throw TError("%s: Module no inited!",MOD_ID);
	    
    
    if( !run_st )
    {
	DSCCB dsccb;
	
	dsccb.base_address	= cfg("ADDR").getI();
        dsccb.int_level 	= cfg("INT").getI();
        dsccb.dma_level 	= cfg("DMA").getI();

	if((result = dscInitBoard(cfg("BOARD").getI(), &dsccb, &dscb)) != DE_NONE)
	{
	    dscGetLastError(&errparams);
	    throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);				    
	}	
	run_st = true;
	
	//Enable all parameters
	vector<string> prm_list;
	for( int i_prm = 0; i_prm < prm_list.size(); i_prm++ )
	    if( at(prm_list[i_prm]).at().toEnable() )
		at(prm_list[i_prm]).at().enable();
    }    
}

void TMdContr::stop_( )
{  
    BYTE result;
    ERRPARAMS errparams;
    
    if( run_st )
    {
	if((result = dscFreeBoard(dscb)) != DE_NONE)
	{
	    dscGetLastError(&errparams);
            throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);	
	}	
	run_st = false;
    }    
} 

//======================================================================
//==== TMdPrm 
//======================================================================
TMdPrm::TMdPrm( string name, TTipParam *tp_prm, TController *contr) :
    TParamContr(name,tp_prm,contr), m_cnl(cfg("CNL").getI()), m_tp(NONE)
{          
    if( tp_prm->name() == "a_prm" ) 	type(AI);
    else if( tp_prm->name() == "d_prm" )type(DI);       
}

TMdPrm::~TMdPrm( )
{   

}

void TMdPrm::preDisable( int flag )
{
    type(NONE);
}

void TMdPrm::type( TMdPrm::Type vtp )
{
    //Free previos type
    if( m_tp == AI )		vlDetElem( &((TTpContr&)owner().owner()).elemAI() );
    else if( m_tp == AO )	vlDetElem( &((TTpContr&)owner().owner()).elemAO() );
    else if( m_tp == DI )  	vlDetElem( &((TTpContr&)owner().owner()).elemDI() );
    else if( m_tp == DO )  	vlDetElem( &((TTpContr&)owner().owner()).elemDO() );    
    
    //Init new type
    if( vtp == AI )
    {    
	cfg("GAIN").view(true);
	ad_set.current_channel      = m_cnl;
	ad_set.gain                 = cfg("GAIN").getI();
	ad_set.range                = 1;
	ad_set.polarity             = cfg("POLAR").getB();
	ad_set.load_cal             = 0;
	
	vlAttElem( &((TTpContr&)owner().owner()).elemAI() );
    }
    else if( vtp == AO )
    {
	cfg("GAIN").view(false);    
	vlAttElem( &((TTpContr&)owner().owner()).elemAO() );
    }
    else if( vtp == DI )
    {
	vlAttElem( &((TTpContr&)owner().owner()).elemDI() );
    }
    else if( vtp == DO )
    {
        vlAttElem( &((TTpContr&)owner().owner()).elemDO() );
    }    
    m_tp = vtp;
}

void TMdPrm::enable()
{
    /*BYTE result;
    ERRPARAMS errparams;
    
    //Init channel
    if( type() == AI )
    {
	DSCSAMPLE dscsample;
	
	//Controll request
	if((result = dscADSetSettings( ((TMdContr&)owner()).cntrAccess(), &dsc.ad)) != DE_NONE )
	{
	    dscGetLastError(&errparams);
    	    throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);
	}
	if((result = dscADSample( ((TMdContr&)owner()).cntrAccess(), &dscsample)) != DE_NONE )
	{
	    dscGetLastError(&errparams);
            throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);    
	}	    	    
    }    
    else if( type() == AO )
    {
        //Controll request
        if((result = dscDAConvert( ((TMdContr&)owner()).cntrAccess(), m_cnl,4095)) != DE_NONE )
        {
	    dscGetLastError(&errparams);
            throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);
        }	
    }*/
    
    TParamContr::enable();
}

void TMdPrm::disable()
{
    TParamContr::disable();
}

bool TMdPrm::cfgChange( TCfg &i_cfg )
{
    //Change TYPE parameter
    if( i_cfg.name() == "TYPE" )
    {
	if( i_cfg.getI() == 0 && type() == AO )		type(AI);
	else if( i_cfg.getI() == 0 && type() == DO ) 	type(DI);
	if( i_cfg.getI() == 1 && type() == AI ) 	type(AO);
        else if( i_cfg.getI() == 1 && type() == DI ) 	type(DO);
	else return false;
    }
    if( type() == AI )
    {
	if( i_cfg.name() == "GAIN" ) 		ad_set.gain = cfg("GAIN").getI();
	else if( i_cfg.name() == "CNL" )	ad_set.current_channel = m_cnl;
	else if( i_cfg.name() == "POLAR" )	ad_set.polarity = cfg("POLAR").getB();
	else return false;
    }
    return(true);
}									

void TMdPrm::vlSet( TVal &val )
{
    BYTE result;
    ERRPARAMS errparams;
	
    if( type() == AO )
    {	
	
	if(val.name()=="value")
	{
	    Mess->put("CONTR:Diamond",MESS_INFO,"AO %d. set value: %f\%",m_cnl,val.getR() );
	    result = dscDAConvert( ((TMdContr&)owner()).cntrAccess(), m_cnl,(DSCDACODE)val.getR() );
	}
	else if(val.name()=="voltage")
	{
	    Mess->put("CONTR:Diamond",MESS_INFO,"AO %d. set voltage: %fv",m_cnl,val.getR() );
	    result = dscDAConvert( ((TMdContr&)owner()).cntrAccess(), m_cnl,(DSCDACODE)(4095*val.getR()/10.) );
	}
	if( result != DE_NONE )
        {
            dscGetLastError(&errparams);
            Mess->put("CONTR:Diamond",MESS_WARNING,"%s (%s)",dscGetErrorString(result), errparams.errstring);
	}
    }	
}

void TMdPrm::vlGet( TVal &val )
{
    BYTE result;
    ERRPARAMS errparams;

    if( type() == AI )
    {
	DSCSAMPLE dscsample;
	
	//Controll request
	if((result = dscADSetSettings( ((TMdContr&)owner()).cntrAccess(), &ad_set)) != DE_NONE )
	{
	    dscGetLastError(&errparams);
	    Mess->put("CONTR:Diamond",MESS_WARNING,"%s (%s)",dscGetErrorString(result), errparams.errstring);
    	    //throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);
	}
	else if((result = dscADSample( ((TMdContr&)owner()).cntrAccess(), &dscsample)) != DE_NONE )
	{
	    dscGetLastError(&errparams);
	    Mess->put("CONTR:Diamond",MESS_WARNING,"%s (%s)",dscGetErrorString(result), errparams.errstring);
            //throw TError("%s (%s)!",dscGetErrorString(result), errparams.errstring);    
	}
	else
	{
	    if(val.name()=="value")	val.setR(100.*((double)dscsample/32768.),NULL,true);
	    else if(val.name()=="code")	val.setI(dscsample,NULL,true);
	    else if(val.name()=="voltage")	val.setR(10.*((double)dscsample/32768.),NULL,true);	    	    
	}
    }
}	
