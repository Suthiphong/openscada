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
 
#ifndef TCNTRNODE_H
#define TCNTRNODE_H

#include <string>
#include <vector>

#include "terror.h"
#include "xml.h"

using std::string;
using std::vector;

//***************************************************************
//* AutoHD - for auto released HD resources			*
//***************************************************************
class TCntrNode;

template <class ORes> class AutoHD
{
    public:
        AutoHD( ): m_node(NULL) {  }
	AutoHD( TCntrNode *node, const string &who = "" ) : m_node(node)
	{ m_node->connect(); }	
	//Copying constructor
	AutoHD( const AutoHD &hd ): m_node(NULL) { operator=(hd); }
	template <class ORes1> AutoHD( const AutoHD<ORes1> &hd_s )
	{  
    	    m_node = hd_s.node();
	    m_node->connect();
	}
	~AutoHD( ){ free(); }
	
	ORes &at()
	{ 
	    if(m_node) return *(ORes *)m_node; 
	    throw TError("AutoHD no init!");	    
	}
	
	void operator=( const AutoHD &hd )
	{  
	    free();
	    
    	    m_node = hd.m_node;
	    m_node->connect();
	}		

	void free() 
	{
	    if(m_node) m_node->disConnect();
	    m_node = NULL;
	}
	
	bool freeStat() 
	{ return (m_node==NULL)?true:false; }

        TCntrNode *node() const { return m_node; }					
			    
    private:
	TCntrNode *m_node;
};

//***************************************************************
//* TCntrNode - Controll node					*
//***************************************************************
class TCntrNode
{
    //***********************************************************
    //*********** Controll section ******************************
    //***********************************************************
    public:
	enum Command { Info, Get, Set };
    
	TCntrNode( );
	virtual ~TCntrNode( );

	void cntrCmd( const string &path, XMLNode *opt, int cmd, int lev = 0 );	//New API	
	
	//============== Static functions =======================
	static XMLNode *ctrId( XMLNode *inf, const string &n_id );      //get node for he individual number
	static string ctrChk( XMLNode *fld, bool fix = false );		// Check fld valid
	
	// Controll Fields
	XMLNode *ctrMkNode( const string &n_nd, XMLNode *nd, const string &req, const string &path, 
	    const string &dscr, int perm=0777, int uid=0, int gid=0, const string &tp="" );	
	XMLNode *ctrInsNode( const string &n_nd, int pos, XMLNode *nd, const string &req, const string &path, 
	    const string &dscr, int perm=0777, int uid=0, int gid=0, const string &tp="" );
	
	// Get option's values
	static string ctrGetS( XMLNode *fld );	//string
	static int    ctrGetI( XMLNode *fld );	//integer
	static double ctrGetR( XMLNode *fld );	//real
        static bool   ctrGetB( XMLNode *fld );	//boolean
	
	// Set option's values	
	static void ctrSetS( XMLNode *fld, const string &val, char *id=NULL );	//string
	static void ctrSetI( XMLNode *fld, int val, char *id=NULL );   	//integer
	static void ctrSetR( XMLNode *fld, double val, char *id=NULL );	//real
	static void ctrSetB( XMLNode *fld, bool val, char *id=NULL );		//boolean

	// Path parse
        static string pathLev( const string &path, int level, bool encode = true );
        static string pathCode( const string &in, bool el );
        static string pathEncode( const string &in, bool el );				
	
    protected:
	virtual void cntrCmd_( const string &path, XMLNode *opt, int cmd ){ };	//NEW API
	virtual void ctrStat_( XMLNode *inf ){ };	
	virtual void ctrDinSet_( const string &area_path, XMLNode *opt ){ };
	virtual void ctrDinGet_( const string &area_path, XMLNode *opt ){ };
        //---------- at mode ------------------
        virtual TCntrNode &ctrAt( const string &br )
        { throw TError("(%s) Function <ctrAt> no support!",__func__); }
        //---------- Auto at mode ------------------
        virtual AutoHD<TCntrNode> ctrAt1( const string &br )
        { throw TError("(%s) Function <ctrAt1> no support!",__func__); }	
	
    //***********************************************************
    //*********** Resource section ******************************
    //***********************************************************
    public:
	enum Mode{ MkDisable, Disable, MkEnable, Enable };
	
       	virtual string nodeName(){ return("NO Named!"); }
   
        Mode mode(){ return m_mod; }
	unsigned use( );

	//Template not understand !!!!
	void connect();
	void disConnect();
    
    protected:
	//Commands
	void nodeEn();
	void nodeDis(long tm = 0,int flag = 0);
	
	void delAll( );	//For hard link objects
	
	//Conteiners
        unsigned grpSize(){ return chGrp.size(); }
        unsigned grpAdd( );
	
	//Childs
	void chldList( unsigned igr, vector<string> &list );
	bool chldAvoid( unsigned igr, const string &name );
	void chldAdd( unsigned igr, TCntrNode *node, int pos = -1 );
	void chldDel( unsigned igr, const string &name, long tm = -1, int flag = 0 );

        AutoHD<TCntrNode> chldAt( unsigned igr, const string &name, const string &user = "" );
	
	virtual void preEnable(){ }
	virtual void postEnable(){ }
	
	virtual void preDisable(int flag){ }
	virtual void postDisable(int flag){ }

    private:	
	int 	hd_res;				//Resource HD
	int 	m_use;				//Use counter
	vector< vector<TCntrNode*> >	chGrp;	//Child groups
	static long	dtm;			//Default timeout
	
	static XMLNode	m_dummy;	//Dummy node for noview requests
	
	Mode	m_mod;
};


#endif //TCNTRNODE_H

