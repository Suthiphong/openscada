
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <iconv.h>

#include "tmessage.h"
#include "tapplication.h"

TMessage::TMessage(  ) : stop_signal(0)
{
    openlog("OpenScada",0,LOG_USER);    
}

void TMessage::sighandler( int signal )
{
    if(signal==SIGINT) 
    { 
//	App->Mess->put(3,"Have get a Interrupt signal. No stop server!");
	App->Mess->stop_signal=signal; 
    }
    if(signal==SIGTERM) 
    { 
	App->Mess->put(3,"Have get a Terminate signal. Server been stoped!"); 
	App->Mess->stop_signal=signal; 
    }
}



TMessage::~TMessage(  )
{
    closelog();
}

int TMessage::Start(  )
{
    struct sigaction sa;
    memset (&sa, 0, sizeof(sa));
    sa.sa_handler= sighandler;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTERM,&sa,NULL);
    
    while(1)	
	if(stop_signal) break;   
       	else            sleep(100); 

//    sa.sa_handler= SIG_DFL;
//    sigaction(SIGINT,&sa,NULL);
//    sigaction(SIGTERM,&sa,NULL);        

    return(stop_signal);       
}

// ������� ������� (App->d_level) ����� ���������� � �������� 0-8 ������������:
// 0 - �� �������� ������� ��������� ������ 
// 8 - ������������ ������� �������
// ������� ��������� (level) ������������� ��� �������������� � ���������� � �������� 0-7:
// 0 - ���������� �������;
// 7 - ������� ���������� ������� ������;

void TMessage::put( int level, char * fmt,  ... )
{
    char str[256];                  //!!!!
    va_list argptr;

    va_start (argptr,fmt);
    vsprintf(str,fmt,argptr);
    va_end(argptr);
    if(level<0) level=0; if(level>7) level=7;
    if(level>=(8-App->d_level)) 
    {
	int level_sys=LOG_DEBUG;
	if(level<1)       level_sys=LOG_DEBUG;
	else if(level==1) level_sys=LOG_INFO;
	else if(level==2) level_sys=LOG_NOTICE;
	else if(level==3) level_sys=LOG_WARNING;
	else if(level==4) level_sys=LOG_ERR;
	else if(level==5) level_sys=LOG_CRIT;
	else if(level==6) level_sys=LOG_ALERT;
	else if(level==7) level_sys=LOG_EMERG;
	if(App->log_dir&1) syslog(level_sys,str);
	if(App->log_dir&2) fprintf(stdout,"%s \n",str);
	if(App->log_dir&4) fprintf(stderr,"%s \n",str);
    }
}

int TMessage::SconvIn(const char *fromCH, string & buf)
{
    return( Sconv(fromCH, App->IOCharSet().c_str(), buf) );
}    
int TMessage::SconvOut(const char *toCH, string & buf)
{
    return( Sconv( App->IOCharSet().c_str(), toCH , buf) );
}

int TMessage::Sconv(const char *fromCH, const char *toCH, string & buf)
{
    //Make convert to blocks 100 bytes !!!    
    char   *ibuf, outbuf[100], *obuf;
    size_t ilen, olen;
    iconv_t hd;
    
    hd = iconv_open(toCH, fromCH);
    if( hd == (iconv_t)(-1) ) return(-1);
    
    string t_buf = buf; 
    buf.erase();
    ibuf = (char *)t_buf.c_str();
    ilen = t_buf.size();
    
    while(ilen)
    {
	obuf = outbuf;
	olen = sizeof(outbuf)-1;
	iconv(hd,&ibuf,&ilen,&obuf,&olen);
	buf.append(outbuf,sizeof(outbuf)-1-olen);
    }
    iconv_close(hd);
    
    return(0);
}

