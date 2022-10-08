#ifndef Json_h__
#define Json_h__

#include <jansson.h>

static inline int json_getUInt(json_t* _JSON, const char* _Filed, UInt8* _Value)
{
	if(_Value == NULL)
		return -1;
	
	json_t* value = json_object_get(_JSON, _Filed);
	if(value == NULL)
		return -2;
		
	*(_Value) = (UInt8)json_number_value(value);
	
	return 0;
}

static inline int json_getUInt16(json_t* _JSON, const char* _Filed, UInt16* _Value)
{
	if(_Value == NULL)
		return -1;
	
	json_t* value = json_object_get(_JSON, _Filed);
	if(value == NULL)
		return -2;
		
	*(_Value) = (UInt16)json_number_value(value);
	
	return 0;
}

static inline int json_getString(json_t* _JSON, const char* _Filed, const char** _Value)
{
	if(_Value == NULL)
		return -1;

	json_t* value = json_object_get(_JSON, _Filed);
	if(value == NULL)
		return -2;

	*(_Value) = (const char*)json_string_value(value);
	
	return 0;
}

#endif // Json_h__