#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h> 

typedef struct Node {
    char  szFileName[1024];
    struct stat strFileInfo; 
    int       is_to_string;
    Node *next;
} FileInfoHead,FileInfoNode;

DIR * openfile(char  *pFilename);
int initial_ls_type(int ac,char *av[]) ;
void do_ls(char * pFilename,int iType);
void file_name(char desfilename[],char srcfilename[] );
int    InsertNode( FileInfoHead * Head,FileInfoNode * Node,int iTpye);
void size_to_string(long iFize,int ls_iType);
void  time_to_string(time_t time);
void *uid_to_name(uid_t uid);
void *gid_to_name(gid_t gid );
void  mode_to_string(int mode);
int  stat_to_string(struct stat strfilestat,int ls_iType);
FileInfoNode * MergeList(FileInfoNode *Lista,FileInfoNode * Listb );
FileInfoNode *sort_filename( DIR * pDir,char * pDirname,int iTpye,FileInfoNode *pFirstAvaNode);
char * FindLateName(char * pFilename);
FileInfoNode * show_file_Attribute(FileInfoNode* pFileQeue,FileInfoNode *pAvaQueue ,int ls_iTpye);
void  FreeNode(FileInfoNode * pHead);

int main (int ac,char *av[])
{             
     char szfilename[1025];
     szfilename[1024] = '\0'; 
     int i;
     //��ʼ��ls����
     int ls_type =  initial_ls_type(ac,av) ;
     if(ls_type == 0)
     {
        if(ac == 1)
        {
           do_ls(".",ls_type);
        }
        else
        {
           for(i = 1;i < ac ;i++)
           {
              //��ȫls���ļ���
		file_name(szfilename,av[i]);
              //�����ļ���������ls
		do_ls(szfilename,ls_type);
	    }      
	   }
	 }
     else
     {
         if(ac == 2)
         {
            do_ls(".",ls_type);
	  }
         else
        {
           for(i = 2;i < ac ;i++)
           {
               file_name(szfilename,av[i]);
               do_ls(szfilename,ls_type);
	    }
	 }
     }
}



int initial_ls_type(int acount,char *avs[])
{
    int ls_type = 0;
    int i; 
    if(acount == 1)
    {
        return 0;         
    }
     else
     {
         if(avs[1][0] == '-')
         {
             //printf("-\n");
             for( i = 1;i < strlen(avs[1]);i++)
             {
                   switch(avs[1][i])
                   {
                    case 'l':
                            ls_type |=  1; 
				break;						
                    case 'R':
                            ls_type |=  2;
				break;	
                    case 'a':
                            ls_type |=  4;
				  break;
		      case 'i':	   
                            ls_type |=  8;
				  break;
                    case '1':
                            ls_type |=  16;
				break;
                     case 'h':
                            ls_type |=  32;
				break;
		      default:
				break;
		     }  
	       }  
          }
	 }
	 return ls_type;
}

//����".","..",".g",����ĸ��ͷ���ļ���ȫ
void file_name(char desfilename[],char srcfilename[] )
{
       switch(srcfilename[0])
              {
                   case '.':
                                  if(strlen(srcfilename) > 1)
                                  {
                                           if(srcfilename[1] != '/' && srcfilename[1] != '.'  )
					                      {
					                                  sprintf(desfilename,"./%s",srcfilename);
                                                      break;
					                      }  
			                       }
					               sprintf(desfilename,"%s",srcfilename);			  
					               break;
                    case '/':
                                   sprintf(desfilename,"%s",srcfilename);
				                    break;
		        default:
                                  sprintf(desfilename,"./%s",srcfilename);
				                  break;
       	       }

}

void do_ls(char * pFilename,int ls_type)
{
     //
     struct stat fileinfo;
	 DIR *dir_ptr;
	 FileInfoHead * fileinfohead = new  FileInfoNode;
     FileInfoNode* tmpSortHead = NULL;
	 FileInfoNode* tmpTail = NULL;  
	 FileInfoNode* pAvaNode = NULL;  
	 FileInfoNode* tmppre = NULL; 
	 FileInfoNode* tmpFirstDir = NULL; 
	 //�ж�ls �ļ������ͣ���Ŀ¼��ʾ���ԣ�Ŀ¼�ݹ��ȡĿ¼����
	 if(stat(pFilename,&fileinfo) ==-1)
	 {     
          perror(pFilename);
          return;
	 }
	 if(!S_ISDIR(fileinfo.st_mode))
	 {//��Ŀ¼�ļ�
           stat_to_string(fileinfo, ls_type);
           printf("%s\n",pFilename);
           return;
	 }
       else
       {
      //Ŀ¼�ݹ��ȡĿ¼����
            fileinfohead->next = new FileInfoNode ;
	     sprintf(fileinfohead->next->szFileName,"%s",pFilename);       
	     while (fileinfohead->next !=NULL )
            {
            //ȡ���ļ���Ϣ����Ŀ¼
            tmpFirstDir = fileinfohead->next;       
            tmpFirstDir->is_to_string  = 1 ;
            fileinfohead->next =fileinfohead->next->next ;
            tmpFirstDir->next = NULL;
            
            //��Ŀ¼
            dir_ptr = openfile(tmpFirstDir->szFileName);
            
            if(dir_ptr == NULL)
            {
                 pAvaNode =  MergeList(tmpFirstDir,pAvaNode);
                 continue;
            }
            //����Ŀ¼�е�����
            tmpSortHead = sort_filename(dir_ptr,tmpFirstDir->szFileName,0,pAvaNode);                              
          
            //��Ŀ¼��continue
	       if(tmpSortHead == NULL)
            {
                pAvaNode = NULL;
                continue;
	      }
	      else
	      {
	         if(tmpSortHead->is_to_string ==  1)
	         {
	                 pAvaNode = MergeList(tmpFirstDir,tmpSortHead);
	                 continue;
	         }
	      }
	       //�ǿ�Ŀ¼
	        tmpTail = tmpSortHead;
	        tmppre =  tmpSortHead;
           //ȡ��Ŀ¼���ݣ�������ýڵ㡣
            while(tmpTail != NULL)
            {
                   if(tmpTail->is_to_string == 0)
                   {  
                        tmppre  =  tmpTail;
			   tmpTail = tmpTail->next ; 
                   }
		     else
		     {
                         break;
		      }		   
	       }
           
            pAvaNode = tmpTail;
            tmppre->next = NULL;
           
           //ȡ��Ŀ¼�ڵ�
	     tmppre->next =  fileinfohead->next;
            fileinfohead->next = tmpSortHead;
            tmpTail = fileinfohead;
	     //��ӡĿ¼����
	     printf("%s\n\n",tmpFirstDir->szFileName);
	     pAvaNode =  MergeList(tmpFirstDir,pAvaNode); 
	     //��ʾĿ¼����
	     pAvaNode = show_file_Attribute(fileinfohead,pAvaNode ,ls_type);	   
	   }    
     }
     FreeNode(  fileinfohead);
     FreeNode(  pAvaNode); 

}
 FileInfoNode * show_file_Attribute(FileInfoNode* pFileQeue,FileInfoNode *pAvaQueue ,int ls_iTpye)
 {
 
         FileInfoNode * tmpTail = pFileQeue;
         FileInfoNode * tmppre = NULL;
         //����ļ������ļ�
	     while(tmpTail ->next != NULL)
         {    
               if( tmpTail->next->is_to_string != 1)
               {
			 //�Ƿ�����ļ�����
			  if(ls_iTpye & 1 )
			   {
			    //�Ƿ���chu '.','..'
			         if(ls_iTpye & 4 || 
			             ( strcmp(".",FindLateName(tmpTail->next->szFileName)) &&
			               strcmp("..",FindLateName(tmpTail->next->szFileName)))
			              )
			          {
			                   stat_to_string(tmpTail->next->strFileInfo,ls_iTpye);  
                        }
                      }
                        //�Ƿ���chu '.','..'
                     if(ls_iTpye & 4 || 
			     ( strcmp(".",FindLateName(tmpTail->next->szFileName)) &&
			        strcmp("..",FindLateName(tmpTail->next->szFileName)))
			      )
			   {
                            //��ӡ�ļ���
				printf("%s",FindLateName(tmpTail->next->szFileName));                        
                               //�Ƿ�ÿ�����һ��
			       if(ls_iTpye & 16 || ls_iTpye & 1)
			      {
			          printf("\n");
                           } 
                           else
                           {
                               printf("  ");
                           }    
			   }
                                         
                tmpTail->next->is_to_string = 1;                   
                //�Ƿ�ݹ�����ļ�
                if(!(ls_iTpye & 2)||
				!S_ISDIR(tmpTail->next->strFileInfo.st_mode) || 
				!strcmp(".",FindLateName(tmpTail->next->szFileName)) || 
				!strcmp("..",FindLateName(tmpTail->next->szFileName)) )  
                {     //ȡ��'.',".."��ȥ����Ŀ¼�ļ�
                              tmppre = tmpTail->next;
                              tmpTail->next = tmpTail->next->next;                          
                              tmppre->next  = NULL;
                              pAvaQueue = MergeList(tmppre,pAvaQueue);
			    }
			   else
               {
                          //����Ŀ¼�ļ�
                          tmpTail = tmpTail->next;
			    }		         
			         
		 }
		else
		 {  //�˴�Ŀ¼������
		  break;	         
		  }          
	}	

	 if( !(ls_iTpye & 16 ) ||!(ls_iTpye & 1))
       {
             printf("\n");
        } 
    return  pAvaQueue;
 }
 //�ҵ�"/"���ļ���
char *FindLateName(char * pFilename)
 {
     int pos = strlen(pFilename);
     while( pos > 0)
     {
        pos --;
        if(pFilename[pos] == '/')
        {
            pos ++;
            break;
        } 
     }
     return ( pFilename +  pos );
 }
 //�ϲ�����
FileInfoNode * MergeList(FileInfoNode *Lista,FileInfoNode * Listb )
 {
     FileInfoNode * pre = Lista;
     FileInfoNode * tail = Lista;
     if(Lista == NULL )
     {
        return Listb;
     }
     if(Listb == NULL )
     {
        return Lista;
     }
     while(tail != NULL)
     {
         pre = tail;
         tail = tail->next;
     }
     pre->next = Listb;
     return Lista;
 }
//��Ŀ¼
DIR * openfile(char  *pFilename)
{
     DIR *dir_ptr;
     if((dir_ptr = opendir(pFilename)) == NULL)
     {
           perror(pFilename);
	    return NULL;
      }
     return dir_ptr;
}
//����Ŀ¼�е�����
 FileInfoNode *sort_filename( DIR * pDir,char * pDirname,int iTpye,FileInfoNode *pFirstAvaNode)
{
     FileInfoNode *tmpHead = NULL ;
     FileInfoNode *tmpNode = NULL ; 
     FileInfoNode *tmp = NULL ;
     struct dirent *direntp; 
     if(pDir == NULL)
     {
         return pFirstAvaNode;
     }
    
     tmpHead = new  FileInfoNode ;
     tmpHead->next =NULL;
     tmpNode= pFirstAvaNode;
    while((direntp = readdir(pDir))!= NULL)
    {
      //�ӿ��ö�����ȡ�ڵ�
       if(tmpNode != NULL)
       {
		       sprintf(tmpNode->szFileName,"%s/%s",pDirname,direntp->d_name);
	           if(stat(tmpNode->szFileName,&(tmpNode->strFileInfo)) ==-1)
	          {
              perror(tmpNode->szFileName);
              continue;          
	          }
             tmpNode ->is_to_string = 0 ; 
	         pFirstAvaNode = tmpNode->next;
	         tmpNode->next = NULL;
            //����ڵ�
	        InsertNode( tmpHead,tmpNode,iTpye); 
	        tmpNode = pFirstAvaNode;
      }
      else
	 {
        //���ýڵ�����
              tmpNode = new  FileInfoNode ;
	          tmpNode->next = NULL;
              sprintf(tmpNode->szFileName,"%s/%s",pDirname,direntp->d_name); 
              if(stat(tmpNode->szFileName,&(tmpNode->strFileInfo)) ==-1)
	          {
                       perror(tmpNode->szFileName);
                       tmpNode = NULL;
                       continue;
	          }
              tmpNode->is_to_string = 0;   
	         InsertNode( tmpHead,tmpNode,iTpye);
	         tmpNode = NULL;
	}
  }  
  //�ϲ�Ŀ¼���ݶ��к�ʣ����ýڵ�
   pFirstAvaNode =  MergeList(tmpHead->next,pFirstAvaNode);
   delete tmpHead;  
   return pFirstAvaNode;     
}
//���ڵ㰴���������Ͳ������
int  InsertNode( FileInfoHead * Head,FileInfoNode * Node,int iTpye)
{
       FileInfoHead  * tmp = NULL; 
	if(Head == NULL)
      {
            return 0;
      }
      if(Head->next ==NULL)
      {
              Head->next = Node;
	       return 1;
      }
      else
      {
            tmp = Head;
        
            while( tmp->next != NULL)
            {   //����ĸ����
                 if((strcmp(Node->szFileName,tmp->next->szFileName)) < 0)
                 {
                          break;
	            }
                   else
                  {
                  
                                tmp= tmp->next;
		          }  
		     }       
                Node->next = tmp->next;
	            tmp->next = Node;
                return 1;
	  }
}

int  stat_to_string(struct stat strfilestat,int ls_type)
{
       if((ls_type&8)) printf("%-8d  ",strfilestat.st_ino);
    
       mode_to_string(strfilestat.st_mode);
       printf("  ");
       printf("%-4d",int(strfilestat.st_nlink));
       printf("  ");
       uid_to_name(strfilestat.st_uid);
	   printf("  ");
	   gid_to_name(strfilestat.st_gid);
	   printf("  ");
	   size_to_string(strfilestat.st_size,ls_type);
	   printf("  ");
	   time_to_string(strfilestat.st_mtime);
	   printf("  ");
 }

void  mode_to_string(int mode)
{     
       char cModeString[11]; 
       strcpy(cModeString,"----------");
	   if(S_ISDIR(mode)) cModeString[0] = 'd';
	   if(S_ISCHR(mode)) cModeString[0] = 'c';
	   if(S_ISBLK(mode)) cModeString[0] = 'b';
	   if(S_ISFIFO(mode)) cModeString[0] = 'f';
	   if(S_ISSOCK(mode)) cModeString[0] = 's';
       if(S_ISLNK(mode)) cModeString[0] = 'l';
       if(mode&S_IRUSR) cModeString[1] = 'r';
       if(mode&S_IWUSR) cModeString[2] = 'w';
       if(mode&S_IXUSR) cModeString[3] = 'x';
       if(mode&S_IRGRP) cModeString[4] = 'r';
       if(mode&S_IWGRP) cModeString[5] = 'w';
       if(mode&S_IXGRP) cModeString[6] = 'x';
	   if(mode&S_IROTH) cModeString[7] = 'r';
       if(mode&S_IWOTH) cModeString[8] = 'w';
       if(mode&S_IXOTH) cModeString[9] = 'x';
	   printf("%s",cModeString);
}

void *uid_to_name(uid_t uid)
{
       
       struct passwd* pw_ptr = NULL;
       pw_ptr = getpwuid(uid) ;
	   if( pw_ptr != NULL)
       {
            printf("%-7s",pw_ptr->pw_name);
	   }
      else
      {
            printf("%-7d",uid);
	  }

}

void *gid_to_name(gid_t gid )
{
        struct  group* grp_ptr = NULL;
        grp_ptr = getgrgid(gid) ;
    	if( grp_ptr != NULL)
       {
            printf("%-7s",  grp_ptr->gr_name);
	   }
      else
      {
            printf("%-7d",gid);;
	  }
}
void  time_to_string(time_t time)
{
    struct tm tmp_tm;
    localtime_r(&time, &tmp_tm);
    printf("%02d-%02d  %02d:%02d:%02d", 
		      tmp_tm.tm_mon,
	          tmp_tm.tm_mday,
              tmp_tm.tm_hour,
              tmp_tm.tm_min,
              tmp_tm.tm_sec
            );
}     
void size_to_string(long iFize,int ls_iType)
{
         if(ls_iType & 32)
         {
              printf("%7fK",float(iFize/1000));      
	      }
	     else
	    {
             printf("%10u",iFize);	  
	    }
	 
}
void  FreeNode(FileInfoNode * pHead);
{
    FileInfoNode *  tmp = NULL;
    while(pHead != NULL)
    {
       tmp = pHead->next;
	   delete pHead;
	   pHead = tmp;
    }
}