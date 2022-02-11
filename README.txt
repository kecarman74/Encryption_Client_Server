To Compile and Run:

1. Navigate to the directory where main.c and Makefile.file are located.
2. Compile the program by running the compileall.sh file with in the terminal as such:

	./compileall.sh

3. Generate a key by running keygen with the command below:

	./keygen {Key_Len}

	where {Key_Len} is the length of key needed

4. Encrypt/Decrypt message by starting the server and then calling with the client as below. Example shows encrypting process, buy decrypt works exactly the same:
	

	./enc_server {port} &
	./enc_client {message} {key} {port}

	Where {port} is the port where the server is running
	{message} is the message file to be en/decrypted
	{key} is the key file to en/decrypt
