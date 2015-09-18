#include "mime_address.h"

/*
static void test_message_address(void)
{
	static const char *input[] = {
		"user@domain", NULL,
		"<user@domain>", "user@domain",
		"foo bar <user@domain>", NULL,
		"\"foo bar\" <user@domain>", "foo bar <user@domain>",
		"<@route:user@domain>", NULL,
		"<@route@route2:user@domain>", "<@route,@route2:user@domain>",
		"hello <@route ,@route2:user@domain>", "hello <@route,@route2:user@domain>",
		"user (hello)", NULL,
		"hello <user>", NULL,
		"@domain", NULL
	};
*/


struct address_parser 
{
	const char *data, *end;
};

struct message_address *mime_parse_address(const char *buffer)
{
}


struct message_address *mime_parse_address(const char *buffer)
{
	if (buffer == NULL || buffer[0] == 0)
		return NULL;
	if (strchr(buffer, '@') == NULL)
		return NULL;

	int addrlen = strlen(buffer);

	struct message_address *address = calloc(1, sizeof(struct message_address));

	char temp[256] = "\0";
	strncpy(temp, buffer, sizeof(temp));

	char *angle_l = strchr(temp, '<');
	char *angle_r = strchr(temp, '>');
	char *display = temp;

	if (angle_l != NULL && angle_r != NULL) 
	{
		int len = angle_r - angle_l;
		address->mailbox = calloc(1, len+1);
		memcpy(address->mailbox, angle_l+1, len);
	}

	
	
}

