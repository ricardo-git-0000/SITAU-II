// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "cxml.h"
#include "c_log_file.h"
#include "xml_err.h"
#include "utils.h"
#include "fileformat.h"

// _____________________________________________________________________________
// ------ CXML class Methods
// _____________________________________________________________________________
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML::CXML()
{
   file_name_ = "No XML File";
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
CXML::~CXML()
{
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CXML::LogWriteLine(char* msg, ...)
{
    va_list argptr;
    time_t t;
    char str[255];
    char date[255];
    char msg_formated[255];

    va_start(argptr, msg);
    if (msg != nullptr)
    {
        time(&t);
        strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", localtime(&t));
        vsprintf(msg_formated, msg, argptr);
        sprintf(str, "[%s] %s", date, msg_formated);
        logFile << str << std::endl;
    }
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CXML::LogWriteStartLine(char* msg, ...)
{
    va_list argptr;
    time_t t;
    char str[255];
    char date[255];
    char msg_formated[255];

    va_start(argptr, msg);
    if (msg != nullptr)
    {
        time(&t);
        strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", localtime(&t));
        vsprintf(msg_formated, msg, argptr);
        sprintf(str, "[%s] %s", date, msg_formated);
        logFile << str;
    }
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CXML::LogWriteEndLine(void)
{
    logFile << std::endl;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void CXML::LogWrite(char* msg, ...)
{
    va_list argptr;
    char str[255];
    char msg_formated[255];

    va_start(argptr, msg);
    if (msg != nullptr)
    {
        vsprintf(msg_formated, msg, argptr);
        sprintf(str, "%s", msg_formated);
        logFile << str;
    }
    va_end(argptr);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML::LoadFile(AnsiString file_name)
{
FILE *f = NULL;
char *buffer = NULL;
size_t size_file, read_size;
int result;

   msg_err_ = "";
   file_name_ = file_name;
   if (file_name == "") return ELOG(charEXML_FILE, EXML_FILE);
   try
   {
      if ((f = fopen(file_name.c_str(), "rb")) != NULL)
      {
         fseek(f, 0, SEEK_END);
         size_file = ftell(f);
         if (size_file <= 0) ELOG(charEXML_FILE, EXML_FILE);
         fseek(f, 0, SEEK_SET);
         if (size_file > 0)
         {
            if ((buffer = (char*)Malloc(size_file + 100)) != NULL)
            {
                for (int i = 0; i < size_file+100; i++) buffer[i] = 0;
               read_size = fread(buffer, 1, size_file, f);
               if (read_size != size_file) 
               {
                   //// ------------------------------------------------------
                   //AnsiString str = FS_ChangeFileExtension(file_name, "");
                   //str = FS_ChangeFileExtension(str + "_DBG", ".XML");
                   //FILE* f_dbg = fopen(str.c_str(), "wt");
                   //if (f_dbg != NULL)
                   //{
                   //    fwrite(buffer, read_size, 1, f_dbg);
                   //    fclose(f_dbg);
                   //}
                   //// ------------------------------------------------------
                   return ELOG(charEXML_FILE, EXML_FILE);
               }
               xml_result_ = doc_.load_buffer(buffer, read_size);
               free(buffer);
            }
            else return ELOG(charEXML_NoMemory, EXML_NoMemory);
         }
         else return ELOG(charEXML_FILE, EXML_FILE);
         if ((result = GetError()) < 0) RLOG(result);         
      }
      else
      {
         xml_result_ = doc_.load_file(file_name_.c_str());
         if ((result = GetError()) < 0) RLOG(result);
      }
   }
   finally
   {
      if (f != NULL) fclose(f);
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML::LoadBuffer(vector<char> &file_buffer)
{
    int result;

    msg_err_ = "";
    file_name_ = "";
    if (file_buffer.size() == 0) return ELOG(charEXML_FILE, EXML_FILE);
    xml_result_ = doc_.load_buffer(&file_buffer[0], file_buffer.size());
    if ((result = GetError()) < 0) RLOG(result);
    return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int CXML::GetError(void)
{
   if (!(xml_result_))
   {
      CXML::msg_err_ = "XML File '" + CXML::file_name_ + "' Error description: " + CXML::xml_result_.description();
      return (int)ELOG((char *)CXML::msg_err_.c_str(), (int)EXML_FILE);
   }
   return EXML_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<float> *parameter)
{
bool result = false, flag_end = false;
pugi::xml_attribute next_attribute;
AnsiString str;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = GetStrFloat(AnsiString(next_attribute.value()));
                if (str.length() > 0)
                {
                    parameter->Set(std::stof(str));
                    result = true;
                }
                //str = AnsiString(next_attribute.value());
                //for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                //{
                //    if (str[i] == ',')
                //    {
                //        str[i] = '.';
                //        flag_end = true;
                //    }
                //}
                //data = std::stof(str);
                //parameter->Set(data);
                //result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute* attribute, char* name, CXMLParameter<int>* parameter)
{
    bool result = false, flag_end = false;
    pugi::xml_attribute next_attribute;
    AnsiString str, str_value;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = GetStrInt(AnsiString(next_attribute.value()));
                if (str.length() > 0)
                {
                    parameter->Set(std::stoi(str));
                    result = true;
                    LogWriteLine("   '%s': %s", name, str.c_str());
                }
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned long>* parameter)
{
    bool result = false, flag_end = false;
    pugi::xml_attribute next_attribute;
    AnsiString str, str_value;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0)
                {
                    parameter->Set(std::stoul(str));
                    result = true;
                    LogWriteLine("   '%s': %s", name, str.c_str());
                }

                //str = AnsiString(next_attribute.value());
                //if (str == XML_tfalse)
                //{
                //    parameter->Set(0);
                //    result = true;
                //}
                //else if (str == XML_ttrue)
                //{
                //    parameter->Set(1);
                //    result = true;
                //}
                //else
                //{
                //    for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                //    {
                //        if ((str[i] >= '0' && str[i] <= '9') || str[i] <= '-') str_value.insert(str_value.length(), 1, str[i]);
                //        else if (str[i] == ',') flag_end = true;
                //    }
                //    if (str_value.length() > 0)
                //    {
                //        data = std::stoul(str_value);
                //        parameter->Set(data);
                //        result = true;
                //    }
                //}
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned short>* parameter)
{
    bool result = false, flag_end = false;
    pugi::xml_attribute next_attribute;
    AnsiString str, str_value;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0)
                {
                    parameter->Set(std::stoi(str));
                    result = true;
                    LogWriteLine("   '%s': %s", name, str.c_str());
                }
                //for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                //{
                //    if (str[i] == ',')
                //    {
                //        str[i] = '.';
                //        flag_end = true;
                //    }
                //}
                //data = std::stoi(str);
                //parameter->Set(data);
                //result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<unsigned char>* parameter)
{
    bool result = false, flag_end = false;
    pugi::xml_attribute next_attribute;
    AnsiString str, str_value;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0)
                {
                    parameter->Set(std::stoi(str));
                    result = true;
                    LogWriteLine("   '%s': %s", name, str.c_str());
                }
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, AnsiString *parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                *parameter = AnsiString(next_attribute.value());
                result = true;
                LogWriteLine("   '%s': %s", name, *parameter);
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute *attribute, char *name, CXMLParameter<AnsiString> *parameter)
{
bool result = false, flag_end = false;
pugi::xml_attribute next_attribute;
AnsiString str;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                str = next_attribute.value();
                parameter->Set(str);
                result = true;
                LogWriteLine("   '%s': %s", name, str.c_str());
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<vector<float> > *parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;
int i, start_element = 0;
AnsiString str, str_value;
bool flag_end;
vector<float> data;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (data.size() < start_element) data.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) data.push_back(std::stof(str_value));
                        parameter->Set(data);
                        flag_end = true;
                        LogWriteStartLine("   '%s': ", name);
                        for (int i = 0; i < data.size(); i++) LogWrite("%.3f ", data[i]);
                        LogWriteEndLine();
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0) data.push_back(std::stof(str_value));
                        str_value = "";
                    }
                    else if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<vector<int> >* parameter)
{
    bool result = false;
    pugi::xml_attribute next_attribute;
    int i, start_element = 0;
    AnsiString str, str_value;
    bool flag_end;
    vector<int> data;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (data.size() < start_element) data.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) data.push_back((int)std::stoi(str_value));
                        parameter->Set(data);
                        flag_end = true;
                        LogWriteStartLine("   '%s': ", name);
						for (int i = 0; i < data.size(); i++) LogWrite("%d ", data[i]);
                        LogWriteEndLine();
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0)data.push_back((int)std::stoi(str_value));
                        str_value = "";
                    }
                    else if (str[i] >= '0' && str[i] <= '9') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, CXMLParameter<vector<unsigned short> >* parameter)
{
    bool result = false;
    pugi::xml_attribute next_attribute;
    int i, start_element = 0;
    AnsiString str, str_value;
    bool flag_end;
    vector<unsigned short> data;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (data.size() < start_element) data.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) data.push_back((unsigned short)std::stoi(str_value));
                        parameter->Set(data);
                        flag_end = true;
                        LogWriteStartLine("   '%s': ", name);
                        for (int i = 0; i < data.size(); i++) LogWrite("%d ", data[i]);
                        LogWriteEndLine();
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0) data.push_back((unsigned short)std::stoi(str_value));
                        str_value = "";
                    }
                    else if (str[i] >= '0' && str[i] <= '9') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, vector<float> &parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;
int i, start_element = 0;
AnsiString str, str_value;
bool flag_end;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (parameter.size() < start_element) parameter.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) parameter.push_back(std::stof(str_value));
                        flag_end = true;
                        LogWriteLine("   '%s': %s", name, str_value);
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0) parameter.push_back(std::stof(str_value));
                        str_value = "";
                    }
                    else if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute(pugi::xml_attribute* attribute, char* name, vector<int>& parameter)
{
    bool result = false;
    pugi::xml_attribute next_attribute;
    int i, start_element = 0;
    AnsiString str, str_value;
    bool flag_end;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (parameter.size() < start_element) parameter.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) parameter.push_back((int)std::stoi(str_value));
                        flag_end = true;
                        LogWriteLine("   '%s': %s", name, str_value);
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0) parameter.push_back((int)std::stoi(str_value));
                        str_value = "";
                    }
                    else if (str[i] >= '0' && str[i] <= '9') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, vector<unsigned short> &parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;
int i, start_element = 0;
AnsiString str, str_value;
bool flag_end;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pStartArrayElement) == 0)
            {
                str = GetStrUInt(AnsiString(next_attribute.value()));
                if (str.length() > 0) start_element = std::stoi(str);
                next_attribute = next_attribute.next_attribute();
            }
            else if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                if (parameter.size() < start_element) parameter.resize(start_element);
                str = AnsiString(next_attribute.value());
                for (i = 1, flag_end = false; !flag_end && i <= str.length(); i++)
                {
                    if (str[i] == ']')
                    {
                        if (str_value.length() > 0) parameter.push_back((unsigned short)std::stoi(str_value));
                        flag_end = true;
                        LogWriteLine("   '%s': %s", name, str_value);
                    }
                    else if (str[i] == ',')
                    {
                        if (str_value.length() > 0) parameter.push_back((unsigned short)std::stoi(str_value));
                        str_value = "";
                    }
                    else if (str[i] >= '0' && str[i] <= '9') str_value.insert(str_value.length(), 1, str[i]);
                }
                result = true;
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, CXMLParameter<bool> *parameter)
{
bool data, result = false;
pugi::xml_attribute next_attribute;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                strcmp(next_attribute.value(), XML_tfalse) == 0 ? data = false : data = true;
                parameter->Set(data);
                result = true;
                LogWriteLine("   '%s': %d", name, data ? 1 : 0);
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, int *parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                *parameter = std::stoi(AnsiString(next_attribute.value()));
                result = true;
                LogWriteLine("   '%s': %d", name, *parameter);  
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
bool CXML::GetAttribute (pugi::xml_attribute *attribute, char *name, bool *parameter)
{
bool result = false;
pugi::xml_attribute next_attribute;

    if (strcmp(attribute->value(), name) == 0)
    {
        next_attribute = attribute->next_attribute();
        while (!result && next_attribute != NULL)
        {
            if (strcmp(next_attribute.name(), XML_pValue) == 0)
            {
                strcmp(next_attribute.value(), XML_tfalse) == 0 ? *parameter = false : *parameter = true;
                result = true;
                LogWriteLine("   '%s': %d", name, *parameter ? 1 : 0);
            }
            else next_attribute = next_attribute.next_attribute();
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string CXML::GetStrInt(string src)
{
    string result;
    int i;

    result.clear();
    if (src == XML_tfalse) result = "0";
    else if (src == XML_ttrue) result = "1";
    else
    {
        for (i = 0; i < src.length(); i++)
        {
            if ((src[i] >= '0' && src[i] <= '9') || src[i] == '-') result.insert(result.length(), 1, src[i]);
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string CXML::GetStrUInt(string src)
{
    string result;
    int i;

    result.clear();
    if (src == XML_tfalse) result = "0";
    else if (src == XML_ttrue) result = "1";
    else
    {
        for (i = 0; i < src.length(); i++)
        {
            if (src[i] >= '0' && src[i] <= '9') result.insert(result.length(), 1, src[i]);
        }
    }
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string CXML::GetStrFloat(string src)
{
    string result;
    int i;

    result.clear();
    if (src == XML_tfalse) result = "0";
    else if (src == XML_ttrue) result = "1";
    else
    {
        for (i = 0; i < src.length(); i++)
        {
            if ((src[i] >= '0' && src[i] <= '9') || src[i] == '-' || src[i] == '.') result.insert(result.length(), 1, src[i]);
            else if (src[i] == ',') result.insert(result.length(), 1, '.');
        }
    }
    return result;
}










// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(CXMLParameter<vector<float> > *data, int decimal_places)
{
    string result;
    char s[255];

    result = "[";
    for (int i = 0; i < data->Value().size(); i++)
    {
        if (i < data->Value().size() - 1) sprintf(s, "%.*f,", decimal_places, data->Value()[i]);
        else sprintf(s, "%.*f", decimal_places, data->Value()[i]);
        result += s;
    }
    result += "]";
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(CXMLParameter<vector<int> >* data)
{
    string result;
    char s[255];

    result = "[";
    for (int i = 0; i < data->Value().size(); i++)
    {
        if (i < data->Value().size() - 1) sprintf(s, "%d,", data->Value()[i]);
        else sprintf(s, "%d", data->Value()[i]);
        result += s;
    }
    result += "]";
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(CXMLParameter<vector<unsigned short> >* data)
{
    string result;
    char s[255];

    result = "[";
    for (int i = 0; i < data->Value().size(); i++)
    {
        if (i < data->Value().size() - 1) sprintf(s, "%d,", data->Value()[i]);
        else sprintf(s, "%d", data->Value()[i]);
        result += s;
    }
    result += "]";
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(vector<float> &data, int decimal_places)
{
    string result;
    char s[20];

    result = "[";
    for (int i = 0; i < data.size(); i++)
    {
        if (i < data.size() - 1) sprintf(s, "%.*f,", decimal_places, data[i]);
        else sprintf(s, "%.*f", decimal_places, data[i]);
        result += s;
    }
    result += "]";
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(vector<int> &data)
{
    string result;
    char s[255];

    result = "[";
    for (int i = 0; i < data.size(); i++)
    {
        if (i < data.size() - 1) sprintf(s, "%d,", data[i]);
        else sprintf(s, "%d", data[i]);
    }
    result += "]";
    return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
string XML_Array2Attribute(vector<unsigned short> &data)
{
    string result;
    char s[255];

    result = "[";
    for (int i = 0; i < data.size(); i++)
    {
        if (i < data.size() - 1) sprintf(s, "%d,", data[i]);
        else sprintf(s, "%d", data[i]);
    }
    result += "]";
    return result;
}
