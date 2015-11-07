/*
this file contains the declaration and definition of functions of the filesystem. 

*/
#include <stdio.h>
#define BLKSIZE    512		//the size of block
#define DATABLKNUM  512		//the number of block
#define BLKGROUPNUM	50		//group number
#define P_BLOCKS	15		//per_block_point
#define	GROUPNUM	DATABLKNUM/BLKGROUPNUM+1 //group number

#define IDNODESIZE   512		//the size of inode
#define IDNODENUM   32		//iSize

#define SYSOPENFILE 40		
#define DIRNUM      32		//maximum file in a directory
#define DIRSIZ      14		//directory size
#define UPWDSIZE     15		//length of password
#define UNAMSIZ     15		//length of username
#define PWDSIZE		sizeof(struct pwd) //length of password strcture       
#define PWDNUM      BLKSIZE/PWDSIZE		
#define NOFILE      20		
#define NHINO       128	
#define USERNUM     10		//length od username
#define IDNODESTART 4*BLKSIZE	//inodes table 
#define DATASTART   (2+IDNODENUM)*BLKSIZE	//start adress of data
#define	DATASTARTNO	36		//pointer to the start of data

/*  di._mode  */
#define IDMODE_EMPTY	00000
#define IDMODE_FILE		00001
#define IDMODE_DIR      00002
#define IDMODE_PASSWD	00004
#define IDMODE_SYSTEM	00040	/*system file*/

#define IDMODE_READ     00010	/*  READ  */
#define IDMODE_WRITE    00020	/*  WRITE  */
#define IDMODE_EXICUTE  01000	/*  EXICUTE  */
#define IDMODE_ADMIN	00100	/*root*/
#define IDMODE_SHARE	00200	/**/
/*group*/
#define GRUP_0			0	//admin
#define GRUP_1			1
#define GRUP_2			2
#define GRUP_4			4


/*
	strcture of directory
*/
struct dir{
		struct direct* direct[DIRNUM];
		int size;
		};

/*
	directory
*/
struct direct{
		char	d_name[DIRSIZ];	/*the name of the directory*/
		int d_ino;	/*number of the directory*/
		};
/*
	superblock
*/
struct super_block{
		int s_inodes_count;      
		int s_blocks_count;      
		int s_r_blocks_count;    
		int s_free_blocks_count; 
		int s_free_inodes_count; 
		int s_free_blocks_group[GROUPNUM];
		int s_first_data_block;  
		int s_log_block_size;    
		int s_blocks_per_group;  
		int s_inodes_per_group;  
		};

/*
	user password
*/
struct pwd{
		int p_uid;
		int p_gid;
		char username[UNAMSIZ];/*proj,os*/ 
		char password[UPWDSIZE];
		};
/*
	inode
*/

struct inode{
		 
		 int  di_ino;	/*identifier*/
	     int  di_number;	/*number of file related*/
	     int  di_mode;	/*store*/
	     int  di_uid;	/* user id*/
	     int  di_gid;	/* right id*/ 
	     int  di_size;	/*size of the file*/
		 int  di_ctime;   /* Creation time */
		 int  di_mtime;   /* Modification time */
		 int  di_block[P_BLOCKS]; /* pointers */
	     };


//		


struct super_block filsys;				//superblock
struct pwd pwd[PWDNUM];	

int di_bitmap[IDNODENUM];	// bitmap of inodeblock
int	bk_bitmap[DATABLKNUM];	// bitmap of data block  

FILE   *fd;								//file pointer
struct inode *cur_inode;						
struct inode *inodetemp;				
			

const char   fsystemname[20]="Linux";	
struct direct dir_buf[BLKSIZE / sizeof(struct direct)];	
char cmdhead[20];//the directory in
int i_lock=0;//
int b_lock=0;
struct pwd *cur_user;	

struct inode * read_inode(int);
struct direct * read_dir_data(int);
extern  void showdir();
char * ReadFile(char[]);
void changeinode();
void login();
void logout();
int cdir(char[]);
void showbitmap();
int deletefd(char[]);
int editfile(char[]);
void showhelp();
int Enterdir(char[]);
int search_fl(char[]);
int Iscmd(char[]);
void cmd_Up(char[],char[]);
int creat(char[]);
int access();

/*		global function		*/
extern int	Format();
extern int	Install();//startup
 
int		ialloc();/*create a new inode*/
int		balloc(int);//apply the space
void showaccess(char strname[20]);

//function definition

void changeinode()
{
	struct inode *intemp;
	intemp=cur_inode;
	cur_inode=inodetemp;
	inodetemp=intemp;
}

int Format()
{
	struct pwd passwd [BLKSIZE/PWDSIZE]; //pwdnum

	int i;
	
	/*	creat the file system file */
	fd = fopen (fsystemname, "wb");/*read or write a file*/
	
	if(fd==NULL)
	{
		printf("format failed!\n");
		return 0;
	}
	//superblock
	filsys.s_inodes_count=IDNODENUM ;      
	filsys.s_blocks_count=DATABLKNUM;      
	filsys. s_r_blocks_count=0;    
	filsys. s_free_blocks_count=DATABLKNUM-5;
	filsys.s_free_blocks_group[0]=50-5;
	for(i=1;i<GROUPNUM-1;i++)
	{
		filsys.s_free_blocks_group[i]=50; 
	
	}
	filsys.s_free_blocks_group[GROUPNUM-1]=12; //last group
	filsys.s_free_inodes_count=IDNODENUM-5; 
	filsys.s_first_data_block=DATASTARTNO;  
	filsys.s_log_block_size=BLKSIZE;    
	filsys.s_blocks_per_group=BLKGROUPNUM;  
	filsys.s_inodes_per_group=0;  
	
	fseek(fd, BLKSIZE, SEEK_SET);
	fwrite (&filsys,BLKSIZE, 1,fd);
	
	//initialize
	di_bitmap[0]=1;
	di_bitmap[1]=1;
	di_bitmap[2]=1;
	di_bitmap[3]=1;
	di_bitmap[4]=1;

	bk_bitmap[0]=1;
	bk_bitmap[1]=1;
	bk_bitmap[2]=1;
	bk_bitmap[3]=1;
	bk_bitmap[4]=1;

	for(i=5;i<IDNODENUM;i++)
	{
		di_bitmap[i]=0;
		bk_bitmap[i]=0;
	}
	for(;i<DATABLKNUM;i++)
	{
		bk_bitmap[i]=0;
	}
	fseek(fd, BLKSIZE*2, SEEK_SET);
	fwrite (di_bitmap,BLKSIZE, 1,fd);
	fseek(fd, BLKSIZE*3, SEEK_SET);
	fwrite (bk_bitmap,BLKSIZE, 1,fd);


	
	struct 	inode *ininode;
	ininode=(struct inode *)malloc(sizeof (struct inode));
	if(!ininode)
	{
		printf("ininode Error!");
		return 0;
	}
	//	strcpy(dinodef->di_name,"/");
	ininode->di_ino=0;//identifier of inode
	ininode->di_number=3;//three files are rellated
	ininode->di_mode=IDMODE_DIR|IDMODE_SYSTEM;//0 is directory
	ininode->di_uid=1;//the first user
	ininode->di_gid=1;//
	ininode->di_size=0;
	ininode->di_ctime=0;   /* Creation time */
	ininode->di_mtime=0;   /* Modification time */
	ininode->di_block[0]=0;

	fseek(fd,IDNODESTART, SEEK_SET);
	fwrite(ininode,sizeof(struct inode), 1, fd);

	
	strcpy(dir_buf[0].d_name, ".");
	dir_buf[0].d_ino= 0;//current inode number
	strcpy(dir_buf[1].d_name,"..");
	dir_buf[1].d_ino= 0;
	strcpy(dir_buf[2].d_name, "etc");
	dir_buf[2].d_ino = 1;//etc
	
	fseek(fd, DATASTART, SEEK_SET);
	fwrite(dir_buf, BLKSIZE, 1, fd);

	//etc
	ininode->di_ino=1;
	ininode->di_number=5;//
	ininode->di_gid=1;
	ininode->di_block[0]=1;
	fseek(fd, IDNODESTART+BLKSIZE, SEEK_SET);
	fwrite(ininode,sizeof(struct inode), 1, fd);
	
	strcpy (dir_buf[0].d_name, ".");
	dir_buf[0].d_ino = 1;
	strcpy(dir_buf[1].d_name, "..");
	dir_buf[1].d_ino = 0;
 	strcpy(dir_buf[2].d_name, "passwd");
 	dir_buf[2].d_ino = 2;
	strcpy(dir_buf[3].d_name, "admin");
 	dir_buf[3].d_ino = 3;
	strcpy(dir_buf[4].d_name, "proj");
 	dir_buf[4].d_ino = 4;

	fseek(fd, DATASTART+BLKSIZE, SEEK_SET);
	fwrite (dir_buf, BLKSIZE,1,fd);
	
	// admin 
	ininode->di_ino=3;
	ininode->di_number=2;//
		ininode->di_gid=0;
	ininode->di_block[0]=3;
	fseek(fd, IDNODESTART+BLKSIZE*3, SEEK_SET);
	fwrite(ininode,sizeof(struct inode), 1, fd);
	
	strcpy (dir_buf[0].d_name, ".");
	dir_buf[0].d_ino = 3;
	strcpy(dir_buf[1].d_name, "..");
	dir_buf[1].d_ino = 1;

	
	fseek(fd, DATASTART+BLKSIZE*3, SEEK_SET);
	fwrite (dir_buf, BLKSIZE,1,fd);
	
	// proj my 
	ininode->di_ino=4;
	ininode->di_number=2;
	ininode->di_uid=2;
	ininode->di_gid=1;
	ininode->di_block[0]=4;
	fseek(fd, IDNODESTART+BLKSIZE*4, SEEK_SET);
	fwrite(ininode,sizeof(struct inode), 1, fd);
	
	strcpy (dir_buf[0].d_name, ".");
	dir_buf[0].d_ino =4;
	strcpy(dir_buf[1].d_name, "..");
	dir_buf[1].d_ino = 1;

	fseek(fd, DATASTART+BLKSIZE*4, SEEK_SET);
	fwrite (dir_buf, BLKSIZE,1,fd);


	//password file
	passwd[0].p_uid= 1; 
	passwd[0].p_gid = GRUP_0; 
	strcpy(passwd[0].username, "root");
	strcpy(passwd[0].password, "root");
	
	passwd[1].p_uid= 2;
	passwd[1].p_gid = GRUP_1;
	strcpy(passwd[1].username, "proj");
	strcpy(passwd[1].password, "os");


	for (i=2; i<PWDNUM; i++)
	{
		passwd[i].p_uid = 0;
		passwd[i].p_gid = GRUP_4;
		strcpy(passwd[i].username, "User not found!");
		strcpy(passwd[i].password,"");
	}
	fseek(fd,DATASTART+BLKSIZE*2, SEEK_SET);
	fwrite(passwd,BLKSIZE,1,fd);
	
	ininode->di_ino=2;
	ininode->di_number=2;
	ininode->di_mode=IDMODE_PASSWD|IDMODE_SYSTEM;//
	ininode->di_uid=1;
	ininode->di_gid=1;
	ininode->di_size=BLKSIZE;
	ininode->di_ctime=0;   /* Creation time */
	ininode->di_mtime=0;   /* Modification time */
	ininode->di_block[0]=2;
	fseek(fd, IDNODESTART+BLKSIZE*2, SEEK_SET);
	fwrite(ininode,sizeof(struct inode), 1, fd);
	
	fclose(fd);
	free(ininode);
	return 1;
}

int	Install()
{
	int i;
	//struct inode *inode_temp;
	printf("Intalling the file System\n");
	fd = fopen (fsystemname, "rb+");
	if(fd==NULL)
	{
		printf("Failed when Open file\n");
		return 0;	
	}
	
	
	fseek(fd,BLKSIZE,SEEK_SET);
	fread(&filsys,sizeof(struct super_block),1,fd);


	inodetemp=(struct inode *)malloc(sizeof (struct inode));
	if(!inodetemp)
	{
		printf("inodetemp memory allocation failed!\n");
		return 0;
	}
	cur_inode=(struct inode *)malloc(sizeof (struct inode));
	if(!cur_inode)
	{
		printf("cur_inode memory allocation failed!\n");
		return 0;
	}

	//read inode bitmap
	fseek(fd,BLKSIZE*2,SEEK_SET);
	fread(di_bitmap,BLKSIZE,1,fd);
	//read block bitmap
	fseek(fd,BLKSIZE*3,SEEK_SET);
	fread(bk_bitmap,BLKSIZE,1,fd);

	//read user passwd file's inode
	inodetemp=read_inode(2);
	if(inodetemp==NULL)
	{
		return 0;
	}
	changeinode();//change pointer
	//read password
	fseek(fd, DATASTART+BLKSIZE*cur_inode->di_block[0], SEEK_SET);
	fread(pwd,BLKSIZE, 1, fd);
	printf("---------------------------------------\n");
	for(i=0;i<PWDNUM;i++)
	{
	//	usernum=0;
		if(pwd[i].p_uid!=0)
		{
		//	usernum++;
			printf("User %d:%s\nPassword:%s\n",i+1,&pwd[i].username,&pwd[i].password);
			printf("---------------------------------------\n");
		}		
	}
	
	inodetemp=read_inode(0);
	if((inodetemp->di_mode&IDMODE_DIR)!=IDMODE_DIR)
	{
		printf("Read Directory failed, Please reFormat!\n");
	}
	else
	{
		changeinode();
		read_dir_data(cur_inode->di_block[0]);
	}
	
	return 1;
}

/*----------------------------------
read data block
----------------------------------*/
struct direct * read_dir_data(int n)
{
	
	fseek(fd, DATASTART+BLKSIZE*n, SEEK_SET);
	fread (dir_buf, cur_inode->di_number*(sizeof(struct direct)),1,fd);
	
	return dir_buf;
}

struct inode * read_inode(int n)
{
	int i;

	fseek(fd,IDNODESTART+BLKSIZE*n,SEEK_SET);
	fread(inodetemp,sizeof(struct inode),1,fd);

	if(inodetemp->di_ino!=n)
	{
		printf("size=%d,number=%d,block=%d\n",inodetemp->di_size,inodetemp->di_number,inodetemp->di_block[0]);
		printf("Read inode failed! not %d,but %d\n",inodetemp->di_ino,n);
		return NULL;
	}

	i=0;
	do{
		inodetemp->di_block[i]=inodetemp->di_block[i];
			if(i>=P_BLOCKS-3)//too big file
		{
			printf("The file is too big!\n");
			break;
		}
		i++;
	}while(i<(int)inodetemp->di_size/BLKSIZE);
	return inodetemp;
	
}

void showdir()
{
	int i;

	for(i=0;i<cur_inode->di_number;i++)
	{
		
		if(i==0)
		{
			printf("\t.\t\t\t<dir>\tinode %d\n",dir_buf[i].d_ino);
			
		}
		else	if(i==1)	
		{
			printf("\t..\t\t\t<dir>\tinode %d\n",dir_buf[i].d_ino);
		}
		else
		{
			
			inodetemp=read_inode(dir_buf[i].d_ino);
			if((inodetemp->di_mode&IDMODE_DIR)==IDMODE_DIR)
			{
				printf("\t%s\t\t\t<dir>\tinode %d\n",dir_buf[i].d_name,dir_buf[i].d_ino);
			}
			else if((inodetemp->di_mode&IDMODE_FILE)==IDMODE_FILE)
			{
				printf("\t%s\t\t\t<file>\tsize %d block %d\n",dir_buf[i].d_name,inodetemp->di_size,inodetemp->di_block[0]);
			}
			else
			{
				printf("\t%s\t\t\t<passwd>inode %d block %d\n",dir_buf[i].d_name,dir_buf[i].d_ino,inodetemp->di_block[0]);
			}
			
		}
	}
}

int Enterdir(char* namestr)//enter the directory
{
	int i;
	i=search_fl(namestr);
	if(i!=-1)
	{
		
		read_inode(i);
		if((inodetemp->di_mode&IDMODE_DIR)!=IDMODE_DIR)
		{
			printf("%s is not a directory!\n",namestr);
			return -1;
		}
	
		if(!access())
		{	
			printf("You do not have the priority!\n");
			return -1;
		}

		changeinode();
	}
	else
	{
		printf("File not found\n");
		return -1;
	}
	read_dir_data(cur_inode->di_block[0]);
	
	return 1;	
}

int search_fl(char namestr[20])//look for file
{
	
	int i=0;
	do{	
		
		if(strcmp(".",namestr)==0)
		{
			i=0;
			break;
		}
		if(strcmp("..",namestr)==0)	
		{
					i=1;
					break;
		}
		if(strcmp(dir_buf[i].d_name,namestr)==0)
		{
			if(dir_buf[i].d_ino!=-1)
			{
				
				break;
			}
		
		}
		i++;
	}while(i<cur_inode->di_number);
	if(i==cur_inode->di_number)
	{
		return(-1);
	}
 return (dir_buf[i].d_ino);
}
int Iscmd(char cmd[10])
{
//	int i;
	if(!strcmp(cmd,"cd") || 
		!strcmp(cmd,"cdir") || 
		!strcmp(cmd,"create") || 
		!strcmp(cmd,"read") || 
		!strcmp(cmd,"edit") || 
		!strcmp(cmd,"del")||
		!strcmp(cmd,"attr"))
		
		return 1;
	else
		return 0;
}


void cmd_Up(char str[10],char strname[14])
{
	int l,i,itemp;
	l=strlen(strname);
	if(l>=14)
	{
		printf("the filename is too long!\n");
		return;
	}
	if(strcmp(str,"cd")==0)
	{
		if(Enterdir(strname)>=0)
		{
			
			l=strlen(cmdhead);
		
		
			if(strcmp(strname,"..")==0)
			{
					if(strcmp(cmdhead,"root")!=0)
					{
						i=0;					
						while(cmdhead[i]!='\0')
						{

							if(cmdhead[i]=='\\')
								{
									itemp=i;
									
								}
							i++;
						}
						cmdhead[itemp]='\0';
						
					}
					
			}
			else if(strcmp(strname,".")!=0)
			{
			
				strcat(cmdhead,"\\");
				strcat(cmdhead,strname);
			}
		}
		else
		{
			printf("Failed to enter the directory!\n");
		}
		
	}
	else if(strcmp(str,"create")==0)
	{
		if(creat(strname))
			{
			
			}
			else
			{	
				printf("Error to creat file!\n");
			}
		
	}
	else if(strcmp(str,"read")==0)
	{
		char * buf;
		buf=ReadFile(strname);
		if(buf==NULL)
		{
			printf("Error!\n");
		}
		else
		{
			printf("%s\n",buf);
		}
		
		free(buf);
	}
	else if(strcmp(str,"cdir")==0)
	{
		if(cdir(strname))
		{
		
		}
		else
		{
			printf("Error!\n");
		}
	}
	else if(strcmp(str,"del")==0)
	{
		if(deletefd(strname))
		{
		}
		else
		{
			printf("%sError!\n",strname);
		}


	}
	else if(strcmp(str,"edit")==0)
	{
		if(editfile(strname))
		{
				printf("%s successsfuly stored!\n",strname);
		}
		else
		{
				printf("Failed to store %s!\n",strname);
		}

	}
	else if(strcmp(str,"attr")==0)
	{
		showaccess(strname);
	}
	else
	{
		printf("Error!\n");
	}
}
/*-----------------
create a free inode
--------------------*/
int ialloc()
{
	int i;
	if(i_lock==1)//deal with the deadlock
	{
		printf("Try Again\n");
		i_lock=0;
		return -1;
	}
	i_lock=1;	
	if(filsys.s_free_inodes_count<=0)
	{
		printf("inode is already full now\n");
		i_lock=0;
		return -1;
	}


	//find a idole inode
	for(i=0;i<IDNODENUM;i++)
	{
		if(di_bitmap[i]==0)
		{
		
			di_bitmap[i]=1;
			filsys.s_free_inodes_count--;
			i_lock=0;
			return i;
			
		}
	}


	i_lock=0;
	return -1;
	
}





/*-------------------------
balloc
request storage space
------------------------*/
int  balloc(int k)
{
	printf("%d",k);
	int bnum,i,j,n,g;
	if(b_lock==1)
	{
		printf("Try Again!\n");
		return -1;
	}
	b_lock=1;//¼ÓËø
	n=BLKGROUPNUM;
	if(filsys.s_free_blocks_count<=0)
	{
		printf("The storage is already full now\n");
		b_lock=0;
		return -1;
	}

	
	for(i=0;i<GROUPNUM;i++)
	{
		if(filsys.s_free_blocks_group[i]<k)
		{
			continue;
		}
		if(i>=GROUPNUM-1)
		{
			n=DATABLKNUM%BLKGROUPNUM;
		}
		g=0;
		for(j=0;j<n;j++)
		{	
			if(bk_bitmap[i*GROUPNUM+j]==0)
			{
				bnum=i*GROUPNUM+j;
				g++;
			
				filsys.s_free_blocks_group[i]--;
				if(g==k)
				{
					b_lock=0;
					printf("block %d at group %d\n",bnum,i);
					
					for(i=0;i<k;i++)
					{
						bk_bitmap[bnum-i]=1;
					}
				
					return bnum-k+1;
				
				}
			
			}
			else
			{
				g=0;
			}
		}
	}

	printf("error!\n");
	b_lock=0;
	return -1;
	
}

/*-----------------
create file
-----------------*/
int creat(char strname[14])
{
	int fi,inum,bnum;
	fi=search_fl(strname);//check existance
	if(fi!=-1)
	{
		printf("%s already exist!\n",strname);
		return 0;
	}

	fflush(stdin);//input
	char *buf;
	int i,k;
	i = 0;k=0;

	buf = (char *)malloc(BLKSIZE*sizeof(char));
	
	printf("Input now and end with \"###\":\n");
	while(k!=3)
	{
		buf[i] = getchar();
		if(buf[i] == '#')
		{
			k++;
			if(k == 3)
				break;
		}
		else
			k=0;
		i++;
		if(i/BLKSIZE > 0)
		{
			buf = (char *)realloc(buf,BLKSIZE*(i/BLKSIZE+1));
		}
	}
	buf[i-2]='\0';

	printf("buf length %d\n",strlen(buf));

	inum=ialloc();
	if(inum<=-1)
	{
		printf("inode failed!\n");
		return 0;
	}

	bnum=balloc(strlen(buf)/BLKSIZE+1);
	if(bnum<=-1)
	{
		printf("block failed!\n");
		
		di_bitmap[inum]=0;
		filsys.s_free_inodes_count++;
		return 0;
		}

	inodetemp->di_ino=inum;

	inodetemp->di_number=1;
	inodetemp->di_mode=IDMODE_FILE;
	inodetemp->di_uid=cur_user->p_uid;
	inodetemp->di_gid=cur_user->p_gid;
	inodetemp->di_size=strlen(buf);//
	inodetemp->di_ctime=0;   /* Creation time */
	inodetemp->di_mtime=0;   /* Modification time */
	printf("cu=%d,cg=%d\n",	inodetemp->di_uid,inodetemp->di_gid);
	for(i=0;i<(int)inodetemp->di_size/BLKSIZE+1;i++)
	{
		inodetemp->di_block[i]=bnum+i;
	}
	fseek(fd,DATASTART+BLKSIZE*bnum, SEEK_SET);//write into storage.data block
	fwrite(buf,BLKSIZE*(inodetemp->di_size/BLKSIZE+1), 1, fd);
	
	fseek(fd,IDNODESTART+BLKSIZE*inum, SEEK_SET);//write the file into inodeblock
	fwrite(inodetemp,BLKSIZE, 1, fd);
	
	dir_buf[cur_inode->di_number].d_ino=inum;
	strcpy(dir_buf[cur_inode->di_number].d_name,strname);
	
	fseek(fd,DATASTART+BLKSIZE*cur_inode->di_block[0], SEEK_SET);
	fwrite(dir_buf,BLKSIZE, 1, fd);

	cur_inode->di_number++;
	printf("There are %d files here\n",cur_inode->di_number);

	fseek(fd,IDNODESTART+BLKSIZE*cur_inode->di_ino, SEEK_SET);
	fwrite(cur_inode,sizeof(struct inode), 1, fd);

	filsys.s_free_blocks_count--;
	filsys.s_free_blocks_group[inodetemp->di_block[0]/GROUPNUM]--;
	filsys.s_free_inodes_count--;
	
	
	fseek(fd, BLKSIZE, SEEK_SET);
	fwrite (&filsys,BLKSIZE, 1,fd);

	fseek(fd, BLKSIZE*2, SEEK_SET);//update bitmap
	fwrite (di_bitmap,BLKSIZE, 1,fd);
	
	fseek(fd, BLKSIZE*3, SEEK_SET);//update bitmap
	fwrite (bk_bitmap,BLKSIZE, 1,fd);
	free(buf);
	return 1;
}





void	bfree(int bnum)
 {

 }

char* ReadFile(char strname[14])
{
	int fi;
	fi=search_fl(strname);
	if(fi!=-1)
	{
	
	}
	else
	{
		printf("Can not find %s\n",strname);
		return NULL;
	}
	inodetemp=read_inode(fi);
	if(!access())
	{	
			printf("Error!\n");
			return NULL;
	}

	if(inodetemp==NULL)
	{
		return NULL;
	}
	if((inodetemp->di_mode&IDMODE_FILE)!=IDMODE_FILE)
	{
		printf("%s is not a file!\n",strname);
		return NULL;
	}

	char *buf;
	buf = (char *)malloc(BLKSIZE*(inodetemp->di_size/BLKSIZE+1));
	
	fseek(fd, DATASTART+BLKSIZE*inodetemp->di_block[0], SEEK_SET);
	fread (buf, BLKSIZE*(inodetemp->di_size/BLKSIZE+1),1,fd);

	return buf;
}

int cdir(char strname[])
{
	int fi,inum,bnum;
	fi=search_fl(strname);
	if(fi!=-1)
	{
		printf("%s already exist\n",strname);
		return 0;
	}
	
	
	inum=ialloc();
	if(inum<=-1)
	{
		printf("inode failed!\n");
		return 0;
	}
	
	
	bnum=balloc(1);
	if(bnum<=-1)
	{
		printf("block failed!\n");
		di_bitmap[inum]=0;
		filsys.s_free_inodes_count++;
		return 0;
	}
	
	inodetemp->di_ino=inum;
	inodetemp->di_number=2;
	inodetemp->di_mode=IDMODE_DIR;//
	inodetemp->di_uid=cur_user->p_uid;//
	inodetemp->di_gid=cur_user->p_gid;// 
	inodetemp->di_size=0;//
	inodetemp->di_ctime=0;   /* Creation time */
	inodetemp->di_mtime=0;   /* Modification time */
	inodetemp->di_block[0]=bnum;
	
	fseek(fd,IDNODESTART+BLKSIZE*inum, SEEK_SET);
	fwrite(inodetemp,BLKSIZE, 1, fd);
	
	struct direct buf[BLKSIZE / sizeof(struct direct)];

	strcpy(buf[0].d_name,".");
	buf[0].d_ino= inum;
	strcpy(buf[1].d_name,"..");
	buf[1].d_ino= cur_inode->di_ino;
	
	fseek(fd,DATASTART+BLKSIZE*bnum, SEEK_SET);
	fwrite(buf,BLKSIZE, 1, fd);

	
	dir_buf[cur_inode->di_number].d_ino=inum;
	strcpy(dir_buf[cur_inode->di_number].d_name,strname);
	
	fseek(fd,DATASTART+BLKSIZE*cur_inode->di_block[0], SEEK_SET);
	fwrite(dir_buf,BLKSIZE, 1, fd);
	
	cur_inode->di_number++;
	printf("There are %d files\n",cur_inode->di_number);

	
	fseek(fd,IDNODESTART+BLKSIZE*cur_inode->di_ino, SEEK_SET);
	fwrite(cur_inode,sizeof(struct inode), 1, fd);

	
	filsys.s_free_blocks_count--;
	filsys.s_free_blocks_group[inodetemp->di_block[0]/GROUPNUM]--;
	filsys.s_free_inodes_count--;
	
	
	fseek(fd, BLKSIZE, SEEK_SET);
	fwrite (&filsys,BLKSIZE, 1,fd);
	
	fseek(fd, BLKSIZE*2, SEEK_SET);
	fwrite (di_bitmap,BLKSIZE, 1,fd);
	
	fseek(fd, BLKSIZE*3, SEEK_SET);
	fwrite (bk_bitmap,BLKSIZE, 1,fd);

	return 1;

}


void showbitmap()
{
	int i;
	printf("inode free number %d,block free number %d\n",filsys.s_free_inodes_count,filsys.s_free_blocks_count);
	for(i=0;i<IDNODENUM;i++)
	{
		printf("%d",di_bitmap[i]);
	
	}
	for(i=0;i<DATABLKNUM;i++)
	{
		if(i%BLKGROUPNUM==0)
		{
			printf("\n");
		}
		printf("%d",bk_bitmap[i]);
	}
	printf("\n");
}

int deletefd(char strname[20])
{
	int fi,i;
	fi=search_fl(strname);
	if(fi==-1)
	{
		printf("%s not exiast\n",strname);
		return 0;
	}
	inodetemp=read_inode(fi);
	
	if((inodetemp->di_mode&IDMODE_SYSTEM)==IDMODE_SYSTEM)
	{
		printf("%s is a system file\n",strname);
		return 0;
	}else	if((inodetemp->di_mode&IDMODE_FILE)==IDMODE_FILE)
	{
		
	}
	else if((inodetemp->di_mode&IDMODE_DIR)==IDMODE_DIR)
	{
		if(inodetemp->di_number>2)
		{
			printf("Files exist in this directory!\n");
			return 0;
		}
	}
	
	if(!access())
	{	
		printf("Error!\n");
		return -1;
	}


	 char c;
	 printf("Delete %s ? <y/n>",strname);
	 fflush(stdin);
	 scanf("%c",&c);
	 if(c!='y')
	 {
		 return 0;
	 }
	 
	 i=0;
	 
	 
	 while(strcmp(dir_buf[i].d_name,strname)!=0)
	 {
		 i++;
	 }
	 for(;i<cur_inode->di_number;i++)
	 {	
		 strcpy(dir_buf[i].d_name,dir_buf[i+1].d_name);
		 dir_buf[i].d_ino=dir_buf[i+1].d_ino;
	 }
	 cur_inode->di_number--;
	 
	 fseek(fd,DATASTART+BLKSIZE*cur_inode->di_block[0], SEEK_SET);
	 fwrite(dir_buf,BLKSIZE, 1, fd);
	 
	 fseek(fd,IDNODESTART+BLKSIZE*cur_inode->di_ino, SEEK_SET);
	 fwrite(cur_inode,BLKSIZE, 1, fd);
	 
	 for(i=0;i<(int)(inodetemp->di_size/BLKSIZE+1);i++)
	 {
		bk_bitmap[inodetemp->di_block[i]]=0;
		printf("%d   %d\n",i,inodetemp->di_block[i]);
	 }

	 di_bitmap[inodetemp->di_ino]=0;
	 
	 filsys.s_free_blocks_count++;
	 filsys.s_free_blocks_group[inodetemp->di_block[0]/GROUPNUM]++;
	 filsys.s_free_inodes_count++;
	 
	 
	 fseek(fd, BLKSIZE, SEEK_SET);
	 fwrite (&filsys,BLKSIZE, 1,fd);
	 
	 fseek(fd, BLKSIZE*2, SEEK_SET);
	 fwrite (di_bitmap,BLKSIZE, 1,fd);
	 
	 fseek(fd, BLKSIZE*3, SEEK_SET);
	 fwrite (bk_bitmap,BLKSIZE, 1,fd);

	return 1;
}


int editfile(char strname[])//edit the file
{
	int fi,i,k;
	char *buf;
	fi=search_fl(strname);
	if(fi==-1)
	{
		printf("%s not exist\n",strname);
		return 0;
	}
	inodetemp=read_inode(fi);

	if(!access())
	{	
		printf("Error!\n");
		return NULL;
	}

	buf=(char *)malloc((inodetemp->di_size/BLKSIZE+1)*BLKSIZE);
	buf=ReadFile(strname);
	if(buf==NULL)
	{
		printf("Error!\n");
		return 0;
	}

	
	i =strlen(buf);
	k=0;
	printf("Input and end with\"###\":\n");
	printf("%s",buf);
	fflush(stdin);
	while(k!=3)
	{
		buf[i] = getchar();
		if(buf[i] == '#')
		{
			k++;
			if(k == 3)
				break;
		}
		else
			k=0;
		i++;
		if(i/BLKSIZE > 0)
		{
			buf = (char *)realloc(buf,BLKSIZE*(i/BLKSIZE+1));
		}
	}
	buf[i-2]='\0';

	inodetemp->di_size=strlen(buf);
	fseek(fd,IDNODESTART+BLKSIZE*inodetemp->di_ino, SEEK_SET);
	fwrite(inodetemp,BLKSIZE, 1, fd);

	fseek(fd, DATASTART+BLKSIZE*inodetemp->di_block[0], SEEK_SET);
	fwrite (buf, BLKSIZE*(inodetemp->di_size/BLKSIZE+1),1,fd);
	return 1;
}

void login()
{
	char str[20];
	int i;
	do{
		do{
		printf("User name:");
		fflush(stdin);
		scanf("%s",str);
		for(i=0;i<PWDNUM ;i++)
		{
			if(strcmp(pwd[i].username,str)==0)
			{
				break;
			}
			if(strcmp("exit",str)==0)
			{
				exit(0);
			}
		}
		if(i!=PWDNUM)
		{
			break;
		}
	
		}while(1);
	printf("Passwd:");
	fflush(stdin);
	scanf("%s",str);
	if(strcmp(pwd[i].password,str)==0)
	{
		break;
	}
	if(strcmp("exit",str)==0)
	{
		exit(0);
	}

	}while(1);

cur_user=&pwd[i];

inodetemp=read_inode(0);
if((inodetemp->di_mode&IDMODE_DIR)!=IDMODE_DIR)
{
	printf("Error, please format!\n");
}
else
{
	changeinode();
	read_dir_data(cur_inode->di_block[0]);
	
}

strcpy(cmdhead,"root");


cmd_Up("cd","etc");
cmd_Up("cd",cur_user->username);



printf("login success!!\n");
		
}


void showhelp()
{
	//printf("\tformat\t\t\t\t Format the Disk\n");
	printf("\tdir\t\t\t\t List files and directorys\n");
	printf("\tbit\t\t\t\t Show inode block\n");
	printf("\tcdir [directory name]\t\t Create a directory\n");
	printf("\tcd [directory]\t\t\t Enter a directory\n");
	printf("\tcreate [file name]\t\t Create a file\n");
	printf("\tedit [file name]\t\t Edit a file\n");
	printf("\tdel [file name]\t\t\t Delete the file\n");
	printf("\tattr [file name]\t\t Show details of the file\n");
	printf("\thelp\t\t\t\t Show the help file\n");
	printf("\tlogout\t\t\t\t Logout\n");
	printf("\tshutdown\t\t\t Exit the system\n");
}

int access()
{

	if(inodetemp->di_uid==cur_user->p_uid)
	{
		return 1;
	}
	else	if(cur_user->p_gid==0)
	{
		return 1;
	}
	else if(inodetemp->di_gid==1)
	{
		return 1;
	}
	return 0;
}


void logout()
{
	login();
}

void showaccess(char strname[20])
{
	int fi;
	fi=search_fl(strname);
	if(fi==-1)
	{
		printf("%s not Exist\n",strname);
	return ;
	}
	inodetemp=read_inode(fi);
	
	printf("---%s---attributes\n",strname);
	if((inodetemp->di_mode&IDMODE_SYSTEM)==IDMODE_SYSTEM)
	{
		printf("System file\n");
	
	}
	if((inodetemp->di_mode&IDMODE_FILE)==IDMODE_FILE)
	{
		printf("Text file\n");
	}
	if((inodetemp->di_mode&IDMODE_DIR)==IDMODE_DIR)
	{
		printf("Directory \n");
	}	
	


	printf("inode %d,block %d\n",inodetemp->di_ino,inodetemp->di_block[0]);
	if(inodetemp->di_gid==0)
	printf("Create by %s\n",pwd[inodetemp->di_uid-1].username);
	else if(inodetemp->di_gid==1)
	printf("Create by %s\n",pwd[inodetemp->di_uid-1].username);
}

