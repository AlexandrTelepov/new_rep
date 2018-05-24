#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <forward_list>
#include <cstdio>
#include <cstring> 
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include "Lexem.h"
#include "Lex_Scanner.h"
#include "C_Where.h"
#include "Execute.h"
#include "Execute_Errors.h"
#include <iostream>
#include <errno.h>
int  max_count, d_arr[10];
forward_list<Lexem> List;
forward_list<Lexem> :: iterator iter;
forward_list<Lexem> :: iterator end_iter;


void Handler (int s) {
	char *str = "Server is closing now";
	for (int i = 0; i < 10; i++) {
		if (d_arr[i]) {
			write(d_arr[i], str, strlen(str));
			close(d_arr[i]);
		}
	}
	int pid = getpid();
	kill(pid, SIGKILL);
}

void Work_With_Socket(int sd) {
	int cur_r;
	char buf[256], *message;
	bool flag = false;
	int i = 0, num;
	char *str, *save, c;
	TYPE type;
	signal(SIGINT, Handler);

	//Scanner SCANNER_OBJECT;
	CARRY_OUT CARRY_OUT_OBJECT; 
	//SCANNER_OBJECT.Check(sd);

	cout<<"work with socket\n";
	cur_r = read(sd, buf, sizeof(buf));
	cout<<"read in server = "<<cur_r<<endl;
	if (cur_r == 0) {
		/*printf("%d\n", errno);
		while (cur_r == 0) {
			sleep(5);
			cout<<"want read again\n";
			cur_r = read(sd, buf, sizeof(buf));
		}
		*/
		//close(sd);
		return;
	}
	i = 0;
	while (not flag) {
		if (buf[i] == '&') {
			break;
		}
		cout<<"buf[i] == "<<(int)buf[i]<<endl;
		if ((buf[i] == 1) or (buf[i] == 8)) {
			if (buf[i] == 1)
				type = IDENTIFICATOR;
			else
				type = CONST_STR;
			i++;
			str = new char [2];
			str[0] = buf[i];
			str[1] = '\0';
			i++;
			while (buf[i] != '#') {
				save = new char [strlen(str) + 1];
				strcpy(save, str);
				delete [] str;
				str = new char [strlen(save) + 2];
				strcpy(str, save);
				str[strlen(save)] = buf[i];
				str[strlen(save) + 1] = '\0';
				i++;
				delete [] save;
			}
			//cout<<"after gain in server ="<<str<<endl;
			List.push_front(Lexem(type, str));
			delete []str;
			i++;
			continue;
		}
		if (buf[i] == 2) {
			i++;
			str = new char [2];
			str[0] = buf[i];
			str[1] = '\0';
			i++;
			while (buf[i] != '#') {
				save = new char [strlen(str) + 1];
				strcpy(save, str);
				delete [] str;
				str = new char [strlen(save) + 2];
				strcpy(str, save);
				str[strlen(save)] = buf[i];
				str[strlen(save) + 1] = '\0';
				i++;
				delete [] save;
			}
			num = atoi(str);
			delete [] str;
			//cout<<"after gain in server ="<<num<<endl;
			List.push_front(Lexem(NUMBER, num));
			i++;
			continue;
		}
		else {
			if (buf[i] == 0)
				type = KEYWORD;
			if (buf[i] == 3)
				type = DELIMETER;
			if (buf[i] == 4)
				type = OPERATOR;
			if (buf[i] == 5)
				type = BRACKETS;
			if (buf[i] == 6)
				type = OPERATION;
			if (buf[i] == 7)
				type = LOGIC_OPERATOR; 
			List.push_front(Lexem(type, buf[i + 1]));
			i += 3;
		}
	}
	List.reverse();
	end_iter = List.end();
	iter = List.begin();
	try {
		CARRY_OUT_OBJECT.Carry_Out(sd);
	}
	catch (const Execute_Errors &er) {
		message = er.Get_Error_Message();
		write(sd, message, strlen(message));
		delete [] message;
	}
	List.clear();
	c = '#';
	write(sd, &c, 1);
	//cout<<"end work with socket\n";
	return;
}

int main() {
	int res, sd, port, new_soc, working_sockets = 0, i;
	char hostname[64];
	struct hostent *hp;
	char *send_mes;
	fd_set readfds;
	signal(SIGINT, Handler);

	for (i = 0; i < 10; i++) {
		d_arr[i] = 0;
	}
	printf("put max count of clients\n");
	scanf("%d", &max_count);
	printf("put port\n");
	scanf("%d", &port);

	gethostname (hostname, sizeof (hostname));
 	if ((hp = gethostbyname (hostname)) == NULL) {
 		fprintf (stderr, "%s: unknown host.\n", hostname);
 		exit(-1);
 	}
	if (-1 == (sd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("server can't create socket");
		exit(-1);
	} 

	struct sockaddr_in addr;
	struct sockaddr_in son_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (-1 == bind(sd, (struct sockaddr *)&addr, sizeof(addr))) {
		perror("server can't bind socket");
		exit(-1);
	}

	if (-1 == listen(sd, 5)) {
		perror("can't made socket listening");
		exit(-1);
	}
	//set up socket
	for( ; ; ) {	//main cicle
		int max_d = sd;
		FD_ZERO(&readfds); 
		FD_SET(sd, &readfds);
		int fd;
		i = 0;
		for (fd = d_arr[0] ; i < max_count ; fd = d_arr[++i]) { 
			if (d_arr[i] != 0) {
				FD_SET(fd, &readfds);
				if (fd > max_d) 
					max_d = fd;
			}
		}
		res = select(max_d + 1, &readfds, NULL, NULL, NULL);
		if(res < 1) {
			perror("select server");
		}
		if (FD_ISSET(sd, &readfds)) {
			if (-1 == (new_soc = accept(sd, NULL, NULL)) ) {
				perror("can't accept");
				exit(-1);
			}
			working_sockets++;
			int j = 0;
			while ((d_arr[j] != 0) && (j < max_count)) {
				j++;
			}
			if (j == max_count) {
				char cstr2[] ="Max count of clients already achieved\n";
				write(new_soc, cstr2, sizeof(cstr2));
				close(new_soc);
				working_sockets--;
			}
			else {
				d_arr[j] = new_soc;
				if (new_soc > max_d)
					max_d = new_soc;
			}
		}

		i = 0;
		for (fd = d_arr[0] ; i < max_count ; fd = d_arr[++i]) {
			if (FD_ISSET(fd, &readfds)) {
				Work_With_Socket(fd);
			}
		}
	}
	close(sd);
	return(0);
}
