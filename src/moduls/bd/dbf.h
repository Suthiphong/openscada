#ifndef DBF_H
#define DBF_H

#include <string>
using std::string;

struct db_head
{
    char ver;			//������ DBF (def = 3)
    char data[3];		//���� �����������
    long numb_rec;		//���������� �������
    short len_head;		//����� ���������
    short len_rec;		//����� ������
    char res[20];
};

struct db_str_rec
{
    char name[11];		//��� ����
    char tip_fild;		//��� ���� (C - ASCII; N - �����; L - ����������; M - Memo; D - Data)
    long adr_in_mem;
    char len_fild;		//����� ����
    char dec_field;		//������ ����� "."
    char res[14];
};

//������ ���� DBF

class TBasaDBF
{
  public:
    TBasaDBF(  );
    ~TBasaDBF(  );

    int LoadFields( db_str_rec * fields, int number );
    int addField( int pos, db_str_rec * field_ptr );
    int DelField( int pos );
    int DelField( char *NameField );
    db_str_rec *getField( int posField );
    db_str_rec *getField( char *NameField );
    int setField( int posField, db_str_rec *attr );
    int setField( char *NameField, db_str_rec *attr );
    int CreateItems( int pos );
    int DeleteItems( int pos, int fr );
    void *getItem( int posItem );
    void AddItem( int posItem, void *it );
    int ModifiFieldIt( int posItems, int posField, char *str );
    int ModifiFieldIt( int posItems, char *NameField, char *str );
    int GetFieldIt( int posItems, int posField, string & str );
    int GetFieldIt( int posItems, char *NameField, string & str );
    int GetCountItems(  );
    int SaveFile( char *Name );
    int LoadFile( char *Name );
  protected: 
    db_head * db_head_ptr;
				//��������� �� ���������
    db_str_rec *db_field_ptr;	//��������� �� ���� ���� ������
    void **items;		//������
};

//----------------------------------------------------------------------

#endif // DBF_H

