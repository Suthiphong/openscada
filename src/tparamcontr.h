
#ifndef TPARAMCONTR_H
#define TPARAMCONTR_H

#include <time.h>
#include <string>
using std::string;

#include "tkernel.h"
#include "tparams.h"
#include "tconfig.h"
#include "tvalue.h"

//==============================================================================================
//Use for register into TControllers and make direct access to controller function for parameter
//and direct acces to parameter from controller
//==============================================================================================
class TController;

class TParamContr : public TConfig, public TValue
{
/** Public methods: */
public:
    TParamContr(TController *contr, TConfigElem *cfgelem); 
    virtual ~TParamContr();
    /*
     * Param name
     */
    inline string Name();
    /*
     * Compare object
     */
    bool operator==( TParamContr & PrmCntr )
    { if( Name() == PrmCntr.Name() ) return(true); return(false); };
    /*
     * Equaly config 
     */
    TParamContr & operator=( TParamContr & PrmCntr );

    /*
     * Check for new value type
     */
    virtual void UpdateVAL();
    /*
     * Enable parameter and open access to value
     */
    virtual void Enable();
    /*
     * Disable parameter and close access to value
     */
    virtual void Disable();
/**Attributes: */
public:
    TController *owner;
    time_t      t_sync;  // time synchronized
private:
    virtual TConfig *GetCfg( ) { return(this); }
/**Attributes: */
private:
    short       own;   // id from password
    short       grp;   // id from group

    static const char *o_name;
};

#endif // TPARAMCONTR_H
