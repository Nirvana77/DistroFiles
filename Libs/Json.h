#ifndef Json_h__
#define Json_h__

#include <jansson.h>

static inline int json_getUInt(json_t* _JSON, const char* _Filed, unsigned int* _Value)
{
    if(_Value == NULL)
        return -1;

    
	json_t* value = json_object_get(_JSON, _Filed);
    *(_Value) = (unsigned int)json_number_value(value);
    
    return 0;
}

#endif // Json_h__