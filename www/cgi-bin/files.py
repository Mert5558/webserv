# #!/usr/bin/env python3
# import os, sys, json, urllib.parse

# upload_dir = "./www/uploads"

# def list_files():
#     if not os.path.exists(upload_dir):
#         files = []
#     else:
#         try:
#             files = os.listdir(upload_dir)
#         except Exception:
#             files = []
#     print("Content-Type: application/json")
#     print()
#     print(json.dumps(files))

# def delete_file(filename):
#     print("Content-Type: text/plain")
#     print()
#     # Security checks
#     if not filename or "/" in filename or "\\" in filename or ".." in filename:
#         print("Invalid filename")
#         return

#     # Protect important files
#     protected = {"index.html", "index.htm"}
#     if filename in protected:
#         print("Protected file, cannot delete")
#         return

#     filepath = os.path.join(upload_dir, filename)
#     try:
#         os.remove(filepath)
#         print("OK")
#     except Exception as e:
#         print(f"Error: {e}")

# # --- CGI entry point ---
# query = os.environ.get("QUERY_STRING", "")
# params = urllib.parse.parse_qs(query)

# if "delete" in params and "name" in params:
#     delete_file(params["name"][0])
# else:
#     list_files()


#!/usr/bin/env python3
import os, sys, json, urllib.parse, cgi, html

# Always resolve uploads directory relative to this script
script_dir = os.path.dirname(os.path.abspath(__file__))
upload_dir = os.path.abspath(os.path.join(script_dir, "..", "uploads"))

def list_files():
    if not os.path.exists(upload_dir):
        files = []
    else:
        try:
            # Filter out directories, list only files
            files = [f for f in os.listdir(upload_dir) if os.path.isfile(os.path.join(upload_dir, f))]
        except Exception:
            files = []
    print("Content-Type: application/json")
    print()
    print(json.dumps(files))

def delete_file(filename):
    print("Content-Type: text/plain")
    print()
    # Security checks
    if not filename or "/" in filename or "\\" in filename or ".." in filename:
        print("Invalid filename")
        return

    # Protect important files
    protected = {"index.html", "index.htm"}
    if filename in protected:
        print("Protected file, cannot delete")
        return

    filepath = os.path.join(upload_dir, filename)
    try:
        os.remove(filepath)
        print("OK")
    except FileNotFoundError:
        print(f"Error: File not found.")
    except Exception as e:
        print(f"Error: {e}")

def upload_file():
    form = cgi.FieldStorage()
    
    if "file" not in form:
        print("Content-Type: text/plain")
        print("Status: 400 Bad Request")
        print()
        print("No file in request")
        return

    fileitem = form["file"]

    if not fileitem.filename:
        print("Content-Type: text/plain")
        print("Status: 400 Bad Request")
        print()
        print("No file selected")
        return

    # Sanitize filename
    filename = os.path.basename(fileitem.filename)
    
    if not os.path.exists(upload_dir):
        os.makedirs(upload_dir)

    filepath = os.path.join(upload_dir, filename)
    
    with open(filepath, 'wb') as f:
        f.write(fileitem.file.read())

    print("Content-Type: text/html")
    print()
    print("<!DOCTYPE html><html><head><title>Upload Status</title>")
    print('<meta http-equiv="refresh" content="3;url=/uploads/index.html" />')
    print("</head><body>")
    print(f"<h2>File '{html.escape(filename)}' uploaded successfully.</h2>")
    print("<p>Redirecting back to uploads page in 3 seconds...</p>")
    print("</body></html>")


# --- CGI entry point ---
method = os.environ.get("REQUEST_METHOD", "GET")

if method == "POST":
    upload_file()
else: # GET
    query = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(query)

    if "delete" in params and "name" in params:
        delete_file(params["name"][0])
    else:
        list_files()