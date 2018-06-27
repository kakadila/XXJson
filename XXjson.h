#ifndef XJSON_H
#define XJSON_H

#include <string>

template <typename T>
class TypeIdx
{
public:
        enum{ IDX = 0};
};

#define BASE_TYPE(T,idx)                 \
template<> class TypeIdx<T> {            \
    public:                            \
       enum{ IDX = idx};                \
}; 

BASE_TYPE(bool,1)

BASE_TYPE(char,2)
BASE_TYPE(short, 2)
BASE_TYPE(int,2)
BASE_TYPE(long long,2)

BASE_TYPE(unsigned char,3)
BASE_TYPE(unsigned short,3)
BASE_TYPE(unsigned int,3)
BASE_TYPE(unsigned long,3)
BASE_TYPE(unsigned long long,3)

BASE_TYPE(float,4)
BASE_TYPE(double,4)

class CheckType
{
public:
        template<typename V>
        static bool IsBool(V)
        {
                return (int)TypeIdx<bool>::IDX == (int)TypeIdx<V>::IDX;
        }

        template<typename V>
        static bool IsInt(V)
        {
                return (int)TypeIdx<long long>::IDX == (int)TypeIdx<V>::IDX;
        }

        template<typename V>
        static bool IsUInt(V)
        {
                return (int)TypeIdx<unsigned long long>::IDX == (int)TypeIdx<V>::IDX;
        }

        template<typename V>
        static bool IsDouble(V)
        {
                return (int)TypeIdx<double>::IDX == (int)TypeIdx<V>::IDX;
        }
};


#define JSON_ERR_TYPE 3001
#define JSON_ERR_ARRAY_SIZE 3002

#define JSON_NULL 0
#define JSON_INVALID 1
#define JSON_ARRAY 1<<1
#define JSON_OBJECT 1<<2
#define JSON_INT 1 << 3
#define JSON_UINT 1<<4
#define JSON_DOUBLE 1<<5
#define JSON_BOOL 1<<6
#define JSON_STRING 1<<7


#include <stdio.h>
#include <vector>
#include <map>

class JsonItem;

typedef std::vector<JsonItem*> JsonArray;
typedef std::map<std::string,JsonItem *> JsonObject;

union JsonValue
{
        long long int_value;
        unsigned long long uint_value;
        double double_value;
        bool bool_value;
};


class JsonItem
{
public:
        JsonItem();
        ~JsonItem();
        JsonItem &operator[](const char *key);
        JsonItem &operator[](int idx);

        const char *ToString();
        void SetNull();
        void PushNull();

        void SetObject(); //set item to object,will clear data
        void SetArray(); //set item to array, will clear data

        long long GetInt();
        unsigned long long GetUInt();
        double GetDouble();
        bool GetBool();
	const char *GetString();
        int Size();  //array size
        int GetType();
	
	bool Parse(std::string str); 
        bool Parse(const char *str); //parse data

        template<typename T>
        void Push(T value)
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
                *item = value;
                m_array_value.push_back(item);
        }

        void Push(JsonItem &value)
        {
                Push<JsonItem&>(value);
        }

        template<typename T>
        JsonItem& operator=(T value)
        {
                ClearItem(this);
                if (CheckType::IsInt(value))
                {
                        m_type = JSON_INT;
                        m_value.int_value = value;
                }
                else if (CheckType::IsUInt(value))
                {
                        m_type = JSON_UINT;
                        m_value.uint_value = value;
                }
                else if (CheckType::IsDouble(value))
                {
                        m_type = JSON_DOUBLE;
                        m_value.double_value = value;
                }
                else if (CheckType::IsBool(value))
                {
                        m_type = JSON_BOOL;
                        m_value.bool_value = value;
                }

                return *this;
        }

        JsonItem& operator=(JsonItem& value);
        JsonItem& operator=(std::string value);
        JsonItem& operator=(const char *value);

private:
        void ClearItem(JsonItem *item);
        const char *ParseObect(const char *str);
        const char *ParseArray(const char *str);
        const char *ParseValue(const char *str);
protected:
        bool m_need_release; //flag is need to release,when copy,set it false
        unsigned short m_type;
        JsonValue m_value;
        JsonArray m_array_value;
        JsonObject m_object_value;
        std::string m_string_value;

        std::string m_tostring;
};

typedef JsonItem XXJson;

#endif               