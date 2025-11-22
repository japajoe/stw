# stw
Small library for making web based applications.

# Features
- HTTP client.
- HTTP server.
- Web socket implementation for client and server. 

# Requirements for Linux
- libcurl (libcurl.so)
- libssl (libssl.so)

# Requirements for Windows
- libcurl (libcurl-x64.dll)
- libssl (libssl-3-x64.dll)
- libcrypto (libcrypto-3-x64.dll)

On Linux you must have these libraries somewhere in your system path. On Windows you must place the .dll in the same directory as the executable.

# Example for http server configuration file
```
port 8080
port_https 8081
max_header_size 16384
bind_address 0.0.0.0
certificate_path cert.pem
private_key_path key.pem
public_html_path www/public_html
private_html_path www/private_html
host_name localhost
use_https true
use_https_forwarding true
```

# Disclaimer
This library is just for educational purposes.
