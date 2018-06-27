/*Copyright 2018 kakadila

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include "XXJson.h"
#include <string.h>
#include <sstream>    //使用stringstream需要引入这个头文件 
using namespace std;


#define SKIP_SPACE(x) while(*x < 32 && *x > 0) ++x;

#define PARSE_CHECK_NULL(x) if(x == NULL) return NULL;
#define ERROR_AT_PRINT(x) {printf("line : %d error at %s\n",__LINE__,x); return NULL;}

JsonItem::JsonItem()
{
        m_type = JSON_INVALID;
        m_need_release = true;
}

JsonItem::~JsonItem()
{
        if(m_need_release)
                ClearItem(this);
}


JsonItem & JsonItem::operator[]( const char *key )
{
        if(m_type == JSON_INVALID)
        {
                m_type = JSON_OBJECT;
        }else if(m_type != JSON_OBJECT)
        {
                throw JSON_ERR_TYPE;
        }

        JsonObject::iterator it = m_object_value.find(key);
        JsonItem *item = NULL;
        if(it == m_object_value.end())
        {
                item = new JsonItem();
                m_object_value[key] = item;
        }
        else
        {
                item = it->second;
        }
        return *item;
}

JsonItem & JsonItem::operator[]( int idx )
{
        if(m_type != JSON_INVALID)
        {
                throw JSON_ERR_TYPE;
        }

        JsonItem *item = NULL;
        if(idx >= m_array_value.size())
        {
                throw JSON_ERR_ARRAY_SIZE;
        }

        item = m_array_value[idx];
        return *item;
}

const char * JsonItem::ToString()
{
        if (m_type == JSON_INVALID)
        {
                m_tostring = "{}";
        }
        else
        {
                char buf[512];
                m_tostring = "";
                if (m_type == JSON_NULL)
                {
                        m_tostring.append("null");
                }
                if (m_type == JSON_STRING)
                {
                        m_tostring.append("\"").append(m_string_value).append("\"");
                }
                if (m_type == JSON_INT)
                {
                        sprintf(buf, "%lld", m_value.int_value);
                        m_tostring.append(buf);
                }
                else if (m_type == JSON_UINT)
                {
                        sprintf(buf, "%llu", m_value.uint_value);
                        m_tostring.append(buf);
                }
                else if (m_type == JSON_BOOL)
                {
                        m_tostring.append(m_value.bool_value ? "true" : "false");
                }
                else if (m_type == JSON_DOUBLE)
                {
                        sprintf(buf, "%lf", m_value.double_value);
                        m_tostring.append(buf);
                }
                else if (m_type == JSON_ARRAY)
                {
                        m_tostring.append("[");
                        int num = Size();
                        for (int i = 0; i < num-1; ++i)
                        {
                                m_tostring.append(m_array_value[i]->ToString());
                                m_tostring.append(",");
                        }
                        m_tostring.append(m_array_value[num-1]->ToString()); //the last one
                        m_tostring.append("]");
                }
                else if (m_type == JSON_OBJECT)
                {
                        m_tostring.append("{");
                        JsonObject::iterator it;
                        for (it = m_object_value.begin();
                                it != m_object_value.end();
                                ++it)
                        {
                                sprintf(buf, "\"%s\":", it->first.c_str());
                                m_tostring.append(buf);
                                m_tostring.append(it->second->ToString());
                                m_tostring.append(",");
                        }
                        m_tostring.pop_back();
                        m_tostring.append("}");
                }
        }

        return m_tostring.c_str();
}

void JsonItem::SetNull()
{
        ClearItem(this);
        m_type = JSON_NULL;
}

void JsonItem::PushNull()
{
        if (m_type == JSON_INVALID)
        {
                m_type = JSON_ARRAY;
        }
        else if (m_type != JSON_ARRAY)
        {
                throw JSON_ERR_TYPE;
        }
        JsonItem *item = new JsonItem();
        item->SetNull();
        m_array_value.push_back(item);
}

void JsonItem::SetObject()
{
        ClearItem(this);
        m_type = JSON_OBJECT;
}

void JsonItem::SetArray()
{
        ClearItem(this);
        m_type = JSON_ARRAY;
}

long long JsonItem::GetInt()
{
        if ((m_type != JSON_INT && m_type != JSON_UINT))
        {
                throw JSON_ERR_TYPE;
        }

        if (m_type == JSON_INT)
        {
                if (m_value.int_value > 0x7fffffffffffffff)
                        throw JSON_ERR_TYPE;
                return m_value.int_value;
        }
        else
        {
                return m_value.uint_value;
        }
}

unsigned long long JsonItem::GetUInt()
{
        if ((m_type != JSON_INT && m_type != JSON_UINT))
        {
                throw JSON_ERR_TYPE;
        }
        if (m_type == JSON_INT)
        {
                if (m_value.int_value < 0)
                        throw JSON_ERR_TYPE;
                return m_value.int_value;
        }
        else
        {
                return m_value.uint_value;
        }
}

double JsonItem::GetDouble()
{
        if (m_type != JSON_DOUBLE)
                throw JSON_ERR_TYPE;

        return m_value.double_value;
}

bool JsonItem::GetBool()
{
        if (m_type != JSON_BOOL)
                throw JSON_ERR_TYPE;

        return m_value.bool_value;
}

const char *JsonItem::GetString()
{
        if (m_type != JSON_STRING)
                throw JSON_ERR_TYPE;

        return m_string_value.c_str();
}

int JsonItem::Size()
{
        if (m_type != JSON_ARRAY)
                throw JSON_ERR_TYPE;
        return m_array_value.size();
}

int JsonItem::GetType()
{
        return m_type;
}

bool JsonItem::Parse(std::string str)
{
	return Parse(str.c_str());
}

bool JsonItem::Parse(const char * str)
{
        ClearItem(this);

        SKIP_SPACE(str);
        switch (*str)
        {
        case '\0':
                return m_type == JSON_ARRAY || m_type == JSON_OBJECT;
        case '{':
        {
                str = ParseObect(str + 1);
                if (str == NULL)
                {
                        return false;
                }
                else if (*str == 0)
                {
                        return m_type == JSON_OBJECT;
                }
        }
        case '[':
        {
                str = ParseArray(str + 1);
                if (str == NULL)
                {
                        return false;
                }
                else if (*str == 0)
                {
                        return m_type == JSON_ARRAY;
                }
        }
        default:
                return false;
        }

        return false;
}

JsonItem & JsonItem::operator=(JsonItem & value)
{
        ClearItem(this);
        m_type = value.m_type;
        if (m_type == JSON_OBJECT)
                m_object_value = value.m_object_value;
        else if (m_type == JSON_ARRAY)
                m_array_value = value.m_array_value;
        else if (m_type == JSON_STRING)
                m_string_value = value.m_string_value;
        else if (m_type != JSON_INVALID)
                m_value = value.m_value;

        value.m_need_release = false;
        return *this;
}

JsonItem & JsonItem::operator=(std::string value)
{
        return JsonItem::operator=(value.c_str());
}

JsonItem & JsonItem::operator=(const char * value)
{
        ClearItem(this);
        m_type = JSON_STRING;
        m_string_value = value;
        return *this;
}

void JsonItem::ClearItem(JsonItem * item)
{
        if (item->m_type == JSON_ARRAY)
        {
                int num = item->Size();
                for (int i = 0; i < num; ++i)
                {
                        JsonItem *obj = item->m_array_value[i];
                        ClearItem(obj);
                        delete obj;
                }
                item->m_array_value.clear();
        }
        else if (item->m_type == JSON_OBJECT)
        {
                JsonObject::iterator it;
                for (it = item->m_object_value.begin();
                        it != item->m_object_value.end(); ++it)
                {
                        JsonItem *obj = it->second;
                        ClearItem(obj);
                        delete obj;
                }
                item->m_object_value.clear();
        }
        m_type = JSON_NULL;
}

const char *JsonItem::ParseObect(const char * str)
{
        m_type = JSON_OBJECT;

        const char *key_start = str;
        while (1)
        {
                SKIP_SPACE(key_start);
                if (*key_start != '\"')
                {
                        if (*key_start == '}') return key_start + 1;
                        else return NULL;
                }
                ++key_start;

                const char *key_end = strchr(key_start, '\"');
                PARSE_CHECK_NULL(key_end);

                const char *split = strchr(key_end + 1, ':');
                PARSE_CHECK_NULL(split);

                std::string key;
                key.append(key_start, key_end - key_start);
                JsonItem *item = new JsonItem();
                m_object_value[key] = item;

                str = item->ParseValue(split + 1);
                if (str == NULL)
                {
                        return NULL;
                }

                key_start = str;
                while (*key_start != '}' && *key_start != ',')
                {
                        if (*key_start == '\0')
                                return NULL;
                        ++key_start;
                }
                if (*key_start == '}')
                {
                        return key_start + 1;
                }
                ++key_start;
        }
        return NULL;
}

const char *JsonItem::ParseArray(const char * str)
{
        m_type = JSON_ARRAY;

        const char *key_start = str;
        while (1)
        {
                SKIP_SPACE(key_start);
                if (*key_start == ']')
                        return key_start + 1;
                else if (*key_start == '\0')
                        return NULL;

                JsonItem *item = new JsonItem();
                m_array_value.push_back(item);
                str = item->ParseValue(key_start);
                if (str == NULL)
                {
                        return NULL;
                }

                key_start = str;
                while (*key_start != ']' && *key_start != ',')
                {
                        if (*key_start == '\0')
                                return NULL;
                        ++key_start;
                }
                if (*key_start == ']')
                {
                        return key_start + 1;
                }
                ++key_start;
        }
        return NULL;
}

const char *JsonItem::ParseValue(const char * str)
{
        SKIP_SPACE(str);
        switch (*str)
        {
        case '{':
                return ParseObect(str + 1);
        case '[':
                return ParseArray(str + 1);
        case '\"':
        {
                //string
                const char *end = strchr(str + 1, '\"');
                if (end == NULL)
                        ERROR_AT_PRINT(str)
                m_type = JSON_STRING;
                m_string_value.append(str+1, end - str - 1);
                return end + 1;
        }
        case 't':
        {
                //true
                if (strncmp(str, "true", 4) == 0)
                {
                        m_type = JSON_BOOL;
                        m_value.bool_value = true;
                        return str + 4;
                }
                else
                        ERROR_AT_PRINT(str)
        }
        case 'f':
        {
                //false
                if (strncmp(str, "false", 5) == 0)
                {
                        m_type = JSON_BOOL;
                        m_value.bool_value = false;
                        return str + 5;
                }else
                        ERROR_AT_PRINT(str)
        }
        case 'n':
        {
                //null
                if (strncmp(str, "null", 4) == 0)
                {
                        m_type = JSON_NULL;
                        return str + 4;
                }else
                        ERROR_AT_PRINT(str)
        }
        default:
        {
                //digit
                if (str[0] != '-' && (str[0]<'0' || str[0] >'9'))
                        ERROR_AT_PRINT(str)
                const char *end = str + 1;
                int dot_count = 0;
                while ((*end >= '0' && *end <= '9') || *end == '.')
                {
                        if (*end == '.')
                        {
                                ++dot_count;
                                if (dot_count > 1)
                                        ERROR_AT_PRINT(str)
                        }
                        ++end;
                }
                std::string v;
                v.append(str, end - str);
                istringstream iss(v);

                if (dot_count == 0)
                {
                        if (str[0] == '-')
                        {
                                m_type = JSON_INT;
                                iss >> m_value.int_value;
                        }
                        else
                        {
                                m_type = JSON_UINT;
                                iss >> m_value.uint_value;
                        }
                }
                else
                {
                        m_type = JSON_DOUBLE;
                        iss >> m_value.double_value;
                }
                return end;
        }
        break;
        }

        return NULL;
}
