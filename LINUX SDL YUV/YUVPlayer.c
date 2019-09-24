#include <stdlib.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <SDL/SDL.h>
#include <pthread.h>

#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
char SDL_windowhack[32];
GtkWidget* MainWindow; 
void * PlayFileThread (void * data);
typedef struct VideoState{
    char szFileName[1024];
    int  iFPS;//帧率
    int  iSpeed;//速度
    int  iPlayState;//播放状态
    
    int  iFileSize; 
    FILE *PlayFileCtrl ;
    
    GtkWidget*    PlayScreen;//绘图区
    SDL_Surface*  pSDLScreen ;    //SDL surface
    SDL_Overlay* pYUVoverlay ; // YUV
    SDL_Rect      strPlayArea; //绘图区大小
    pthread_t   PlayThread;  //播放线程
    SDL_mutex*  pMutex;        //操作锁

}VideoControl,*PVideoControl;
VideoControl strPlayControl;
int InitialSDL();
void FileOpen( GtkWidget* widget, gpointer data )
{
    const gchar* strFileNameTmp = NULL;
    GtkWidget* SelectFile = NULL;
    
    if( strPlayControl.iPlayState > -1)
    {
      strPlayControl.iPlayState  = -1;
    }
    SelectFile = gtk_file_selection_new("YUV File");
    if(gtk_dialog_run(GTK_DIALOG(SelectFile)) == GTK_RESPONSE_OK)
    {
      ///获得文件名
     strFileNameTmp = gtk_file_selection_get_filename(
                                      GTK_FILE_SELECTION(SelectFile)
                                                      ); 
      if (strFileNameTmp == NULL)
        {
           gtk_widget_destroy(SelectFile);
           return;  
        }
        strcpy (strPlayControl.szFileName, strFileNameTmp);
        printf("%s\n",strPlayControl.szFileName);
        gtk_widget_destroy(SelectFile);
    
        //打开文件
        strPlayControl.PlayFileCtrl = fopen( 
                                            strPlayControl.szFileName,
                                            "rb");
        if (!strPlayControl.PlayFileCtrl)
        {
            fprintf(stderr,"open file error");
            return;
            
        }
       
        fseek(strPlayControl.PlayFileCtrl, 0, SEEK_END);
        strPlayControl.iFileSize = ftell(
                                    strPlayControl.PlayFileCtrl
                                         );
       if( strPlayControl.iFileSize < 0)
        {
            printf("Fail to get file size");;
            return;
        }

		fseek(strPlayControl.PlayFileCtrl, 0, SEEK_SET);
    }
    
    return;
}
void PlayFile( GtkWidget* widget, gpointer data )
{ 
  int nRet;
  if(strPlayControl.PlayFileCtrl == NULL)
  {
     return;
  }
  while( strPlayControl.iPlayState != -2)
       {
       if(strPlayControl.iPlayState  == 1)
       {
        return;
       }
       if(strPlayControl.iPlayState == 0)
       {
        strPlayControl.iPlayState = 1;
        return;
       }
       }
   nRet = pthread_create( &strPlayControl.PlayThread, 
                          NULL, 
                          PlayFileThread, 
                          (void*)(&strPlayControl)
                         );
   if(nRet != 0)
   {
    fprintf(stderr, " Create play thread fail \n");
    return;
   }
   strPlayControl.iPlayState = 1;
}
void * PlayFileThread (void * data)
{
  VideoControl *PlayControl = (VideoControl *)data;
  //文件操作指针
  FILE *FileCtrl = PlayControl->PlayFileCtrl;
  //首数据
  fseek(FileCtrl,0,SEEK_SET); 
  
  //int FileSize = PlayControl->iFileSize;
  //显示区
  SDL_Overlay* YUVoverlay = NULL;
  //绘图区
  GtkWidget* ViewScreen =PlayControl->PlayScreen ;
  //帧率
  int iSpeed;
  //分辨率
  int Fpd =  PlayControl->iFPS;
   //帧大小
  int FrameSize ;
  //绘图区域
  SDL_Rect ViewArea;
  //数据快
  char FrameData[608256];
  int DataSize;
  
  //绘图区域
  ViewArea.x = 0;
  ViewArea.y = 0;
  //正的大小
  switch(Fpd)
  {
  case 0: //QCIF
  FrameSize =176*144*3/2;
  ViewArea.w =176;
  ViewArea.h = 144;
  break;
  
  case 1: //CIF
  FrameSize = 352*288*3/2;
  ViewArea.w = 352;
  ViewArea.h = 288;
  break;
  
  case 2: //4CIF
  FrameSize =704*576*3/2;
  ViewArea.w = 704;
  ViewArea.h = 576;
  break;
  }
 // printf("%d\n",FrameSize);
  YUVoverlay = SDL_CreateYUVOverlay( ViewArea.w,
                                     ViewArea.h,
                                     SDL_IYUV_OVERLAY,
                                     strPlayControl.pSDLScreen
                                    );
  while(1)
  {
  //播放状态
  if(PlayControl->iPlayState == 1)
  {
  //显示区的大小改变，重新setVideoMode
   if( ViewArea.w != ViewScreen->allocation.width 
       || ViewArea.h != ViewScreen->allocation.height)
   {
       ViewArea.w = ViewScreen->allocation.width;
       ViewArea.h = ViewScreen->allocation.height;
       PlayControl->pSDLScreen = SDL_SetVideoMode(
                                               ViewArea.w,
                                               ViewArea.h,
                                               0,
                                               SDL_SWSURFACE|SDL_NOFRAME);
   }
   
   //播放速率  
  iSpeed = (int)(1000/PlayControl->iSpeed);
  
   //读取帧
   DataSize = fread(FrameData,1, FrameSize, FileCtrl);
   //printf("%d\n",DataSize);
   if(DataSize == FrameSize)
    {
    //显示图像
    SDL_LockSurface(PlayControl->pSDLScreen);
    SDL_LockYUVOverlay(YUVoverlay);
    memcpy(*(YUVoverlay->pixels),FrameData,FrameSize);
    SDL_UnlockYUVOverlay(YUVoverlay);
    SDL_UnlockSurface(PlayControl->pSDLScreen);
    SDL_DisplayYUVOverlay(YUVoverlay, &ViewArea);
    
    //控制帧率
    SDL_Delay(iSpeed);
    }
   else
   {
   if(feof(FileCtrl))
   {
   printf("文件结束\n");
   PlayControl->iPlayState = 0;
   fseek(FileCtrl,0,0);
   }
   else
   {
   printf("文件读取错误\n");
   PlayControl->iPlayState = -2;
   break;
   }
   }  
  }  
 
  //非播放状态
  else
  {
  //停止状态,退出
  if(PlayControl->iPlayState == -1)
  {
  PlayControl->iPlayState = -2;
  break;
  }
  //暂停状态，继续执行，等待他停止
  }
 }
 fclose(FileCtrl);
 strPlayControl.PlayFileCtrl = NULL;
 SDL_FreeYUVOverlay(YUVoverlay);
 return NULL;
}

void Pause( GtkWidget* widget, gpointer data )
 {
 if(strPlayControl.iPlayState == 1)
 {
   strPlayControl.iPlayState = 0;
 }
 }
void StopPlay( GtkWidget* widget, gpointer data )
 {
   strPlayControl.iPlayState = -1;
 }
 void ReplayFile( GtkWidget* widget, gpointer data )
 {
   if(strPlayControl.iPlayState >= 0)
   {
    strPlayControl.iPlayState = 1;
    fseek(strPlayControl.PlayFileCtrl,0,SEEK_SET);
    
   }
 }
 void PlaySlow( GtkWidget* widget, gpointer data )
 {
  if(strPlayControl.iSpeed > 5)
  { 
   strPlayControl.iSpeed = (strPlayControl.iSpeed-5);
  }
 }
 void PlayFast( GtkWidget* widget, gpointer data )
 {
  if(strPlayControl.iSpeed < 50)
  { 
   strPlayControl.iSpeed = (strPlayControl.iSpeed+5);
  }
 }
 void FullScreen( GtkWidget* widget, gpointer data )
 {
   gtk_window_fullscreen( GTK_WINDOW( MainWindow));
 }
 void SetPixel( GtkWidget* widget, gpointer data )
 {
   gchar* string; 
   string = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
     if (!strcasecmp(string, "QCIF"))
    {
       strPlayControl.iFPS = 0;
    }
    if (!strcasecmp(string, "CIF"))
    {
        strPlayControl.iFPS = 1;
    }
    if (!strcasecmp(string, "4CIF"))
    {
        strPlayControl.iFPS = 2;
    }
   return ;
 }

gint delete_event( GtkWidget *widget, gpointer data )
{
    /* 如果你的 "delete_event" 信号处理函数返回 FALSE，GTK 会发出 "destroy" 信号。
     * 返回 TRUE，你不希望关闭窗口。
     * 当你想弹出“你确定要退出吗?”对话框时它很有用。*/
    if(strPlayControl.iPlayState != -2)
    {
      strPlayControl.iPlayState  = -1 ;
    }
    while( strPlayControl.iPlayState != -2)
    {
    SDL_Delay(10);
    }
    SDL_Quit();
    g_print ("delete event occurred\n");
    /* 改 TRUE 为 FALSE 程序会关闭。*/
    gtk_main_quit ();
    return TRUE;
}

 
void InitialDrawArea(GtkWidget* PlayVideoArea)
{
    gtk_widget_set_size_request(PlayVideoArea, 704,576);
    gtk_widget_add_events(GTK_WIDGET(PlayVideoArea), GDK_BUTTON_PRESS_MASK);
    //g_signal_connect(G_OBJECT(PlayVideoArea),
               //      "button-press-event",
                //     NULL,//G_CALLBACK(ScreenPressed),
                //     NULL);
    printf("InitialDrawArea\n");
}
 void destroy( GtkWidget *widget, gpointer   data )
{
    gtk_main_quit ();
}
void InitialMainWindow(GtkWidget* MainWindow)
{
     
    gtk_window_set_title(GTK_WINDOW(MainWindow), "YUVPlayer");
    gtk_window_set_position(GTK_WINDOW(MainWindow), GTK_WIN_POS_CENTER);
   g_signal_connect(G_OBJECT (MainWindow), 
                              "destroy",
		                      G_CALLBACK (destroy), 
		                      NULL); 
	g_signal_connect(G_OBJECT(MainWindow),
	                 "delete_event",
	                 G_CALLBACK(delete_event), 
	                 NULL);
    printf("InitialMainWindow\n");
}

void  InitialToolBar(GtkWidget*  gToolBar)
{   
    GtkWidget* ButtonImage;  
    GtkWidget* Button ;
    //工具条类型
    gtk_toolbar_set_orientation (GTK_TOOLBAR (gToolBar), GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style(GTK_TOOLBAR(gToolBar), GTK_TOOLBAR_BOTH);
    gtk_container_set_border_width(GTK_CONTAINER(gToolBar), 2);
    gtk_toolbar_set_icon_size(GTK_TOOLBAR (gToolBar), GTK_ICON_SIZE_SMALL_TOOLBAR);
    
    
    //添加按钮
    //打开文件
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    ButtonImage = gtk_image_new_from_file("./icon/open.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                             NULL,
                                             "Open",
                                             NULL,
                                             ButtonImage,
                                             G_CALLBACK(FileOpen),
                                             NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //播放
    ButtonImage = gtk_image_new_from_file("./icon/play.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                         NULL,
                                         "Play",
                                         NULL,
                                         ButtonImage,
                                         G_CALLBACK(PlayFile),
                                         NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //暂停
    ButtonImage = gtk_image_new_from_file("./icon/pause.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                          NULL,
                                          "Pause",
                                          NULL,
                                          ButtonImage,
                                         G_CALLBACK(Pause),
                                          NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //停止
    ButtonImage = gtk_image_new_from_file("./icon/stop.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                         NULL,
                                         "Stop",
                                         NULL,
                                         ButtonImage,
                                         G_CALLBACK(StopPlay),
                                         NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //重新播放
    ButtonImage = gtk_image_new_from_file("./icon/replay.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                           NULL,
                                           "Replay",
                                           NULL,
                                           ButtonImage,
                                           G_CALLBACK(ReplayFile),
                                           NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //慢放
    ButtonImage = gtk_image_new_from_file("./icon/backward.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                         NULL,
                                         "SlowDown",
                                         NULL,
                                         ButtonImage,
                                         G_CALLBACK(PlaySlow),
                                         NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //快放
    ButtonImage = gtk_image_new_from_file("./icon/forward.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                         NULL,
                                         "Acelerate",
                                         NULL,
                                         ButtonImage,
                                         G_CALLBACK(PlayFast),
                                         NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //全屏
    ButtonImage = gtk_image_new_from_file("./icon/fullscreen.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                               NULL,
                                               "Fullscreen",
                                               NULL,
                                               ButtonImage,
                                               G_CALLBACK(FullScreen),
                                              NULL);
    gtk_toolbar_append_space (GTK_TOOLBAR (gToolBar));
    //退出
    ButtonImage = gtk_image_new_from_file("./icon/quit.png");
    Button = gtk_toolbar_append_item(GTK_TOOLBAR(gToolBar),
                                         NULL,
                                         "Quit",
                                         NULL,
                                         ButtonImage,
                                         G_CALLBACK(delete_event),
                                         NULL);
                              printf("InitialToolBar\n");
                                         
}
void  InitialPixelCom(GtkWidget*  gPixelCombo)
{
    gtk_combo_box_append_text(GTK_COMBO_BOX(gPixelCombo), "QCIF");
    gtk_combo_box_append_text(GTK_COMBO_BOX(gPixelCombo), "CIF");
    gtk_combo_box_append_text(GTK_COMBO_BOX(gPixelCombo), "4CIF");
    gtk_combo_box_set_active(GTK_COMBO_BOX(gPixelCombo), 0);
    g_signal_connect(G_OBJECT(gPixelCombo),
                     "changed", 
                      G_CALLBACK(SetPixel),
                      NULL);
                     printf("InitialPixelCom\n");
}
int InitialSDL()
{
   if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf( "SDL_Init() Error!\n");
        return 0;
    }
    return 1;
}

int main( int   argc, char *argv[] )

{
   
    //------------------------------------------------------------
    gtk_init (&argc, &argv);
             
    GtkWidget* VideoScreen;         
    GtkWidget* PixelCombo;
    GtkWidget* ComboBoxTable;  
    GtkWidget* ToolBar;
    GtkWidget* text;                            
    GtkWidget* vBox;               
    GtkWidget* StatusBar;
    //------------------------------------------------------------
    MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    InitialMainWindow(MainWindow);
    //绘图区
    VideoScreen = gtk_drawing_area_new();
    InitialDrawArea(VideoScreen);

	//状态栏
    StatusBar = gtk_statusbar_new();

    //分辨率
    PixelCombo = gtk_combo_box_new_text();
    text = gtk_label_new("Pixel Set:");
    ComboBoxTable = gtk_table_new(1, 30, TRUE);
    gtk_table_attach_defaults(GTK_TABLE(ComboBoxTable), text, 0, 4, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(ComboBoxTable), PixelCombo, 4, 7, 0, 1);
    InitialPixelCom(PixelCombo);
    //工具条
    ToolBar = gtk_toolbar_new();
    InitialToolBar(ToolBar);
   
    //------------------------------------------------------------
    vBox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vBox), VideoScreen, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vBox), StatusBar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vBox),ComboBoxTable, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vBox),ToolBar, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(MainWindow), vBox);
   
    //
    gtk_widget_show_all(MainWindow);
	//-----------------------------------------------------------
   XSync(GDK_DISPLAY_XDISPLAY(gtk_widget_get_display(MainWindow)),
          FALSE);
    sprintf(SDL_windowhack, 
            "SDL_WINDOWID=%ld",
            GDK_WINDOW_XWINDOW(VideoScreen->window)
            );
    putenv(SDL_windowhack);
    //初始化SDL
     if(InitialSDL())
    {
    strPlayControl.iPlayState = -2; 
    strPlayControl.iSpeed = 25;
    strPlayControl.iFPS = 0;
    strPlayControl.PlayScreen = VideoScreen;
    printf("SDL%d  %d\n",strPlayControl.PlayScreen->allocation.width,
                         strPlayControl.PlayScreen->allocation.height);
    strPlayControl.pSDLScreen= SDL_SetVideoMode(
                                         strPlayControl.PlayScreen->allocation.width,
                                         strPlayControl.PlayScreen->allocation.height,
                                         0,
                                         SDL_SWSURFACE|SDL_NOFRAME);
   if(strPlayControl.pSDLScreen == NULL)
   {
    printf("SDL_SetVideoMode Fail\n");
    return 0;
   }
    gtk_main();
    }
    return 0;

}



 



