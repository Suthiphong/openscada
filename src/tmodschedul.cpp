#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <dirent.h>

#include <string>

#include "tapplication.h"
#include "tmessage.h"
#include "tbd.h"
#include "tarhive.h"
#include "tparams.h"
#include "tparam.h"
#include "tcontroller.h"
#include "tparamcontr.h"
#include "tcontrollers.h"
#include "ttipcontroller.h"
#include "tprocrequest.h"
#include "tprotocol.h"
#include "tspecial.h"
#include "tvalue.h"
#include "tmodschedul.h"

TModSchedul::TModSchedul(  ) : work(false)
{

}

TModSchedul::~TModSchedul(  )
{
    work=false;
    sleep(1);
}

void TModSchedul::StartSched( )
{
 
    pthread_attr_t      pthr_attr;
    struct sched_param  prior;

    //==== Test ====
    try
    {
	vector<string> list_el;
	App->Param->at("TEST_VirtualC")->at()->Elem()->List(list_el);
	App->Mess->put(1,"Elements: %d",list_el.size());
	for(int i=0; i< list_el.size(); i++)
	    App->Mess->put(1,"Element: %s",list_el[i].c_str());
    } catch(TError error) {  }
    

    
    vector<string> list_ct,list_c,list_pt,list_pc;

    //App->Controller->AddContr("test3","virtual_v1","virt_c");
    //App->Controller->at("test3")->Add("ANALOG","TEST_VirtualC",-1);
    //App->Controller->at("test3")->Del("ANALOG","TEST_VirtualC");
    //App->Controller->DelContr("test3");
    //App->Controller->UpdateBD();    

    App->Controller->List(list_ct);
    App->Mess->put(1,"Controller types: %d",list_ct.size());
    for(int i=0; i < list_ct.size(); i++)
    {
	try
	{
    	    App->Mess->put(1,"Controller type: <%s>",list_ct[i].c_str());

    	    App->Controller->at_tp(list_ct[i])->ListTpPrm(list_pt);
    	    App->Mess->put(1,"Types param's: %d",list_pt.size());
    	    for(int ii=0; ii < list_pt.size(); ii++)
		App->Mess->put(1,"Type: <%s>",list_pt[ii].c_str());

	    App->Controller->at_tp(list_ct[i])->List(list_c);
	    App->Mess->put(1,"Controllers: %d",list_c.size());
	    for(int ii=0; ii < list_c.size(); ii++)
	    {
		App->Mess->put(1,"Controller: <%s>",list_c[ii].c_str());
		for(int i_pt=0; i_pt < list_pt.size(); i_pt++)
		{
		    App->Controller->at(list_c[ii])->List(list_pt[i_pt],list_pc);
		    App->Mess->put(1,"%s Parameters: %d",list_pt[i_pt].c_str(),list_pc.size());
		    for(int iii=0; iii < list_pc.size(); iii++)
		    {
			App->Mess->SconvOut("KOI8-U",list_pc[iii]);
			App->Mess->put(1,"Parameter: <%s>",list_pc[iii].c_str());
		    }
		}
	    }
	}
	catch(TError err){ }
    }

    App->Param->List(list_pc);
    App->Mess->put(1,"Params: %d",list_pc.size());
    for(int i=0; i < list_pc.size(); i++)
    {
	App->Mess->SconvOut("KOI8-U",list_pc[i]);
	App->Mess->put(1,"Param: <%s>",list_pc[i].c_str());
    }
    //==============


    work=true;    
    pthread_attr_init(&pthr_attr);
    pthread_attr_setschedpolicy(&pthr_attr,SCHED_OTHER);
    pthread_create(&pthr_tsk,&pthr_attr,TModSchedul::SchedTask,NULL);
    pthread_attr_destroy(&pthr_attr);
}

void *TModSchedul::SchedTask(void *param)
{
    do {	
#if debug
	App->Mess->put(1,"Call scheduler!");
#endif
   	App->ModSchedul->Load(App->ModPath,-1);
       	for(unsigned i_gm=0; i_gm < App->ModSchedul->grpmod.size(); i_gm++)
    	    App->ModSchedul->Load(App->ModSchedul->grpmod[i_gm]->ModPath(),i_gm); 

	sleep(10);
    } while(App->ModSchedul->work==true); 
}

int TModSchedul::RegGroupM(TGRPModule *gmod)
{
    if(gmod == NULL) return(-1);
    for(unsigned i_grmd = 0; i_grmd < grpmod.size(); i_grmd++)
	if(grpmod[i_grmd] == gmod) return(i_grmd);
    grpmod.push_back(gmod);

    return(grpmod.size()-1);
}

int TModSchedul::UnRegGroupM(TGRPModule *gmod)
{
    if(gmod == NULL) return(-1);
    for(unsigned i_grmd = 0; i_grmd < grpmod.size(); i_grmd++)
	if(grpmod[i_grmd] == gmod)
	{
	    grpmod.erase(grpmod.begin()+i_grmd);
	    return(0);
	}

    return(-2);
}

void TModSchedul::CheckCommandLine(  )
{
    for(unsigned i_gm=0; i_gm < grpmod.size(); i_gm++)
	grpmod[i_gm]->CheckCommandLine();
}

void TModSchedul::LoadAll(  )
{
    Load(App->ModPath,-1);
    for(unsigned i_gm=0; i_gm < grpmod.size(); i_gm++)
	Load(grpmod[i_gm]->ModPath(),i_gm);
}

void TModSchedul::InitAll(  )
{
    for(unsigned i_gm=0; i_gm < grpmod.size(); i_gm++)
	grpmod[i_gm]->InitAll( );
}

void TModSchedul::StartAll(  )
{
     for(unsigned i_gm=0; i_gm < grpmod.size(); i_gm++)
	grpmod[i_gm]->StartAll( ); 
}

void TModSchedul::Load( string path, int dest)
{
    string Mod, Mods;
    int ido, id=-1;

    ScanDir( path, Mods );
    do
    {
        ido=id+1; id = Mods.find(",",ido);
        Mod=Mods.substr(ido,id-ido);
        if(Mod.size()<=0) continue;
        if(CheckFile((char *)Mod.c_str(),true) == true) 
	    AddShLib((char *)Mod.c_str(),dest);
    } while(id != string::npos);
}

void TModSchedul::ScanDir( const string & Paths, string & Mods )
{
    string NameMod, Path;
    char   buf[256];           //!!!!

    int ido, id=-1;
    do
    {
        ido=id+1; id = Paths.find(",",ido);

        dirent *scan_dirent;
        Path=Paths.substr(ido,id-ido);
        if(Path.size() <= 0) continue;
	// Convert to absolutly path
        getcwd(buf,sizeof(buf));
        if(chdir(Path.c_str()) != 0) continue;
        Path=buf;
        getcwd(buf,sizeof(buf));
        chdir(Path.c_str());
        Path=buf;

#if debug
        App->Mess->put(0,"Open dir <%s> !", Path.c_str());
#endif
        DIR *IdDir = opendir(Path.c_str());
        if(IdDir == NULL) continue;

        while(scan_dirent=readdir(IdDir))
        {
            NameMod=Path+"/"+scan_dirent->d_name;
            if(CheckFile((char *)NameMod.c_str(),false) != true) continue;
            if(Mods.find(NameMod) == string::npos ) Mods=Mods+NameMod+",";
        }
        closedir(IdDir);
    } while(id != string::npos);
}
	    
bool TModSchedul::CheckFile(char * name, bool new_f)
{
    struct stat file_stat;
    string NameMod;

    stat(name,&file_stat);

    if( (file_stat.st_mode&S_IFMT) != S_IFREG ) return(false);
    if( access(name,F_OK|R_OK|X_OK) != 0 )      return(false);
    NameMod=name;

    void *h_lib = dlopen(name,RTLD_GLOBAL|RTLD_LAZY);
    if(h_lib == NULL)
    {
        App->Mess->put(2, "File %s error: %s !",name,dlerror());
        return(false);
    }
    else dlclose(h_lib);
    if(new_f)
	for(unsigned i_sh=0; i_sh < SchHD.size(); i_sh++)
	    if(SchHD[i_sh].path == name && SchHD[i_sh].modif == file_stat.st_mtime) 
		return(false);

    return(true);
}

int TModSchedul::AddShLib( char *name, int dest )
{
    struct stat file_stat;
    string NameTMod;
    TModule *LdMod;
    int n_mod, add_mod, id;

    if( CheckFile(name,true) != true ) return(0);

    void *h_lib = dlopen(name,RTLD_GLOBAL|RTLD_LAZY);
    TModule *(*attach)(char *, int);
    (void *)attach = dlsym(h_lib,"attach");
    if(dlerror() != NULL)
    {
        App->Mess->put(2, "File %s error: %s !",name,dlerror());
        dlclose(h_lib);
        return(0);
    }
    n_mod=0, add_mod=0;
    while((LdMod = (attach)(name, n_mod++ )) != NULL )
    {
        LdMod->info("NameType",NameTMod);
	if(dest < 0)
	{
	    for( unsigned i_grm=0; i_grm < grpmod.size(); i_grm++)
		if(NameTMod == grpmod[i_grm]->NameTMod())
		{ 
		    id = grpmod[i_grm]->AddM(LdMod);
		    if(id >= 0)
		    {
    			RegMod_ShLb(h_lib, name, file_stat.st_mtime, i_grm, id );
			add_mod++;
		    }
		    break;
		}
	}
	else
	{
	    if(NameTMod == grpmod[dest]->NameTMod())
	    { 
		id = grpmod[dest]->AddM(LdMod);
		if(id >= 0)
		{
		    RegMod_ShLb(h_lib, name, file_stat.st_mtime, dest, id );
		    add_mod++;
		}
	    } 
	}
    }
    if(add_mod == 0) dlclose(h_lib);

    return(add_mod);
}

int TModSchedul::RegMod_ShLb(const void* hd, char *path, time_t modif, int id_tmod, int id_mod )
{
    //Add to alredy registry share lib
    for(int i=0; i < SchHD.size(); i++)
        if(SchHD[i].hd == hd)
        {
            for(unsigned i_use=0; i_use < SchHD[i].use.size(); i_use++)
		if(SchHD[i].use[i_use].id_tmod == id_tmod && SchHD[i].use[i_use].id_mod == id_mod)
		    return(i);
       	    SchHD[i].use.push_back();
       	    SchHD[i].use[SchHD[i].use.size() -1].id_tmod = id_tmod; 
       	    SchHD[i].use[SchHD[i].use.size() -1].id_mod  = id_mod; 
            return(i);
        }
    //Regystry new share lib
    int i_sh = SchHD.size();
    SchHD.push_back( );
    int i_use = SchHD[i_sh].use.size();
    SchHD[i_sh].use.push_back();

    SchHD[i_sh].hd = (void *)hd;
    SchHD[i_sh].use[i_use].id_tmod = id_tmod;
    SchHD[i_sh].use[i_use].id_mod  = id_mod;
    SchHD[i_sh].modif = modif;
    SchHD[i_sh].path = path;

    return(i_sh);
}

int TModSchedul::UnRegMod_ShLb(int id_tmod, int id_mod)
{
    for(unsigned i_sh = 0; i_sh < SchHD.size(); i_sh++)
	for(unsigned i_use=0; i_use < SchHD[i_sh].use.size(); i_use++)
	    if(SchHD[i_sh].use[i_use].id_tmod == id_tmod && SchHD[i_sh].use[i_use].id_mod == id_mod)
	    {
		SchHD[i_sh].use.erase(SchHD[i_sh].use.begin()+i_use);
		if(SchHD[i_sh].use.size() == 0)
		{ 
		    dlclose(SchHD[i_sh].hd);
		    SchHD.erase(SchHD.begin()+i_sh);
		    return(0);
		}
	    }
    return(-1);
}

