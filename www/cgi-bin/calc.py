#!/usr/bin/env python3
import cgi, html, sys

# Correct CGI header
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write("\r\n")  # end of headers

form = cgi.FieldStorage()
raw_a = form.getfirst("num1", "")
raw_b = form.getfirst("num2", "")
op    = form.getfirst("operation", "+")

def compute(a, b, op):
    if op == '+': return a + b
    if op == '-': return a - b
    if op == '*': return a * b
    if op == '/':
        if b == 0: return "Division by zero"
        return a / b
    return "Bad op"

result_html = ""
if raw_a != "" and raw_b != "":
    try:
        a_num = int(raw_a)
        b_num = int(raw_b)
        result = compute(a_num, b_num, op)
        if isinstance(result, str):
            result_html = f"<p><strong>Error:</strong> {html.escape(result)}</p>"
        else:
            result_html = f"<p>Result: <strong>{a_num} {html.escape(op)} {b_num} = {result}</strong></p>"
    except ValueError:
        result_html = "<p><strong>Error:</strong> Please enter valid integers.</p>"
else:
    result_html = "<p>Enter numbers and submit.</p>"

print(f"""<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Calc Result</title>
  <style>
    body {{
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        height: 100vh;
        margin: 0;
        font-family: Arial, sans-serif;
        background-color: #f0f8ff;
    }}
    h1 {{
        color: #333;
    }}
    p {{
        color: #555;
    }}
    form {{
        margin-top: 20px;
    }}
    label {{
        display: block;
        margin: 10px 0;
    }}
    input[type="number"], select {{
        padding: 5px;
        font-size: 16px;
    }}
    input[type="submit"], .back-button, .operation-button {{
        margin-top: 10px;
        padding: 10px 20px;
        font-size: 16px;
        color: white;
        background-color: #007BFF;
        border: none;
        border-radius: 5px;
        cursor: pointer;
    }}
    input[type="submit"]:hover, .back-button:hover, .operation-button:hover {{
        background-color: #0056b3;
    }}
    .back-button {{
        text-decoration: none;
        display: inline-block;
        text-align: center;
    }}
  </style>
</head>
<body>
  <h1>Interactive Calculator (CGI)</h1>
  <form action="/cgi-bin/calc.py" method="get">
    <label>Number 1: <input type="number" name="num1" value="{html.escape(raw_a)}"></label>
    <label>Number 2: <input type="number" name="num2" value="{html.escape(raw_b)}"></label>
    <!-- Removed hidden input field for operation -->
    <div>
      <button type="submit" name="operation" value="+" class="operation-button">Add</button>
      <button type="submit" name="operation" value="-" class="operation-button">Subtract</button>
      <button type="submit" name="operation" value="*" class="operation-button">Multiply</button>
      <button type="submit" name="operation" value="/" class="operation-button">Divide</button>
    </div>
  </form>
  {result_html}
  <a href="/index.html" class="back-button">Back to Home</a>
</body>
</html>""")
