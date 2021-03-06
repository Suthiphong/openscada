
//OpenSCADA file: tbds.h
/***************************************************************************
 *   Copyright (C) 2003-2019 by Roman Savochenko, <rom_as@oscada.org>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
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

#ifndef TBDS_H
#define TBDS_H

#define SDB_VER		20		//BDS type modules version
#define SDB_ID		"BD"

#include <stdio.h>

#include <string>
#include <vector>
#include <deque>

#include "resalloc.h"
#include "tsubsys.h"
#include "tvariant.h"
#include "tconfig.h"

using std::string;
using std::vector;
using std::deque;

namespace OSCADA
{

//*************************************************
//* TTable                                        *
//*************************************************
class TBD;

class TTable : public TCntrNode
{
    public:
	//Public methods
	TTable( const string &name );
	virtual ~TTable( );

	TCntrNode &operator=( const TCntrNode &node );

	string	name( )		{ return mName.c_str(); }
	string	fullDBName( );
	time_t	lstUse( )	{ return mLstUse; }

	virtual void fieldStruct( TConfig &cfg )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"fieldStruct"); }
	virtual bool fieldSeek( int row, TConfig &cfg, vector< vector<string> > *full = NULL )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"fieldSeek"); }
	virtual void fieldGet( TConfig &cfg )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"fieldGet"); }
	virtual void fieldSet( TConfig &cfg )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"fieldSet"); }
	virtual void fieldDel( TConfig &cfg )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"fieldDel"); }

	TBD &owner( ) const;

    protected:
	//Protected methods
	void cntrCmdProc( XMLNode *opt );       //Control interface command process
	TVariant objFuncCall( const string &id, vector<TVariant> &prms, const string &user );

	//Protected attributes
	time_t	mLstUse;

    private:
	//Private methods
	const char *nodeName( ) const	{ return mName.c_str(); }

	//Private attributes
	const string	mName;
	bool		notFullShow;
	int		tblOff, tblSz;
};

//************************************************
//* TBD                                          *
//************************************************
class TTypeBD;

class TBD : public TCntrNode, public TConfig
{
    public:
	//Public methods
	TBD( const string &iid, TElem *cf_el );
	virtual ~TBD( );

	TCntrNode &operator=( const TCntrNode &node );

	string	id( )		{ return mId; }
	string	fullDBName( );
	string	name( );
	string	dscr( )		{ return cfg("DESCR").getS(); }
	string	addr( ) const	{ return cfg("ADDR").getS(); }
	string	codePage( )	{ return cfg("CODEPAGE").getS(); }

	bool enableStat( ) const	{ return mEn; }
	bool toEnable( )		{ return mToEn; }

	void setName( const string &inm )	{ cfg("NAME").setS(inm); }
	void setDscr( const string &idscr )	{ cfg("DESCR").setS(idscr); }
	void setAddr( const string &iaddr )	{ cfg("ADDR").setS(iaddr); }
	void setCodePage( const string &icp )	{ cfg("CODEPAGE").setS(icp); }
	void setToEnable( bool ivl )		{ mToEn = ivl; modif(); }

	virtual void enable( );
	virtual void disable( );

	// Opened DB tables
	virtual void allowList( vector<string> &list ) const
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"allowList"); }
	void list( vector<string> &list ) const		{ chldList(mTbl, list); }
	bool openStat( const string &table ) const	{ return chldPresent(mTbl, table); }
	void open( const string &table, bool create );
	void close( const string &table, bool del = false, long tm = -1 )	{ chldDel(mTbl, table, tm, del); }
	AutoHD<TTable> at( const string &name ) const	{ return chldAt(mTbl, name); }

	// SQL request interface
	virtual void sqlReq( const string &req, vector< vector<string> > *tbl = NULL, char intoTrans = EVAL_BOOL )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"sqlReq"); }

	virtual void transCloseCheck( )		{ }

	TTypeBD &owner( ) const;

	//Public attributes
	ResMtx	resTbls;

    protected:
	//Protected methods
	virtual TTable *openTable( const string &table, bool create )
	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"openTable"); }

	void postEnable( int flag );
	void preDisable( int flag );
	void postDisable( int flag );
	bool cfgChange( TCfg &co, const TVariant &pc )	{ modif(); return true; }

	void load_( TConfig *cfg );
	void save_( );

	void cntrCmdProc( XMLNode *opt );	//Control interface command process
	TVariant objFuncCall( const string &id, vector<TVariant> &prms, const string &user );

	AutoHD<TCntrNode> chldAt( int8_t igr, const string &name, const string &user = "" ) const;

	//Protected attributes
	bool	mEn;

    private:
	//Private methods
	const char *nodeName( ) const	{ return mId.getSd(); }

	//Private attributes
	// Base options
	TCfg	&mId;	//ID
	char	&mToEn;

	// Special options
	int	mTbl;

	string	userSQLReq;
	vector< vector<string> > userSQLResTbl;
	char	userSQLTrans;
};

//************************************************
//* TTypeBD                                      *
//************************************************
class TBDS;

class TTypeBD : public TModule
{
    public:
	//Public methods
	TTypeBD( const string &id );
	virtual ~TTypeBD( );

	bool fullDeleteDB( )	{ return fullDBDel; }

	// Opened DB
	void list( vector<string> &list ) const		{ chldList(mDB, list); }
	bool openStat( const string &idb ) const	{ return chldPresent(mDB, idb); }
	string open( const string &id );
	void close( const string &id, bool erase = false ) { chldDel(mDB, id, -1, erase); }
	AutoHD<TBD> at( const string &id ) const	{ return chldAt(mDB, id); }

	TBDS &owner( ) const;

    private:
	//Private methods
	void cntrCmdProc( XMLNode *opt );       //Control interface command process

	virtual TBD *openBD( const string &id )	{ throw TError(nodePath().c_str(),_("Function '%s' is not supported!"),"openBD"); }

	//Private attributes
	bool	fullDBDel;
	int	mDB;
};

//************************************************
//* TBDS                                         *
//************************************************
class TSYS;

class TBDS : public TSubSYS, public TElem
{
    public:
	//Data
	enum ReqGen {
	    OnlyCfg	= 0x01,		//Only from cinfig request
	    UseTranslate= 0x02		//Use translation for request
	};

	//Public methods
	TBDS( );
	~TBDS( );

	int subVer( )		{ return SDB_VER; }

	static string realDBName( const string &bdn );
	void dbList( vector<string> &ls, bool checkSel = false );

	void perSYSCall( unsigned int cnt );

	// Open/close table.
	AutoHD<TTable> open( const string &bdn, bool create = false );
	void close( const string &bdn, bool del = false );

	// Get Data from DB or config file. If <tbl> cleaned then load from config-file
	bool dataSeek( const string &bdn, const string &path, int lev, TConfig &cfg, bool forceCfg = false, vector< vector<string> > *full = NULL );
	bool dataGet( const string &bdn, const string &path, TConfig &cfg, bool forceCfg = false, bool noEx = false );
	bool dataSet( const string &bdn, const string &path, TConfig &cfg, bool forceCfg = false, bool noEx = false );
	bool dataDel( const string &bdn, const string &path, TConfig &cfg, bool useKeyAll = false, bool forceCfg = false, bool noEx = false );	//Next test for noEx=false

	// Generic DB table
	static string genDBGet( const string &path, const string &oval = "", const string &user = "root", char rFlg = 0 );
	static void genDBSet( const string &path, const string &val, const string &user = "root", char rFlg = 0 );

	string fullDBSYS( );
	string fullDB( );

	TElem &openDB_E( )	{ return elDB; }

	AutoHD<TTypeBD> at( const string &iid ) const	{ return modAt(iid); }

	string optDescr( );

    protected:
	void load_( );

    private:
	//Private methods
	void cntrCmdProc( XMLNode *opt );       //Control interface command process

	//Private attributes
	TElem	elDB;
	bool	mSYSStPref;
};

}

#endif // TBDS_H
