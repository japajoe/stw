# stw
Small library for making web based applications.

# Features
- HTTP server.
- HTTP client.
- Web socket implementation for client and server.
- Template library (`templ`) that lets you mix HTML and C++.

# Requirements for Linux
- libcurl (libcurl.so)
- libssl (libssl.so)
- libcrypto (libcrypto.so)

# Requirements for Windows
- libcurl (libcurl-x64.dll)
- libssl (libssl-3-x64.dll)
- libcrypto (libcrypto-3-x64.dll)

No linking is required with any libraries, but on Linux you must have these libraries somewhere in your system path. On Windows you must place the dll files in the same directory as the executable. MacOS is not fully supported (yet).

# Generating certificate with openssl
```
openssl req -newkey rsa:4096 -x509 -sha256 -days 3650 -nodes -out cert.pem -keyout key.pem
```

# Mixing HTML and C++
```php
<?cpp
#include <map>

std::string title = "Simple website";

std::map<std::string,std::string> posts = {
    { "First Blog Post", "This is the content of the first post."},
    { "Second Blog Post", "This is the content of the second post."}
};
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title><?cpp echo(title); ?></title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 20px;
        }
        h3 {
            color: #333;
        }
        p {
            color: #666;
        }
        footer {
            margin-top: 20px;
            font-size: 0.8em;
            color: #888;
        }
    </style>
</head>
<body>
	<h1>Simple web page</h1>
    <p>This is a simple example of an HTML page. You can modify the content as per your needs.</p>
	<ul>
		<?cpp for(auto &post : posts) { ?>
			<li>
				<h3><?cpp echo(post.first); ?></h3>
				<p><?cpp echo(post.second); ?></p>
			</li>
		<?cpp } ?>
	</ul>

    <footer>
        <p>Powered by stw and temple</p>
    </footer>
</body>
</html>
```

# Disclaimer
This library is just for educational purposes.
