#!/usr/bin/env python3
import os

print("Relative path to this file:", os.path.relpath(__file__))

with open("data.txt", "r") as file:
	data = file.read()
	print(data)