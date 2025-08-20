#!/usr/bin/env python3

import os

print("Hello from CGI script!")
print(f"REQUEST_METHOD = {os.environ.get('REQUEST_METHOD')}")
print(f"SCRIPT_FILENAME = {os.environ.get('SCRIPT_FILENAME')}")
print(f"QUERY_STRING = {os.environ.get('QUERY_STRING')}")
print(f"CONTENT_LENGTH = {os.environ.get('CONTENT_LENGTH')}")
print(f"SERVER_PORT = {os.environ.get('SERVER_PORT')}")
print(f"SERVER_NAME = {os.environ.get('SERVER_NAME')}")
print(f"REDIRECT_STATUS = {os.environ.get('REDIRECT_STATUS')}")
print(f"REQUEST_URI = {os.environ.get('REQUEST_URI')}")