#include <json/json.h>
#include <stdio.h>
#include <string.h>

json_object *json_array_test()
{
    json_object *jobj = json_object_new_object();

    /*Creating a json array*/
    json_object *jarray = json_object_new_array();

    json_object *jhost1 = json_object_new_string("172.16.0.135:27017");
    json_object *jhost2 = json_object_new_string("172.16.0.136:27017");
    json_object *jhost3 = json_object_new_string("172.16.0.137:27017");

    json_object_array_add(jarray,jhost1);
    json_object_array_add(jarray,jhost2);
    json_object_array_add(jarray,jhost3);

    /*Form the json object*/
    json_object_object_add(jobj,"server", jarray);

    printf ("Array: %s\n",json_object_to_json_string(jobj));
    return jobj;
}

json_object *json_map_test()
{
    /* { "jmaster": { "host": "10.0.11.224", "port": "3306", "username": "yongche" } } */

    json_object *jmaster = json_object_new_object();
    json_object *jassoc = json_object_new_object();

    json_object *jhost = json_object_new_string("10.0.11.224");
    json_object *jport = json_object_new_string("3306");
    json_object *juser = json_object_new_string("yongche");

    json_object_object_add(jassoc,"host", jhost);
    json_object_object_add(jassoc,"port", jport);
    json_object_object_add(jassoc,"username", juser);
    json_object_object_add(jmaster,"master", jassoc);

    printf ("Map: %s\n",json_object_to_json_string(jmaster));
    return jmaster;
}

void json_object_test()
{
    json_object * jobj = json_object_new_object();
    /*Creating a json string*/
    json_object *jstring = json_object_new_string("http://www.yongche-inc.com:8080/");
    /*Creating a json integer*/
    json_object *jint = json_object_new_int(3);
    /*Creating a json boolean*/
    json_object *jboolean = json_object_new_boolean(1);
    /*Creating a json double*/
    json_object *jdouble = json_object_new_double(3.14);

    json_object *jmap= json_array_test();
    json_object *jarray = json_map_test();

    /*Form the json object*/
    /*Each of these is like a key value pair*/
    json_object_object_add(jobj,"url", jstring);
    json_object_object_add(jobj,"is-master", jboolean);
    json_object_object_add(jobj,"pi", jdouble);
    json_object_object_add(jobj,"connection-timeout", jint);
    json_object_object_add(jobj,"mysql", jmap);
    json_object_object_add(jobj,"mongo.message", jarray);

    const char *str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    size_t len = strlen(str);
    FILE* fp = fopen("/tmp/config.json", "w");
    if (fp) {
	int sz = fwrite(str, len, 1, fp);
	fclose(fp);
    }    
    printf ("Config: (len:%d)\n%s\n", len, 
	    json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY));
}

int main(int argc, char *argv[])
{
    json_object_test();

    return 0;
}
