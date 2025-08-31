#!/bin/bash

echo "Content-type: text/html"
echo ""

# List of fortunes/quotes
fortunes=(
    "The quieter you become, the more you can hear."
    "Do one thing every day that scares you."
    "Simplicity is the ultimate sophistication."
    "A journey of a thousand miles begins with a single step."
    "Stay hungry, stay foolish."
    "Your server is smiling at you 😎"
)

# Pick a random one
RANDOM_FORTUNE=${fortunes[$RANDOM % ${#fortunes[@]}]}

# HTML output
cat <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Random Fortune</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet">
  <style>
    /* Global styles */
    body {
      margin: 0;
      font-family: 'Inter', sans-serif;
      background-color: #f9f9f9;
      color: #222;
      display: flex;
      flex-direction: column;
      min-height: 100vh;
    }

    a {
      text-decoration: none;
      color: inherit;
    }

    /* Header */
    header {
      padding: 1rem 2rem;
      background-color: #ffffff;
      box-shadow: 0 1px 3px rgba(0,0,0,0.1);
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    header h1 {
      font-size: 1.5rem;
      font-weight: 600;
      margin: 0;
    }

    nav a {
      margin-left: 1.5rem;
      font-weight: 500;
      transition: color 0.2s;
    }

    nav a:hover {
      color: #007BFF;
    }

    /* Main content */
    main {
      flex: 1;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      text-align: center;
      padding: 2rem;
    }

    .fortune {
      background-color: #fff;
      padding: 4rem;
      border-radius: 14px;
      box-shadow: 0 3px 8px rgba(0,0,0,0.07);
      font-size: 1.5rem;
      font-weight: 500;
      color: #333;
      max-width: 550px;
      width: 100%;
    }

    .btn {
      margin-top: 2rem;
      padding: 12px 24px;
      font-size: 1rem;
      font-weight: 500;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      background-color: #007BFF;
      color: #fff;
      transition: background 0.3s ease;
      text-decoration: none;
    }

    .btn:hover {
      background-color: #0056b3;
    }

    /* Footer */
    footer {
      background-color: #ffffff;
      padding: 1rem 2rem;
      text-align: center;
      font-size: 0.9rem;
      color: #555;
      box-shadow: 0 -1px 3px rgba(0,0,0,0.05);
    }

    footer a {
      color: #007BFF;
      margin: 0 0.5rem;
      font-weight: 500;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <header>
    <h1>Bucket Hats</h1>
    <nav>
      <a href="/">Home</a>
      <a href="/cgi-bin/calc.py">Calculator</a>
      <a href="/uploads/">Post</a>
      <a href="/cgi-bin/fortune.sh">Fortune</a>

    </nav>
  </header>

  <main>
    <div class="fortune">
      $RANDOM_FORTUNE
    </div>
    <a href="/cgi-bin/fortune.sh" class="btn">Get another one</a>
  </main>

  <footer>
    <p>&copy; 2025 Bucket Hats. All rights reserved.</p>
    <nav>
      <a href="/">Home</a> | 
      <a href="/cgi-bin/calculator.py">Calculator</a> | 
      <a href="/uploads/">Post</a>
    </nav>
  </footer>
</body>
</html>
EOF
