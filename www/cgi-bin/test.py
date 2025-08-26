#!/usr/bin/env python3

import os
import cgi

print("Content-Type: text/html\n")

print("<html><body>")
print("<h1>CGI Query String Example</h1>")

# Get QUERY_STRING raw value
query_string = os.environ.get("QUERY_STRING", "")
print(f"<p>Raw QUERY_STRING: {query_string}</p>")

# Parse it into key/value pairs
form = cgi.FieldStorage()
name = form.getfirst("name", "(not provided)")
age = form.getfirst("age", "(not provided)")

print(f"<p>Hello, {name}! You are {age} years old.</p>")
print("</body></html>")