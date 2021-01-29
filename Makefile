# FIXME: make it smart
default:
	gcc -Iinc network/tcp.c network/network.c main.c -o csample
