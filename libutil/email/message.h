#ifndef MESSAGE_H__
#define MESSAGE_H__

// http://tools.ietf.org/html/rfc5234
/*
	ALPHA          =  %x41-5A / %x61-7A   ; A-Z / a-z
	BIT            =  "0" / "1"
	CHAR           =  %x01-7F             ; any 7-bit US-ASCII character, excluding NUL
	CR             =  %x0D                ; carriage return
	CRLF		   =  CR LF			      ; Internet standard newline
	CTL            =  %x00-1F / %x7F      ; controls
	DIGIT          =  %x30-39             ; 0-9
	DQUOTE         =  %x22                ; " (Double Quote)
	HEXDIG         =  DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
	HTAB           =  %x09                ; horizontal tab
	LF             =  %x0A                ; linefeed
	LWSP           =  *(WSP / CRLF WSP)
	                                ; Use of this linear-white-space rule
	                                ;  permits lines containing only white
	                                ;  space that are no longer legal in
	                                ;  mail headers and have caused
	                                ;  interoperability problems in other
	                                ;  contexts.
	                                ; Do not use when defining mail
	                                ;  headers and use with caution in
	                                ;  other contexts.
	OCTET          =  %x00-FF             ; 8 bits of data
	SP             =  %x20                                
	VCHAR          =  %x21-7E             ; visible (printing) characters
	WSP            =  SP / HTAB           ; white space
*/

/*
FWS             =   ([*WSP CRLF] 1*WSP) /  obs-FWS
                                          ; Folding white space
ctext           =   %d33-39 /          ; Printable US-ASCII
                    %d42-91 /          ;  characters not including
                    %d93-126 /         ;  "(", ")", or "\"
                    obs-ctext
ccontent        =   ctext / quoted-pair / comment
comment         =   "(" *([FWS] ccontent) [FWS] ")"
CFWS            =   (1*([FWS] comment) [FWS]) / FWS
*/

#endif

