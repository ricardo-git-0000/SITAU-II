#ifndef cxmlH
#define cxmlH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

#include <vector>
#include <fstream>

using namespace std;

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "pugixml.hpp"
#include "tad.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define XML_tArrayFloat		"tArrayFloat"

#define XML_tString			"tString"
#define tStringParameter         "tStringParameter"

#define XML_tInt16			"tInt16"

#define XML_tInt32			"tInt32"
#define tIntegerParameter        "tIntegerParameter"

#define XML_tUInt32			"tUInt32"

#define XML_tFloat			"tFloat"
#define tFloatParameter          "tFloatParameter"

#define XML_tBoolean		"tBoolean"
#define tBooleanParameter        "tBooleanParameter"

#define XML_tArrayUInt16	"tArrayUInt16"
#define tArrayUShortParameter    "tArrayUShort"

//#define tInt32                   "tInt32"
//#define tfalse                   "false"
//#define ttrue                    "true"

#define XML_tfalse                     "false"
#define XML_ttrue                      "true"

#define XML_pStartArrayElement       "pStartArrayElement"
#define XML_pName                    "pName"
#define XML_pType                    "pType"
#define XML_pValue                   "pValue"

// _____________________________________________________________________________
// ------ CXMLParameter class
// _____________________________________________________________________________
// =============================================================================
template <typename T>
class CXMLParameter
{
private:
   T value_;
   bool update_;

public:

   CXMLParameter()  {update_ = false;}

	~CXMLParameter() {}

   void Set (T value) {value_ = value; update_ = true;}

   void Reset(T value) { value_ = value; update_ = false; }

   void Resize(int n_data) {value_.resize(n_data);}

   void PushBack_UInt16(int value) {value_.push_back(value);}

   void PushBack_UInt8(unsigned char value) {value_.push_back(value);}

   int Size(void) {return (int)value_.size();}

   void Clear(void) {value_.clear();}

   //void Assign_UInt16(vector<unsigned short> &data) {data.assign(value_.begin(), value_.end());}

   //void Assign_UInt8(vector<unsigned char> &data) {data.assign(value_.begin(), value_.end());}

   void Assign(T &data) { data.assign(value_.begin(), value_.end()); }

   T Value (bool *update=NULL) {if (update != NULL) *update = update_; return value_;}

   void Value(T value) { value_ = value; }

   bool Update (void) {return update_;}
};

// _____________________________________________________________________________
// ------ CXML class
// _____________________________________________________________________________
// =============================================================================
class CXML
{
private:

	string GetStrInt(string src);
	string GetStrUInt(string src);
	string GetStrFloat(string src);

protected:
  
	std::ofstream logFile;
	pugi::xml_document doc_;
	pugi::xml_parse_result xml_result_;
	AnsiString msg_err_;
	AnsiString file_name_;

	void LogWriteLine(char* msg, ...);
	void LogWrite(char* msg, ...);
	void LogWriteStartLine(char* msg, ...);
	void LogWriteEndLine(void);

public:

	CXML();

	~CXML();

	int LoadFile(AnsiString file_name);

	int LoadBuffer(vector<char> &file_buffer);

	int GetError(void);

	bool GetAttribute(pugi::xml_attribute *attribute, char *name, CXMLParameter<float> *parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<int>* parameter);
	
	bool GetAttribute(pugi::xml_attribute *attribute, char *name, CXMLParameter<AnsiString> *parameter);

	bool GetAttribute(pugi::xml_attribute *attribute, char *name, AnsiString *parameter);

	bool GetAttribute(pugi::xml_attribute *attribute, char *name, CXMLParameter<vector<float> > *parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<vector<int> >* parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned long>* parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned short>* parameter);

	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned char>* parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<vector<unsigned short> >* parameter);

	bool GetAttribute(pugi::xml_attribute *attribute, char *name, vector<float> &parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, vector<int>& parameter); 
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, vector<unsigned short>& parameter);

	bool GetAttribute(pugi::xml_attribute *attribute, char *name, CXMLParameter<bool> *parameter);

	bool GetAttribute(pugi::xml_attribute* attribute, char* name, int* parameter);
	
	bool GetAttribute(pugi::xml_attribute* attribute, char* name, bool* parameter);
};


string XML_Array2Attribute(CXMLParameter<vector<float> >* data, int decimal_places=2);
string XML_Array2Attribute(CXMLParameter<vector<int> >* data);
string XML_Array2Attribute(CXMLParameter<vector<unsigned short> >* data);
string XML_Array2Attribute(vector<float>& data, int decimal_places=2);
string XML_Array2Attribute(vector<int>& data);
string XML_Array2Attribute(vector<unsigned short>& data);

#endif
