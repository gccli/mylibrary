#! /bin/bash

host=localhost
port=2222

function sendto {
nc -i 5 -C $host $port <<EOF
      HELO host.example.com
      MAIL FROM:<user@host.example.com>
      RCPT TO:<user2@host.example.com>
      DATA
      Body of email.
      .
      QUIT
EOF
}
i=0
for i in `seq 1 10000`
do
  sendto &
  pkill -USR1 epoll-server
  pkill -USR2 epoll-server
  i=$(($i+1))
done

echo "done"
pause
