#! /usr/bin/env python
# -*- coding: utf-8 -*-
# 邮件测试


import os
import sys
import datetime
import smtplib
import mimetypes

import email
from email.message import Message
from email.mime.audio import MIMEAudio
from email.mime.base import MIMEBase
from email.mime.image import MIMEImage
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart


textfile  = 'sendmail.py'
sender    = 'lijing@safedefense.net'
recipients= ['lijing1-s@360.cn', 'inetlinux@126.com', 'inetlinux@163.com']

body      = open(sys.argv[0], 'r').read(64)
directory = 's'

cs = email.charset.Charset('UTF-8')
cs.header_encoding = email.charset.BASE64

outer = MIMEMultipart('mixed')
outer.set_charset(cs)

t = datetime.datetime.now().time()
outer['Subject'] = cs.header_encode('时间:%s 目录 %s' % (t, directory))
outer['From'] = sender
outer['To'] = ', '.join(recipients)
outer['Date'] = email.utils.formatdate()

msg = MIMEText(body, _subtype='plain', _charset='UTF-8')
outer.attach(msg)
print '------------ Body ------------'
print msg.as_string()


for filename in os.listdir(directory):
    path = os.path.join(directory, filename)
    if not os.path.isfile(path):
        continue

    ctype, encoding = mimetypes.guess_type(path)
    if ctype is None or encoding is not None:
        ctype = 'application/octet-stream'

    maintype, subtype = ctype.split('/', 1)
    if maintype == 'text':
        fp = open(path)
        msg = MIMEText(fp.read(), _subtype=subtype)
        fp.close()
    elif maintype == 'image':
        fp = open(path, 'rb')
        msg = MIMEImage(fp.read(), _subtype=subtype)
        fp.close()
    elif maintype == 'audio':
        fp = open(path, 'rb')
        msg = MIMEAudio(fp.read(), _subtype=subtype)
        fp.close()
    else:
        fp = open(path, 'rb')
        msg = MIMEBase(maintype, subtype)
        msg.set_payload(fp.read())
        fp.close()
        # Encode the payload using Base64
        email.encoders.encode_base64(msg)
    msg.add_header('Content-Disposition', 'attachment', filename=cs.header_encode(filename))
    #msg.add_header('Content-Disposition', 'attachment', filename=('UTF-8','en_US',filename))
    outer.attach(msg)

# Send the message via our own SMTP server, but don't include the
# envelope header.
composed = outer.as_string()
open('mail.txt', 'w').write(composed)

if len(sys.argv) > 1 and sys.argv[1] == '-s':
    s = smtplib.SMTP('localhost')
#    s.set_debuglevel(True)
    s.sendmail(sender, recipients, composed);
    s.quit()
