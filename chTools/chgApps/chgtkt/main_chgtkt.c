#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>			
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sched.h>
#include <sys/wait.h>
//
#include <CHANTILLY_BP.h>
#include "/home/pi/code_004/common/CHAN/daemon_defs.h"
#include "/home/pi/code_004/common/CHAN/daemon_utils.h"
//#include "/home/pi/code_004/UTILS/debug_time.h"
//
#include <gtk/gtk.h>
#include <gdk/gdk.h>
//
//#include "/home/pi/code_004/common/CHAN/coredef_0xBB_139_020.h"
//
#define _BUS_BOARD      0x07
#define _IO24		    0x01
#define _THIS_APP_ID 	0xAA
#define _THIS_VERS 		0x01

typedef struct
{
	GtkBuilder *builder;
    gpointer user_data; 
}SGlobalData;

struct sched_param param;

uint8_t *str_sub (uint8_t *s, unsigned int start, unsigned int end)
{
   uint8_t *new_s = NULL;
   if (s != NULL && start < end)
   {
      new_s = malloc (sizeof (*new_s) * (end - start + 2));
      if (new_s != NULL)
      {
         int i;
         for (i = start; i <= end; i++)
         {
            new_s[i-start] = s[i];
         }
         new_s[i-start] = '\0';
      }
      else
      {
         fprintf (stderr, "Memoire insuffisante\n");
         exit (EXIT_FAILURE);
      }
   }
   return new_s;
}

pid_t create_process(void)
{
    pid_t pid;
    do {
		pid=fork();
    } while ((pid==-1) && (errno==EAGAIN));
    return pid;
}

void son_process(char *arg[])
{
	if (execv("/usr/bin/sudo",arg)==-1) {
		perror("execv");
		CHAN_close();
		exit(EXIT_FAILURE);
    }
}

void setCheckConf_cb2(GtkWidget * p_wid,gpointer p_data) {                                          // checkbutton "warn me ..." callback in popup window at startup
	SGlobalData *data =(SGlobalData*) p_data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) *(gint*)(data->user_data)=1;
	else *(gint*)(data->user_data+4)=0;
}
	
void setCheckConf_cb(GtkWidget * p_wid,gpointer p_data) {											// checkbutton "warn me ...." callback in preferences popup window
	SGlobalData *data =(SGlobalData*) p_data;
	FILE* fd = NULL;
	uint8_t* ptr=NULL; 
	int8_t str[100]="";
	fd = fopen("./chgtkt.conf", "rb+");
	if (fd != NULL) {
		do {
			fgets((char*)str, 100, fd);
		} while(strstr((const char*)str,"check_conf")==NULL);
		fseek(fd,-strlen((const char*)str), SEEK_CUR);
		ptr=(uint8_t*)str;
		while (*ptr++!='=');
		while (*ptr==' ') ptr++;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
			*ptr='1';
			*(gint*)(data->user_data)=1;
		}
		else {
			*ptr='0';
			*(gint*)(data->user_data)=0;
		}
		fputs((const char*)str,fd);
		fclose(fd);	 
	}
}

void setSetupError_cb2(GtkWidget * p_wid,gpointer p_data) {																	// starup popup
	SGlobalData *data =(SGlobalData*) p_data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) *(gint*)(data->user_data+4)=0;
	else *(gint*)(data->user_data+4)=-1;
}

void setSetupError_cb(GtkWidget * p_wid,gpointer p_data) {																	//preferences popup
	SGlobalData *data =(SGlobalData*) p_data;
	FILE* fd = NULL;
	uint8_t* ptr=NULL; 
	int8_t str[255]="";
	int8_t str2[127]="";
	uint8_t i=0;
	fflush(stdout);
	fd = fopen("./chgtkt.conf", "rb+");
	if (fd != NULL) {
		do {
			fgets((char*)str, 255, fd);
		} while(strstr((const char*)str,"setup_error")==NULL);
		fseek(fd,-strlen((const char*)str), SEEK_CUR);
		ptr=(uint8_t*)str;
		while (*ptr++!='=');
		while (*ptr==' ') ptr++;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
			*ptr++='0';
			*ptr++=' ';
			ptr++;
			strcpy((char*)str2,(const char*)ptr);
			ptr--;
			while(*(str2+i)) *ptr++=str2[i++];
			*(gint*)(data->user_data+4)=0;
		}
		else {
			*ptr++='-';
			*ptr++='1';
			strcpy((char*)str2,(const char*)ptr);
			*ptr++=' ';
			while(*(str2+i)) *ptr++=str2[i++];
			*(gint*)(data->user_data+4)=-1;
		}
		fputs((const char*)str,fd);
		fclose(fd);	 
	}
}

void imagemenuitem12_activate_cb(GtkWidget * p_wid,gpointer p_data) {									//_Preferences
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWindow *parent=NULL;
	GtkWidget *dialog=NULL,*label=NULL,*checkButton=NULL,*contentArea=NULL,*radio1=NULL,*radio2=NULL;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gchar* message="settings \n";
	//int32_t result=0;
	//FILE* fd = NULL;
	//uint8_t* ptr=NULL; 
	//int8_t str[100]="";
	//int8_t str2[100]="";
	//uint8_t i=0;
		
	parent=GTK_WINDOW(gtk_builder_get_object(data->builder,"window1"));
	dialog = gtk_dialog_new_with_buttons ("chrtcf Preferences",parent,flags,("_CLOSE"),GTK_RESPONSE_NONE,NULL);
    contentArea = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	label = gtk_label_new (message);
	radio1 = gtk_radio_button_new_with_label(NULL,"Should use direct access mode.");
	radio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (radio1),"Should use daemonized access mode.");
	checkButton= gtk_check_button_new_with_label("Warn me at startup if current acces mode differs from predefined access mode in conf file.");
	printf("checkConf : %d\n",*(gint*)(data->user_data)); 
	printf("setupErrorC : %d\n",*(gint*)(data->user_data+4)); 
	if (*(gint*)(data->user_data)) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton),TRUE);
	if (!*(gint*)(data->user_data+4)) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio1),TRUE);
	else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio2),TRUE);
	
	
	gtk_container_add(GTK_CONTAINER (contentArea),label);
	
	gtk_container_add(GTK_CONTAINER(contentArea),radio1);
	gtk_container_add(GTK_CONTAINER(contentArea),radio2);
	gtk_container_add(GTK_CONTAINER(contentArea),checkButton);
	g_signal_connect(checkButton,"toggled",G_CALLBACK(setCheckConf_cb),data);
	g_signal_connect(radio1,"toggled",G_CALLBACK(setSetupError_cb),data);
	g_signal_connect_swapped(dialog,"response",G_CALLBACK(gtk_widget_destroy),dialog);
	gtk_widget_show_all(dialog);
	/*result=gtk_dialog_run(GTK_DIALOG(dialog));
	if (result==GTK_RESPONSE_OK) {
		fd = fopen("./chgtkt.conf", "rb+");
		if (fd != NULL) {
			do {
				fgets((char*)str, 100, fd);
			} while(strstr((const char*)str,"check_conf")==NULL);
			fseek(fd,-strlen((const char*)str), SEEK_CUR);
			ptr=(uint8_t*)str;
			while (*ptr++!='=');
			while (*ptr==' ') ptr++;
			if (*(gint*)(data->user_data)) *ptr='1';
			else *ptr='0';
			fputs((const char*)str,fd);
			rewind(fd);
			do {
				fgets((char*)str, 255, fd);
			} while(strstr((const char*)str,"setup_error")==NULL);
			fseek(fd,-strlen((const char*)str), SEEK_CUR);
			ptr=(uint8_t*)str;
			while (*ptr++!='=');
			while (*ptr==' ') ptr++;
			if (!*(gint*)(data->user_data+4)) {
				*ptr++='0';
				*ptr++=' ';
				ptr++;
				strcpy((char*)str2,(const char*)ptr);
				ptr--;
				while(*(str2+i)) *ptr++=str[i++];
			}
			else {
				*ptr++='-';
				*ptr++='1';
				strcpy((char*)str2,(const char*)ptr);
				*ptr++=' ';
				while(*(str2+i)) *ptr++=str2[i++];
			}
			fputs((const char*)str,fd);
			fclose(fd);	 
		}
		
	}*/
	
}



void checkmenuitem11_toggled_cb(GtkWidget * p_wid,gpointer p_data) {									// _ioDAEMON	
	SGlobalData *data =(SGlobalData*) p_data;
	pid_t pid;
	int32_t setupError=255;
	int8_t mySlot=127;
	int32_t myId=_THIS_APP_ID;
	char *arg[]={"sudo","ioDAEMON",NULL};
	mySlot=*(gint*)(data->user_data+12);
	printf("mySlot : %d\n",mySlot);
	printf("check.........................................................\n");
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gtk_builder_get_object (data->builder, "checkmenuitem11")))) {
		
		CHAN_close();
		pid=create_process();                            //sudo ioDAEMON
		switch (pid) {
			case -1://..............................................(ENOMEM error)
				perror("fork");
				exit(0);
			break;
			case 0://...............................................child_process
				printf("Starting \"ioDAEMON\" (pid = %d) as child process running in background.\n",getpid());
				fflush(stdout);
				son_process(arg);
				CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);
			break;
			default://..............................................father_process
				sleep(2);
				printf("Continuing main program (pid = %d) \n",getpid());
			break;
		}
		setupError=CHAN_setup("chgtkt",1);
		switch (setupError) {
			case 0:																						// CHAN_SETUP() is successfull 
				printf("Direct access mode is set.\n");
			break;
			case -1:
				myId=_THIS_APP_ID;
				mySlot=CHAN_getDAEMONslot(myId);
				printf("Slot %d is open.\n\n",mySlot);
				printf("Daemonized access mode is set.\n");
			break;
			case -2:																					// Other errors
				printf("Something wrong has happened, program will close now.\n");
				exit(EXIT_FAILURE);
			break;
			default :																					// The process, which PID is returned, is allready uses immediat mode.
				printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
				exit(EXIT_SUCCESS);
			break;
		}
	}
	else {
		CHAN_releaseDAEMONslot(mySlot,myId);
		sleep(2);
		CHAN_commandDAEMON(_DAEMON_SS_QUIT);
		printf("Killing ioDAEMON...\n");
		fflush(stdout);
		sleep(2);
		setupError=CHAN_setup("chgtkt",1);
		switch (setupError) {
			case 0:																						// CHAN_SETUP() is successfull 
				myId=_THIS_APP_ID;
				mySlot=127;
				printf("Direct access mode is set.\n");
			break;
			case -1:
				myId=_THIS_APP_ID;
				mySlot=CHAN_getDAEMONslot(myId);
				printf("Slot %d is open.\n\n",mySlot);
				printf("Daemonized access mode is set.\n");
			break;
			case -2:																					// Other errors
				printf("Something wrong has happened, program will close now.\n");
				exit(EXIT_FAILURE);
			break;
			default :																					// The process, which PID is returned, is allready uses immediat mode.
				printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
				exit(EXIT_SUCCESS);
			break;
		}
	}
	*(gint*)(data->user_data+8)=setupError;
	*(gint*)(data->user_data+12)=mySlot;
	*(gint*)(data->user_data+16)=myId;
}

void menuitem1_activate_cb(GtkWidget * p_wid,gpointer p_data) {													// _File in menu 	
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWidget* checkMenuItem=NULL;
	static uint32_t i=0;
	checkMenuItem=GTK_WIDGET(gtk_builder_get_object (data->builder, "checkmenuitem11"));
	if (*(gint*)(data->user_data+8)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkMenuItem),TRUE);	// checks if ioDAEMON is running, else uncheck. 
	if (!i++) g_signal_connect (checkMenuItem, "toggled", G_CALLBACK(checkmenuitem11_toggled_cb),data);			// connects signal first time
}

void window1_realize_cb(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWindow *parent=NULL;
	GtkWidget *dialog=NULL,*label=NULL,*checkButton=NULL,*contentArea=NULL;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gchar* message2="According to \"chgtkt.conf\", this application should use direct access mode. Do you want to kill ioDAEMON now ?\n\0";
	gchar* message1="According to \"chgtkt.conf\", this application should use daemonized access mode. Do you want to start ioDAEMON now ?\n\0";
	int32_t result=0;
	uint8_t buf[100]={0};
	uint8_t i=0;
	uint8_t* ptr=NULL; 
	int8_t str[255]="";
	uint8_t savedC='\0';
	FILE* fd = NULL;
	int32_t setupError=255;
	int8_t mySlot=127;
	int32_t myId=_THIS_APP_ID;
	int32_t setupErrorC=127;
	int32_t checkConf=127;
	pid_t pid;
	char *arg[]={"sudo","ioDAEMON",NULL};
	setupError=CHAN_setup("chgtkt",1);
	switch (setupError) {
		case 0:																						// CHAN_SETUP() is successfull 
			printf("Direct access mode is set.\n");
		break;
		case -1:
			myId=_THIS_APP_ID;
			mySlot=CHAN_getDAEMONslot(myId);
			printf("Slot %d is open.\n\n",mySlot);
			printf("Daemonized access mode is set.\n");
		break;
		case -2:																					// Other errors
			printf("Something wrong has happened, program will close now.\n");
			exit(EXIT_FAILURE);
		break;
		default :																					// The process, which PID is returned, is allready uses immediat mode.
			printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
			exit(EXIT_SUCCESS);
		break;
	}  
    fd = fopen("./chgtkt.conf", "r+");
    if (fd != NULL)
    {
		fread(buf,sizeof(char),100,fd);
		fclose(fd);
		ptr=(uint8_t*)strstr((const char*)buf,"setup_error");
		while (*ptr++!='=');
		while (*ptr==' ') ptr++;
		while (ptr[i]!=' '&&ptr[i++]!=0x0A);
		savedC=ptr[i];
		ptr[i]='\0';
		sscanf((const char*)ptr,"%d",(int*)&setupErrorC);
		printf("setupErrorC : %d\n",setupErrorC);
		ptr[i]=savedC;
		ptr=buf;
		i=0;
		ptr=(uint8_t*)strstr((const char*)buf,"check_conf");
		while (*ptr++!='=');
		while (ptr[i++]!=' ');
		while (ptr[i]!=' '&&ptr[i++]!=0x0A);
		savedC=ptr[i];
		ptr[i]='\0';
		sscanf((const char*)ptr,"%d",(int*)&checkConf);
		printf("check conf : %d\n",checkConf);
		ptr[i]=savedC;
		if (checkConf) {
			if (setupError!=setupErrorC) {
				dialog = gtk_dialog_new_with_buttons ("chgtkt WARNING",parent,flags,"_Cancel",GTK_RESPONSE_CANCEL,"_OK",GTK_RESPONSE_OK,NULL);
				contentArea = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
				if (setupError==0) {                                    // start with direct access mode while daemonized access mode is awaited.
					label = gtk_label_new (message1);
					gtk_container_add (GTK_CONTAINER (contentArea), label);
					checkButton= gtk_check_button_new_with_label("Always ask at startup.");
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton),TRUE);
					gtk_container_add (GTK_CONTAINER (contentArea), checkButton);
					g_signal_connect(checkButton,"toggled", G_CALLBACK (setCheckConf_cb2),data);
					gtk_widget_show_all (dialog);
					result=gtk_dialog_run(GTK_DIALOG(dialog));
					if (result==GTK_RESPONSE_OK) {
						CHAN_close();
						pid=create_process();                            //sudo ioDAEMON
						switch (pid) {
							case -1://..............................................(ENOMEM error)
								perror("fork");
								exit(0);
							break;
							case 0://...............................................child_process
								printf("Starting \"ioDAEMON\" (pid = %d) as child process running in background.\n",getpid());
								fflush(stdout);
								son_process(arg);
								CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);
							break;
							default://..............................................father_process
								sleep(2);
								printf("Continuing main program (pid = %d) \n",getpid());
							break;
						}
						setupError=CHAN_setup("chgtkt",1);
						switch (setupError) {
							case 0:																						// CHAN_SETUP() is successfull 
								printf("Direct access mode is set.\n");
							break;
							case -1:
								myId=_THIS_APP_ID;
								mySlot=CHAN_getDAEMONslot(myId);
								printf("Daemonized access mode is set.\n");
								printf("Slot %d is open.\n\n",mySlot);
							break;
							case -2:																					// Other errors
								printf("Something wrong has happened, program will close now.\n");
								exit(EXIT_FAILURE);
							break;
							default :																					// The process, which PID is returned, is allready uses immediat mode.
								printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
								exit(EXIT_SUCCESS);
							break;
						}  
					}
				}
				else {																// start with daemonizedt access mode while direc access mode is awaited.
					label = gtk_label_new (message2);
					gtk_container_add (GTK_CONTAINER (contentArea), label);
					checkButton= gtk_check_button_new_with_label("Always ask at startup.");
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton),TRUE);
					gtk_container_add (GTK_CONTAINER (contentArea), checkButton);
					g_signal_connect(checkButton,"toggled", G_CALLBACK (setCheckConf_cb),data);
					gtk_widget_show_all (dialog);
					result=gtk_dialog_run(GTK_DIALOG(dialog));
					fflush(stdout);
					if (result==GTK_RESPONSE_OK) {
						CHAN_releaseDAEMONslot(mySlot,myId);
						sleep(1);
						CHAN_commandDAEMON(_DAEMON_SS_QUIT);
						printf("Killing ioDAEMON...\n");
						sleep(1);
						setupError=CHAN_setup("chgtkt",1);
						switch (setupError) {
							case 0:																						// CHAN_SETUP() is successfull 
							myId=_THIS_APP_ID;
								mySlot=127;
								printf("Direct access mode is set.\n");
							break;
							case -1:
								myId=_THIS_APP_ID;
								mySlot=CHAN_getDAEMONslot(myId);
								printf("Slot %d is open.\n\n",mySlot);
								printf("Daemonized access mode is set.\n");
							break;
							case -2:																					// Other errors
								printf("Something wrong has happened, program will close now.\n");
								exit(EXIT_FAILURE);
							break;
							default :																					// The process, which PID is returned, is allready uses immediat mode.
								printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
								exit(EXIT_SUCCESS);
							break;
						}
					}
				}
				//g_signal_connect_swapped (dialog,"response",G_CALLBACK (gtk_widget_destroy),dialog);
				//if (result==GTK_RESPONSE_OK || result==GTK_RESPONSE_CANCEL) gtk_widget_destroy (dialog);
				gtk_widget_destroy (dialog);
			}
		}
		*(gint*)(data->user_data)=checkConf;
		*(gint*)(data->user_data+4)=setupErrorC;
		*(gint*)(data->user_data+8)=setupError;
		*(gint*)(data->user_data+12)=mySlot;
		*(gint*)(data->user_data+16)=myId;
		//*(gint*)(data->user_data+16)=
		printf("setupError : %d\n",setupError);
		printf("slot : %d\n",mySlot);
		printf("Id : %d\n",myId);
		fflush(stdout);
		if (result==GTK_RESPONSE_OK && !*(gint*)(data->user_data)) {														// write checkconf on button "ok" and not on checkbutton to eventually cancel.
			fd = fopen("./chgtkt.conf", "rb+");
			if (fd != NULL) {
				do {
					fgets((char*)str, 100, fd);
				} while(strstr((const char*)str,"check_conf")==NULL);
				fseek(fd,-strlen((const char*)str), SEEK_CUR);
				ptr=(uint8_t*)str;
				while (*ptr++!='=');
				while (*ptr==' ') ptr++;
				*ptr='0';
				fputs((const char*)str,fd);
				fclose(fd);	 
			}
		}
	}
	else exit(0);
}
	
int main(int argc, char **argv)
{
	SGlobalData data;
	gint myDatas[8];
	gchar* filename;
	GError* error=NULL;
	GtkWidget*	window=NULL;
	//int32_t setupError=255;
	//int8_t mySlot=127;
	//int32_t myId=255;
	//uint32_t chanErrors=0;
	//gint func_ref =0;
	//gint func_ref2 =0;
		
	//myDatas[0]=checkConf;
	//myDatas[1]=setupErrorConf;
	//myDatas[2]=setupError;
	//myDatas[3]=(gint)mySlot;                                                       							
	//myDatas[4]=myId;
	//myDatas[5]=(gint)chanErrors;																	
	//myDatas[6]=0;																					
	/*
	printf("checkConf : %d\n",*((gint*)data.user_data));
	printf("setupErrorConf : %d\n",*((gint*)data.user_data+1));
	printf("setupError : %d\n",*((gint*)data.user_data+2));
	fflush(stdout);*/
	myDatas[5]=0;
	data.user_data=myDatas;
	gtk_init(&argc, &argv);
	//gdk_init(&argc, &argv);
	data.builder = gtk_builder_new();
	filename =  g_build_filename ("/home/pi/code_004/TESTS/chgtkt/chgtkt.glade", NULL);
	gtk_builder_add_from_file(data.builder,filename,&error);
	g_free (filename);
	if (error) 
	{ 
		gint code = error->code; 
		g_printerr("%s\n", error->message);											// Displays gtk errors. 
		g_error_free (error); 
		return code; 
	} 
	window = GTK_WIDGET(gtk_builder_get_object (data.builder, "window1"));
	gtk_builder_connect_signals (data.builder,&data);
	g_signal_connect (G_OBJECT (window), "destroy", (GCallback)gtk_main_quit, NULL);
	gtk_widget_show_all(window);
	gtk_main();
	if (myDatas[2]==-1) {															// if deamonized access mode, release slot without killing ioDAEMON.
		CHAN_releaseDAEMONslot(myDatas[3],myDatas[4]);								// 
		//sleep(1);
		//CHAN_commandDAEMON(_DAEMON_SS_QUIT);
	}
	else CHAN_close();
	printf("\naccess errors : %d\n\n",(uint32_t)myDatas[5]);						// Displays chantilly access errors (optionnal for debug).
	fflush(stdout);
	return 0;
}
