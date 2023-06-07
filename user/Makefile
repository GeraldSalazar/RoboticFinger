#gcc -I../static-lib/include RoboticFinger/user/server/server.c ../static-lib/lib/rob-finger-lib.a -o server
#gcc -I../static-lib/include RoboticFinger/user/server/server.c ../static-lib/lib/rob-finger-lib.a -o server
all: bs bc
bs:
	gcc -I../static-lib/include  -o bin/server server/server.c ../static-lib/lib/rob-finger-lib.a encrypt/aes-encrypt.c -Iencrypt -lssl -lcrypto -Wno-deprecated-declarations

bc:
	gcc -o bin/client client/client.c encrypt/aes-encrypt.c -Iencrypt -lssl -lcrypto -Wno-deprecated-declarations

runc:
	bin/client

runs:
	bin/server