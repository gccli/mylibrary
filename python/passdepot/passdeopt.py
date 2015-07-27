#!/usr/bin/env python
#

import cgi
import datetime
import webapp2
import urllib
import logging
import string

from google.appengine.ext import ndb
from google.appengine.api import users

TEMPLATE_MAINPAGE = '''
    <form action="/logging" method="post">
    <dl class="form"><dd><input type="text" name="name" class="textfield" placeholder="Account Name" data-autocheck-url="/check/name" autofocus></dd></dl>
    <dl class="form"><dd><input type="text" name="login" class="textfield" placeholder="Account Identity, such as email, username, id, etc."></dd></dl>
    <dl class="form"><dd><input type="password" name="password" class="textfield" placeholder="Create a password"></dd></dl> 
    <button class="button primary button-block" type="submit">Submit</button>
    </form>

    <form action="/query" method="post">
    <dl class="form"><dd><input type="text" name="name" class="textfield" placeholder="Account Name" data-autocheck-url="/check/name" autofocus></dd></dl>
    <button class="button primary button-block" type="submit">Query</button>
    </form>

    '''

TEMPLATE_BEGIN = '''
    <html><head>
    <meta http-equiv="content-type" content="text/html;charset=utf-8">
    <title>$title</title>
    <style><!--
    body {font-family: arial,sans-serif}
    div.nav {margin-top: 1ex}
    div.nav A {font-size: 10pt; font-family: arial,sans-serif}
    span.nav {font-size: 10pt; font-family: arial,sans-serif; font-weight: bold}
    div.nav A,span.big {font-size: 12pt; color: #0000cc}
    div.nav A {font-size: 10pt; color: black}
    A.l:link {color: #6f6f6f}
    A.u:link {color: green}
    //--></style>
    </head>
    <body text=#000000 bgcolor=#ffffff>
    <table border=0 cellpadding=2 cellspacing=0 width=100%>
    <tr><td bgcolor=#3366cc><font face=arial,sans-serif color=#ffffff><b>$message</b></td></tr>
    <tr><td> </td></tr></table>
'''

TEMPLATE_END = '''
    <table width=100% cellpadding=0 cellspacing=0><tr><td bgcolor=#3366cc><img alt="" width=1 height=4></td></tr></table>
    </body></html>
    '''

default_account_key = ndb.Key('Account', 'default_key')

class Account(ndb.Model):
  name = ndb.StringProperty()
  identity = ndb.StringProperty()
  login = ndb.JsonProperty()
  passwd = ndb.JsonProperty()
  tags = ndb.StringProperty(repeated=True)
  description = ndb.TextProperty()
  date = ndb.DateTimeProperty(auto_now_add=True)

class MainPage(webapp2.RequestHandler):
  def get(self):
    self.response.out.write(string.Template(TEMPLATE_BEGIN).substitute(title='Home',message='MainPage'))
    self.response.out.write(string.Template(TEMPLATE_MAINPAGE).substitute())
    self.response.out.write(string.Template(TEMPLATE_END).substitute())

class PassQuery(webapp2.RequestHandler):
  def write_account(self, acct):
    if acct:
      if acct.name:
        self.response.out.write('<br>Name      : %s</br>' % cgi.escape(acct.name))
      if acct.login:
        self.response.out.write('<br>Login     : %s</br>' % cgi.escape(acct.login))
      if acct.passwd:
        self.response.out.write('<br>Password  : %s</br>' % cgi.escape(acct.passwd))

  def list_account(self, name):
    entities = ndb.gql('SELECT * FROM Account WHERE ANCESTOR IS :1 '
                       'ORDER BY date DESC LIMIT 10',
                       default_account_key)

    self.response.out.write(string.Template(TEMPLATE_BEGIN).substitute(title='Query',message='Accounts'))
    found = False
    for acct in entities:
      if name and name == acct.name:
        self.write_account(acct)
        found = True
        break;
      elif not name:
        self.write_account(acct)
      if name and not found:
        self.response.out.write('Not Found')
    self.response.out.write(string.Template(TEMPLATE_END).substitute())

  def get(self):
    self.list_account(self.request.get('name'))

  def post(self):
    acctname = urllib.unquote(self.request.get('name'))
    if not acctname:
      logging.debug('Please input a username')
      self.get()
      return None
    else:
      self.redirect('/query?name={0}'.format(acctname))

class PassLogging(webapp2.RequestHandler):
  @ndb.transactional
  def post(self):
    acctname = urllib.unquote(self.request.get('name'))
    acctlogin = urllib.unquote(self.request.get('login'))
    acctpassword = urllib.unquote(self.request.get('password'))

    logging.debug("Account Name:%s, login:%s, password:%s", acctname, acctlogin, acctpassword)

    account_key = ndb.Key('Account', acctname)
    account = Account(parent=default_account_key,name=acctname,login=acctlogin,passwd=acctpassword)
    account_key = account.put()
    self.redirect('/')

app = webapp2.WSGIApplication([
  ('/', MainPage),
  ('/logging', PassLogging),
  ('/query', PassQuery),
  ('/query?.*', PassQuery)
], debug=True)

