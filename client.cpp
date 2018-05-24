#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdio>
#include <cstring> 
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <forward_list>
#include "Lexem.h"
#include "Lex_Scanner.h"
#include "Syntax.h"
#include "Syntax_Errors.h"
using namespace std;

int s;
forward_list<Lexem> List;
forward_list<Lexem> :: iterator iter;
forward_list<Lexem> :: iterator end_iter;
char buffer[512];

void Handler(int c) {
		char *str = "/leave\n";
		write(s, str, strlen(str));
		exit(0);
}
 

bool Check_In_Buf(char *buf) {
	char *q;

	q = buf;
	while ((*q) and ((*q) != '#')) {
		q++;
	}
	if (not (*q))
		return(false);
	else
		return(true);
}

int main() {
	signal(SIGINT, Handler);
	int i, fd, port, num, max_d, already_read, n;
	char hostname[64], *str, c, *number;
	struct hostent *hp;
	struct sockaddr_in sin;
	Scanner LEX_SCANNER_OBJECT;
	Syntax SYNTAX_OBJECT;
	Lexem cur;
	bool answer_flag = true;

	printf("put port\n");
	scanf("%d", &port);
	gethostname (hostname, sizeof (hostname));
	if ((hp = gethostbyname (hostname)) == NULL) {
		fprintf(stderr, "%s: unknown host.\n", hostname);
		exit (1);
	}
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0 ) {
		perror("client: can't create socket");
		exit(1);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons (port);
	memcpy(&sin.sin_addr, hp -> h_addr, hp->h_length);
	if ( connect ( s, (struct sockaddr *)&sin, sizeof (sin)) < 0 ) {
		perror ("client: connect, try another port");
		exit (1);
	}
	max_d = s;
	for ( ; ; ) {
			if (not LEX_SCANNER_OBJECT.Check()) {
				printf("Wrong lexems\n");
				List.clear();
				continue;
			}
			List.reverse();
			iter = List.begin();
			end_iter = List.end();
			try {
				SYNTAX_OBJECT.Syntax_Analize();
			}
			catch (const Syntax_Errors &er) {
				cout<<"Syntax error: ";
				er.Get_Error_Message();
				List.clear();
				continue;
			}
			iter = List.begin();
			i = 0;
			while (iter != end_iter) {
				cur = *iter;
				iter++;
				buffer[i] = cur.get_type();
				i++;
				if (cur.get_type() == NUMBER) {
					num = cur.get_number();
					number = new char[200];
					sprintf(number, "%d", num);
					strcpy(buffer + i, number);
					i += (strlen(number) + 1);
					delete [] number;
					buffer[i] ='#';
					i++;
					continue;
				}
				if ((cur.get_type() == IDENTIFICATOR) or (cur.get_type() == CONST_STR)) {
					str = cur.get_str();
					strcpy(buffer + i, str);
					i += (strlen(str) + 1);
					delete []str;
					buffer[i] ='#';
					i++;
					continue;
				}
				else {
					buffer[i] = cur.get_number();
					i++;
					buffer[i] ='#';
					i++;
				}
			}
			buffer[i] = '&';
			i++;
			List.clear();
			n = write(s, buffer, i);
			
			already_read = read(s, buffer, sizeof(buffer));
			if ((already_read == -1) or (already_read == 0)) {
				return 0;
			}
			buffer[already_read] = '\0';
			while (Check_In_Buf(buffer) == false) {
				n = read(s, buffer + already_read, sizeof(buffer) - already_read);
				already_read += n;
				buffer[already_read] = '\0';
			}
			buffer[already_read - 1] = '\0';
			int wr_count = write(1, buffer, strlen(buffer));
			if (wr_count == - 1) {
				perror("write errror");
				exit(-1);
			}
	}
	return 0;
}
