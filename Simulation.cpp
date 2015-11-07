/*this file is the main function of filesystem*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "File_sys.h"


int main()
{
		char str[10];
		char strname[10];
		char c;

		printf("FormatDisk?<y/n>");
		scanf("%c",&c);
		fflush(stdin);
		if(c=='y')
		{
			if(!Format())
			{
				return -1;
			}
			printf("Finished!\n");
		}

		if(!Install())
		{
			return -1;
		}
		printf("login now\n");
		login();
		showhelp();
		printf("%s>",cmdhead);
		while(1)
		{
			scanf("%s",&str);
			if(strcmp(str,"shutdown")==0)
			{
				fclose(fd);
				return 0 ;
			}
			else	if(strcmp(str,"dir")==0)
					{
						showdir();
					}
			else if(strcmp(str,"bit")==0)
			{
					showbitmap();
			}

			else if(strcmp(str,"help")==0)
			{
					showhelp();
			}
			else if(strcmp(str,"logout")==0)
			{
				logout();
			}
			else	if(Iscmd(str))
			{
				scanf("%s",&strname);
				cmd_Up(str,strname);
			}

			else
			{
				printf("Error!!\n");
			}
			printf("%s>",cmdhead);
		}
return 0;
}
