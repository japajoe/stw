#include "web_app.hpp"
#include "web_socket_app.hpp"

int main(int argc, char **argv)
{
	web_socket_app app;
	app.run();
	return 0;
}