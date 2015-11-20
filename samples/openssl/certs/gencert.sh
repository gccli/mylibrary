#! /bin/bash

CFG=/etc/ssl/openssl.cnf

# To create the root CA
openssl req -newkey rsa:1024 -sha1 -keyout rootkey.pem -out rootreq.pem
openssl x509 -req -in rootreq.pem -sha1 -extfile $CFG -extensions v3_ca -signkey rootkey.pem -out rootcert.pem
cat rootcert.pem rootkey.pem > root.pem
openssl x509 -subject -issuer -noout -in root.pem


# To create the server CA and sign it with the root CA
openssl req -newkey rsa:1024 -sha1 -keyout serverCAkey.pem -out  serverCAreq.pem
openssl x509 -req -in serverCAreq.pem -sha1 -extfile $CFG -extensions v3_ca -CA root.pem -CAkey root.pem -CAcreateserial -out serverCAcert.pem
cat serverCAcert.pem serverCAkey.pem rootcert.pem > serverCA.pem
openssl x509 -subject -issuer -noout -in serverCA.pem


# To create the server's certificate and sign it with the Server CA
openssl req -newkey rsa:1024 -sha1 -keyout serverkey.pem -out serverreq.pem
openssl x509 -req -in serverreq.pem -sha1 -extfile $CFG -extensions usr_cert -CA serverCA.pem -CAkey serverCA.pem  -CAcreateserial -out servercert.pem
cat servercert.pem serverkey.pem serverCAcert.pem rootcert.pem > server.pem
openssl x509 -subject -issuer -noout -in server.pem


# To create the client certificate and sign it with the Root CA
openssl req -newkey rsa:1024 -sha1 -keyout clientkey.pem -out clientreq.pem
openssl x509 -req -in clientreq.pem -sha1 -extfile $CFG -extensions usr_cert -CA root.pem -CAkey root.pem -CAcreateserial -out clientcert.pem
cat clientcert.pem clientkey.pem rootcert.pem > client.pem
openssl x509 -subject -issuer -noout -in client.pem


# To create dh512.pem and dh1024.pem
openssl dhparam -check -text -5 512 -out dh512.pem
openssl dhparam -check -text -5 1024 -out dh1024.pem
