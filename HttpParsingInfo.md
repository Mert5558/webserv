# Informations regard with the raw HTTP Request Parsing
 
### What "Processing Requests" Means
Processing requests involves interpreting the raw HTTP request data received from the client and extracting meaningful information from it. This is necessary to understand what the client wants the server to do (e.g., retrieve a resource, submit data, etc.).

### Key Steps in Processing Requests

1. **Parsing the Raw Request**:
   - The raw request is a string containing the HTTP method, URL, headers, and possibly a body (for methods like POST).
   - You need to break this string into its components:
     - **HTTP Method**: Determines the action (e.g., GET, POST, DELETE).
     - **URL/Path**: Specifies the resource being requested.
     - **HTTP Version**: Indicates the protocol version (e.g., HTTP/1.1).
     - **Headers**: Provide additional information (e.g., `Content-Type`, `User-Agent`).
     - **Body**: Contains data sent by the client (if applicable).

2. **Validating the Request**:
   - Ensure the HTTP method is valid (e.g., GET, POST, DELETE, etc.).
   - Check that the HTTP version is supported (e.g., HTTP/1.1 or HTTP/2).
   - Verify that required headers are present and correctly formatted.

3. **Determining the Action**:
   - Based on the parsed method and URL, decide what the server should do:
     - **GET**: Retrieve and return the requested resource.
     - **POST**: Process the data sent in the request body.
     - **DELETE**: Remove the specified resource.
   - If the method or URL is invalid, return an appropriate error response (e.g., 404 Not Found, 400 Bad Request).

4. **Handling the Request**:
   - Use the parsed data to locate the requested resource or perform the desired action.
   - For example:
     - If the URL corresponds to a file path, read the file and prepare it for the response.
     - If the URL corresponds to a dynamic resource, execute the necessary logic (e.g., querying a database).

5. **Generating a Response**:
   - Create an HTTP response based on the result of the request handling:
     - **Status Line**: Include the HTTP version and status code (e.g., `HTTP/1.1 200 OK`).
     - **Headers**: Specify metadata about the response (e.g., `Content-Type`, `Content-Length`).
     - **Body**: Include the requested data or an error message.

### Why Parsing is Important
Without parsing, the server cannot understand the client's intent. Parsing transforms the raw request into structured data that the server can use to make decisions and generate appropriate responses.

The functions (`parseRequest`, `isValidMethod`, `isValidVersion`), will help us:
- Extract and validate the components of the request.
- Ensure the request adheres to the HTTP protocol.
- Prepare the server to handle the request appropriately.


#
# Workflow:
1. **Receiving the Request**:
   - The `recv()` function in `main.cpp` retrieves the raw HTTP request from the client and stores it in a buffer (e.g., `char buffer[1024]`).
   - This raw request is then passed to the `parseRequest()` function as a `std::string`.

2. **Parsing the Request**:
   - The `parseRequest()` function processes the raw string to extract:
     - The HTTP method (e.g., GET, POST).
     - The requested path (e.g., `/index.html`).
     - The HTTP version (e.g., HTTP/1.1).
     - Headers (e.g., `Content-Type: text/html`).
     - The body (if present, for methods like POST).

3. **Validating the Request**:
   - The `parseRequest()` function should also validate the extracted components using the helper functions `isValidMethod()` and `isValidVersion()`.

### Why Not Use `recv()` in `parseRequest()`?
- **Separation of Concerns**: `recv()` is responsible for receiving data from the network, while `parseRequest()` is responsible for interpreting that data. Mixing these responsibilities would make your code harder to maintain and test.
- **Reuse**: By keeping `parseRequest()` independent of `recv()`, you can reuse it for testing or other scenarios where the raw request is provided directly (e.g., from a file or a mock client).

### Summary:
- Use `recv()` in `main.cpp` to receive the raw request.
- Pass the raw request string to `parseRequest()` for parsing and validation.
- Keep `parseRequest()` focused on processing the string, not on network communication.

#
# Raw bytes
"Raw bytes" refer to unprocessed, low-level binary data that is transmitted over a network or stored in memory. These bytes are the fundamental building blocks of data and are not yet interpreted or structured into meaningful information like strings, numbers, or objects.

### In the Context of Networking:
When data is sent over a network (e.g., HTTP requests), it is transmitted as a sequence of raw bytes. These bytes represent the actual content of the request or response, including headers, body, and other metadata, but they are not yet parsed or interpreted.

For example:
- An HTTP request like `GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n` is sent as raw bytes over the network.
- Each character in the request (e.g., `G`, `E`, `T`, `/`, etc.) is represented as a byte in memory.

### Why Raw Bytes Matter:
1. **Low-Level Communication**:
   - Functions like `recv()` and `send()` operate at the raw byte level. They read or write binary data directly from/to sockets.

2. **Unstructured Data**:
   - Raw bytes are unstructured and need to be processed (e.g., converted to strings, parsed into headers) to make sense of the data.

3. **Efficiency**:
   - Working with raw bytes is efficient for transmitting data, but it requires additional processing to interpret the data.

### Example of Raw Bytes:
Imagine the following HTTP request:

```
GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n
```

In memory, this might look like:
```
71 69 84 32 47 105 110 100 101 120 46 104 116 109 108 32 72 84 84 80 47 49 46 49 13 10 72 111 115 116 58 32 101 120 97 109 112 108 101 46 99 111 109 13 10 13 10
```

Each number represents the ASCII value of a character (e.g., `71` is `G`, `69` is `E`, etc.).

### How Raw Bytes Are Used in Your Code:
1. **Receiving Data**:
   - The `recv()` function reads raw bytes from the socket and stores them in a buffer (e.g., `char buffer[1024]` or `std::vector<char>`).

2. **Processing Data**:
   - After receiving the raw bytes, you need to interpret them (e.g., convert them to a string and parse the HTTP request).

### Summary:
- Raw bytes are the unprocessed binary data transmitted over a network or stored in memory.
- Functions like `recv()` deal with raw bytes directly.
- You need to process raw bytes to extract meaningful information (e.g., HTTP method, headers, body).
