#ifndef MIME_ADDRESS_H__
#define MIME_ADDRESS_H__

struct message_address 
{
	char                   *display_name;
	char                   *mailbox;
	struct message_address *next;
};

#endif

