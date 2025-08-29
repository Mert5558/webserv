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
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Calculator - Bucket Hats</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet">
  <style>
    /* Global styles */
    body {{
      margin: 0;
      font-family: 'Inter', sans-serif;
      background-color: #f9f9f9;
      color: #222;
      display: flex;
      flex-direction: column;
      min-height: 100vh;
    }}

    a {{
      text-decoration: none;
      color: inherit;
    }}

    /* Header */
    header {{
      padding: 1rem 2rem;
      background-color: #ffffff;
      box-shadow: 0 1px 3px rgba(0,0,0,0.1);
      display: flex;
      justify-content: space-between;
      align-items: center;
    }}

    header h1 {{
      font-size: 1.5rem;
      font-weight: 600;
      margin: 0;
    }}

    nav a {{
      margin-left: 1.5rem;
      font-weight: 500;
      transition: color 0.2s;
    }}

    nav a:hover {{
      color: #007BFF;
    }}

    /* Main content */
    main {{
      flex: 1;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      text-align: center;
      padding: 2rem;
    }}

    form {{
      background-color: #fff;
      padding: 8rem;
      border-radius: 14px;
      box-shadow: 0 3px 8px rgba(0,0,0,0.07);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 1.5rem;
      width: 100%;
      max-width: 550px;
    }}

    label {{
      width: 100%;
      font-weight: 500;
      color: #333;
      font-size: 1.1rem;
    }}

    input[type="number"] {{
      width: 370px;
      padding: 12px 14px;
      margin-top: 6px;
      font-size: 1rem;
      border: 1px solid #ccc;
      border-radius: 6px;
    }}

    .operations {{
      display: flex;
      justify-content: center;
      gap: 1.75rem;
      flex-wrap: wrap;
      width: auto;
    }}

    .btn {{
      flex: 1;
      padding: 12px 24px;
      font-size: 1rem;
      font-weight: 500;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      background-color: #007BFF;
      color: #fff;
      transition: background 0.3s ease;
    }}

    .btn:hover {{
      background-color: #0056b3;
    }}


    .btn.add {{
        background-color: #28a745; /* green */
    }}
    
    .btn.subtract {{
        background-color: #dc3545; /* red */
    }}

    .btn.multiply {{
        background-color: #ffc107; /* yellow */
    }}
    
    .btn.divide {{
        background-color: #17a2b8; /* teal */
    }}

    button[value="+"] {{
        background-color: #28a745;
    }}

    button[value="-"] {{
        background-color: #dc3545;
    }}

    button[value="*"] {{
        background-color: #ffc107;
        color: #222;
    }}
    button[value="/"] {{
        background-color: #17a2b8;
    }}

    .result {{
      margin-top: 4rem;
      font-weight: 700;
      color: #000;
      font-size: 2.5rem;
    }}

    /* Footer */
    footer {{
      background-color: #ffffff;
      padding: 1rem 2rem;
      text-align: center;
      font-size: 0.9rem;
      color: #555;
      box-shadow: 0 -1px 3px rgba(0,0,0,0.05);
    }}

    footer a {{
      color: #007BFF;
      margin: 0 0.5rem;
      font-weight: 500;
      text-decoration: none;
    }}
  </style>
</head>
<body>
  <header>
    <h1>Bucket Hats</h1>
    <nav>
      <a href="/">Home</a>
      <a href="/cgi-bin/calc.py">Calculator</a>
      <a href="/uploads">Post</a>
    </nav>
  </header>

  <main>
    <form action="/cgi-bin/calc.py" method="get">
      <label>Num 1:
        <input type="number" name="num1" value="{html.escape(raw_a)}">
      </label>
      <label>Num 2:
        <input type="number" name="num2" value="{html.escape(raw_b)}">
      </label>
      <div class="operations">
        <button type="submit" name="operation" value="+" class="btn">Add</button>
        <button type="submit" name="operation" value="-" class="btn">Subtract</button>
        <button type="submit" name="operation" value="*" class="btn">Multiply</button>
        <button type="submit" name="operation" value="/" class="btn">Divide</button>
      </div>
    </form>

    {f'<div class="result">{result_html}</div>' if result_html else ''}
  </main>

  <footer>
    &copy; 2025 Bucket Hats. 
    <a href="/">Home</a> | 
    <a href="/cgi-bin/calc.py">Calculator</a> | 
    <a href="/uploads/">Post</a>
  </footer>
</body>
</html>
""")
