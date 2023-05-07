all: bs bc
bs:
	gcc -o bin/server server/server.c encrypt/aes-encrypt.c -Iencrypt -lssl -lcrypto

bc:
	gcc -o bin/client client/client.c encrypt/aes-encrypt.c -Iencrypt -lssl -lcrypto

runc:
	bin/client

runs:
	bin/server