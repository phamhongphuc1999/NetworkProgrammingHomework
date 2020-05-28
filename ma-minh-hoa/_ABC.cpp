SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
	SESSION session; session.username = new char[BUFF_SIZE];
	session.password = new char[BUFF_SIZE];
	session.numberOfError = 0; 
	session.type = 1; 
	session.location = -1;
	char username[2048], password[2048];
	char* result = new char[10];
	while (true) {
		ret = RECEIVE_TCP(connSock, buff, 0);
		if (ret <= 0) {
			if (ret == 0) printf("Client close connection\n");
			else printf("Connection shutdown\n");
			if (session.type == 0) {
				printf("The username: %s has not logged out, will perform automatic logout\n", session.username);
			}
			closesocket(connSock);
			break;
		}
		else {
			buff[ret] = 0;
			if (CheckDataFromClient(buff)) {
				strcpy_s(result, 4, "001");
				int ret = SEND_TCP(connSock, AddHeader(buffSend, result), 0);
				if (ret == SOCKET_ERROR) printf("can not send message\n");
			}
			else {
				if (buff[0] == '1') {
					Slip(&buff[1], username, password);
					printf("Request: Login[username: %s, password: %s]\n", username, password);
					result = CHECK_CLIENT_LOGIN(&session, username, password);
					if (!strcmp(result, "110")) {
						result = LOGIN(&session, username, password);
					}
				}
				else if (buff[0] == '2') {
					printf("Request: Logout[username: %s]\n", &buff[1]);
					result = LOGOUT(&session, &buff[1]);
				}

				printf("%s\n", result);
				int ret = SEND_TCP(connSock, AddHeader(buffSend, result), 0);
				if (ret == SOCKET_ERROR) printf("can not send message\n");
			}
		}
	}
	closesocket(listenSocket);
	WSACleanup();