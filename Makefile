all: enc_client enc_server dec_client dec_server keygen

keygen:
	gcc keygen.c -o keygen

enc_client: enc_client.c utils.c
	gcc -o enc_client enc_client.c utils.c

enc_server: enc_server.c utils.c
	gcc -o enc_server enc_server.c utils.c

dec_client: dec_client.c utils.c
	gcc -o dec_client dec_client.c utils.c

dec_server: dec_server.c utils.c
	gcc -o dec_server dec_server.c utils.c