#include <gtk/gtk.h>
#include "stdio.h"
#include <gdk/gdkx.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "stdlib.h"
#include "GL/glut.h"
#include "GL/glu.h"
#include "GL/gl.h"
GtkWidget *drawingarea = NULL;
long ParentID  = -1 ;
char SDL_windowhack[32];
int canShuNum = -1;
char ** pCanshu = NULL;
GLuint  texture ;		
static GLfloat spin = 0.0 ;

int  Show3DEarth(int argc,char *argv[]);
void init();
void LoadGLTextures();	
void draw_sphere();
void reshape(int w, int h);
void display();
void spinDisplay(void);
void keyboard (unsigned char key, int x, int y);

SDL_Surface* LoadImage(const char* FileName);
int ShowImage(SDL_Surface* pSource,int x,int y,int width,int height,SDL_Surface* pDestinaton);
SDL_Surface* SDLShowImage(SDL_Surface* pScreen ,char* FileName,int iScreenWidth,int iScreenHeight,int iScreenBPP);

SDL_Surface* LoadImage(const char* FileName)
{
    SDL_Surface* pLoadImage = NULL;
    SDL_Surface* pOptiImage = NULL ;
    pLoadImage  = SDL_LoadBMP(FileName);
    if(pLoadImage != NULL)
    {
    pOptiImage = SDL_DisplayFormat(pLoadImage);
    SDL_FreeSurface(pLoadImage);     
    }
   else
     {
   printf("SDL 加载图片失败%s\n",SDL_GetError());
     }
   return pOptiImage;
}
int ShowImage(SDL_Surface* pSource,int iPosx,int iPosy,int width,int height,SDL_Surface* pDestination)
{
    SDL_Rect  ShowPos;
    ShowPos.x = iPosx;
    ShowPos.y = iPosy;
    ShowPos.w = (width == 0) ? pSource->w : width;
    ShowPos.h = (height == 0) ? pSource->h : height;
   
  if( SDL_BlitSurface(pSource,NULL,pDestination,&ShowPos)== 0)
   {
   SDL_UpdateRects(pDestination,1,&ShowPos);
   return 1;
   }
   else
   {
   printf("SDL_BlitSurface 显示错误\n");   
   return 0;
   }
}
SDL_Surface* SDLShowImage(SDL_Surface* pScreen ,char* FileName,int iScreenWidth,int iScreenHeight,int iScreenBPP)
{
    SDL_Surface*     pBackgroud = NULL;
    SDL_Surface*     pImage  = NULL; 
    if(pScreen == NULL)
    {
   pScreen = SDL_SetVideoMode(iScreenWidth,
                               iScreenHeight,
                               iScreenBPP,
                               SDL_SWSURFACE | SDL_NOFRAME);
    }  
    if(pScreen != NULL)
     {
      pBackgroud = LoadImage (FileName);
      if(pBackgroud != NULL)
        {
        if(ShowImage(pBackgroud,0,0,0,0,pScreen) != 1 )
           {
          pScreen = NULL; 
          SDL_FreeSurface( pBackgroud);
           }
         }
      }
     return pScreen ; 
}


void LoadGLTextures()								
{ 	
	SDL_Surface *surface;						
	if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) 
        {
       printf("Unable to initialize SDL: %s\n", 
              SDL_GetError());
       return ;
         } 
	if(surface = SDL_LoadBMP("earth.bmp"))
	{						
		glGenTextures(1, &texture);		
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 3,surface->w, 
		             surface->h, 0, GL_BGR, 
		             GL_UNSIGNED_BYTE, surface->pixels);
	      glTexParameteri (GL_TEXTURE_2D,
	                GL_TEXTURE_MIN_FILTER,
	                GL_LINEAR);
	      glTexParameteri(GL_TEXTURE_2D,
	                GL_TEXTURE_MAG_FILTER,
	                GL_LINEAR);
		
	}
    if(surface)				
    {
     SDL_FreeSurface(surface);			
    }

}
void init()
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 5.0 };
    GLfloat light_position[] = { 10.0, 0.0, 10.0, 0.0 };
	GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat lmodel_ambient[] = {0.1, 0.1, 0.1, 1.0};
	
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glOrtho( 0, 640, 480, 0, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D); 
}
void draw_sphere()
{
	GLUquadricObj *quadratic = gluNewQuadric(); 
	gluQuadricTexture(quadratic, GL_TRUE);	
	glBindTexture(GL_TEXTURE_2D, texture);
	gluSphere(quadratic, 1.0, 48, 48);
}
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w/(float)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -4.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glDrawBuffer(GL_BACK);
    
    glPushMatrix();
    glRotatef((GLfloat)spin, 0.0, 0.0, 1.0);
    draw_sphere();
    glPopMatrix();
    glutSwapBuffers();
}
void spinDisplay(void)
{
	spin = spin + 0.25;
	if (spin > 360.0)
		spin = spin - 360.0;
	glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
	switch (key) {
	
	case 'L':
	case 'l':
		spin = spin + 5;
		if(spin > 360) spin -= 360;
	glutPostRedisplay();
		break;
	case 'H':
	case 'h':
		spin = spin - 5;
		if(spin < 0) spin += 360;
	glutPostRedisplay();
		break;
	case 'J':
	case 'j':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(5.0, 1.0, 0.0, 0.0);
		glutPostRedisplay();
		break;
	case 'k':
	case 'K':
		glMatrixMode(GL_MODELVIEW);
		glRotatef(-5.0, 1.0, 0.0, 0.0);
		glutPostRedisplay();
		break;
	case 27:
		break;
	default:
		break;
	}
}

int   Show3DEarth(int argc,char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(640,320); 
   glutInitWindowPosition(-1,-1);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("Earth");
   
   init();
   LoadGLTextures();//加载纹理
   
   display();//显示地球
  
   glutReshapeFunc(reshape);//窗口大小改变函数
   glutDisplayFunc(display);//窗口显示时调用的函数。
   glutIdleFunc(spinDisplay);//系统空闲时毁掉函数
   glutKeyboardFunc(keyboard);//键盘消息处理函数
   glutMainLoop();//消息循环函数。
   return 0; 
}


void  show2DPicture (GtkWidget *widget, gpointer  data)
{
  
    if((SDL_Init(SDL_INIT_EVERYTHING) == -1))
    {
      printf("Could not initialize SDL:%s",SDL_GetError());
      return ;
    }  
    SDLShowImage(NULL,"./1.bmp",0,0,32);
    return ;
}  
void  show3DPicture (GtkWidget *widget, gpointer  data)
{
    Show3DEarth(canShuNum ,pCanshu);
}

gint delete_event( GtkWidget *widget, gpointer data )
{
    /* 如果你的 "delete_event" 信号处理函数返回 FALSE，GTK 会发出 "destroy" 信号。
     * 返回 TRUE，你不希望关闭窗口。
     * 当你想弹出“你确定要退出吗?”对话框时它很有用。*/
    SDL_Quit();
    g_print ("delete event occurred\n");
    /* 改 TRUE 为 FALSE 程序会关闭。*/
    return FALSE;
}

/* 另一个回调函数 */
void destroy( GtkWidget *widget, gpointer   data )
{
    gtk_main_quit ();
}

int main( int  argc,char *argv[] )
{
    /* GtkWidget 是构件的存储类型 */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *button2;
    GtkWidget * box1 ; 
    /* 这个函数在所有的 GTK 程序都要调用。参数由命令行中解析出来并且送到该程序中*/
    gtk_init(&argc, &argv);

    /* 创建一个新窗口 */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, TRUE);
    
    drawingarea = gtk_drawing_area_new ();
    gtk_widget_set_size_request (GTK_WIDGET (drawingarea), 500, 500);
   
   
    box1 = gtk_vbox_new(FALSE,0);
   
    /* 当窗口收到 "delete_event" 信号 (这个信号由窗口管理器发出，通常是“关闭”
     * 选项或是标题栏上的关闭按钮发出的)，我们让它调用在前面定义的 delete_event() 函数。
     * 传给回调函数的 data 参数值是 NULL，它会被回调函数忽略。*/
     //将delete_envent关联到delete_event函数
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(delete_event), NULL);
   // printf("2\n");
    /* 在这里我们连接 "destroy" 事件到一个信号处理函数。  
     * 对这个窗口调用 gtk_widget_destroy() 函数或在 "delete_event" 回调函数中返回 FALSE 值
     * 都会触发这个事件。*/
    g_signal_connect(G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);
    /* 设置窗口边框的宽度。*/
    gtk_container_set_border_width(GTK_CONTAINER (window), 10);
    
    /* 创建一个标签为 "2D" 的新按钮。*/
    button = gtk_button_new_with_label ("2D");
    /* 当按钮收到 "clicked" 信号时会调用 hello() 函数，并将NULL传给
     * 它作为参数。hello() 函数在前面定义了。*/
    g_signal_connect(G_OBJECT(button), "clicked",
		      G_CALLBACK(show2DPicture), NULL);
    
   button2 = gtk_button_new_with_label ("3D");
   g_signal_connect(G_OBJECT(button2), "clicked",
		      G_CALLBACK(show3DPicture), NULL);

   gtk_box_pack_start (GTK_BOX(box1), drawingarea, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX (box1), button, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX (box1), button2, FALSE, FALSE, 0);
   gtk_container_add (GTK_CONTAINER (window), box1);
  
   
    /* 最后一步是显示新创建的按钮和窗口 */
    gtk_widget_show(button);
    gtk_widget_show(button2);
    gtk_widget_show(drawingarea);
    gtk_widget_show(box1);
    gtk_widget_show(window);
    
    XSync(GDK_DISPLAY_XDISPLAY(gtk_widget_get_display(window)),
          FALSE);
    sprintf(SDL_windowhack,
            "SDL_WINDOWID=%ld",
            GDK_WINDOW_XWINDOW(drawingarea->window));
    ParentID = GDK_WINDOW_XWINDOW(drawingarea->window);
    putenv(SDL_windowhack);
    canShuNum = argc;
    pCanshu  =  argv;
    //Show3DEarth(canShuNum,pCanshu);
    /* 所有的 GTK 程序必须有一个 gtk_main() 函数。程序运行停在这里
     * 等待事件 (如键盘事件或鼠标事件) 的发生。*/
    gtk_main ();
    
    return 0;
}
