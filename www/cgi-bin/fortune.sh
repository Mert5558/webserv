#!/bin/bash

echo "Content-type: text/html"
echo ""

# List of fortunes/quotes
fortunes=(
    "Once you go WEBSERVER you never go IRC. - Bucket Hats"
    "Once you write WEBSERVER, life gets real sweet, IRC is still blinking like it missed the beat! - Bucket Hats"
    "Once you write a CGI script,You never go back to chat, IRC has no POST method, You just laugh at that. - Bucket Hats"
    "In a world full of WEBSERVER, be a CGI. - Bucket Hats"
    "To POST or not to POST, that is the question. - Bucket Hats"
    "CGI is the spice of WEBSERVER life. - Bucket Hats"
    "Roses are red, Violets are blue, Once you do WEBSERVER, IRC won’t do you. - Bucket Hats"
    "CGI is the key to unlock the WEBSERVER magic. - Bucket Hats"
    "The quieter you become, the more you can hear. - Ram Dass"
    "In the middle of difficulty lies opportunity. - Albert Einstein"
    "What we think, we become. - Buddha"
    "Simplicity is the ultimate sophistication. - Leonardo da Vinci"
    "A journey of a thousand miles begins with a single step. - Lao Tzu"
    "The only way to do great work is to love what you do. - Steve Jobs"
    "Believe you can and you're halfway there. - Theodore Roosevelt"
    "It does not matter how slowly you go as long as you do not stop. - Confucius"
    "Happiness is not something ready made. It comes from your own actions. - Dalai Lama"
    "Do one thing every day that scares you. - Eleanor Roosevelt"
    "Stay hungry, stay foolish. - Steve Jobs"
)

# Pick a random one
RAW_FORTUNE=${fortunes[$RANDOM % ${#fortunes[@]}]}

# Color specific words (example: color "WEBSERVER" and "IRC")
RANDOM_FORTUNE=$(echo "$RAW_FORTUNE" | sed 's/WEBSERVER/<span style="color:#372BE0;">WEBSERVER<\/span>/g;
                                            s/POST/<span style="color:#2AAA8A;">POST<\/span>/g;
                                            s/IRC/<span style="color:#6B1010;">IRC<\/span>/g;
                                            s/CGI/<span style="color:#007BFF;">CGI<\/span>/g;
                                            s/blue/<span style="color:#2B64E0;">blue<\/span>/g;
                                            s/red/<span style="color:#E02B3A;">red<\/span>/g;
                                            s/Violets/<span style="color:#8C2BE0;">Violets<\/span>/g;
                                            s/Roses/<span style="color:#E02BDA;">Roses<\/span>/g')

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
       color: #4e0b41;
      display: flex;
      flex-direction: column;
      min-height: 100vh;
      background-image: url('/pixel-weave.png');
      filter: contrast(0.9) brightness(1.2);
       position: relative;
      z-index: 1;
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
      color: #000000;
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
    <div class="logo">
      <a href="/"><h1>Bucket Hats</h1></a>
    </div>
    <nav>
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
      <a href="/uploads/">Post</a> |
      <a href="/cgi-bin/fortune.sh">Fortune</a>
    </nav>
  </footer>
</body>
</html>
EOF
