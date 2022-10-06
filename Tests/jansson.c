#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>


/* 
int json_dump_file(const json_t *json, const char *path, size_t flags)
json_t *json_loadf(FILE *input, size_t flags, json_error_t *error)
*/

int main(int argc, char* argv[])
{

    json_t *root;
    json_error_t error;

	root = json_load_file("test.json", 0, &error);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 1;
	}
	
	json_t* name = json_object_get(root, "name");
	json_t* age = json_object_get(root, "age");
	json_t* members = json_object_get(root, "members");

	char msg[512] = {0};

	sprintf(msg, "Name: %s\r\nAge: %i\r\nFriends: [", json_string_value(name), (int)json_number_value(age));
	
	for(size_t i = 0; i < json_array_size(members); i++)
	{
		json_t* data;
		char message_text[256];

		data = json_array_get(members, i);
		sprintf(message_text, "%s%s", json_string_value(data), i + 1 < json_array_size(members) ? ", ": "");
		strcat(msg, message_text);
	}

	strcat(msg, "]\n\r");
	

	printf("Json\n\r");
	printf(msg);
	
	json_decref(root);
	return 0;
}
