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
#include "/home/pi/code_004/UTILS/debug_time.h"
//
#include <gtk/gtk.h>
#include <gdk/gdk.h>
//
#include "/home/pi/code_004/common/CHAN/coredef_0xBB_139_020.h"
//
#define _BUS_BOARD      0x07
#define _THIS_APP_ID 	0xBB
#define _THIS_VERS 		0x01

//
typedef struct
{
	GtkBuilder *builder;
    gpointer user_data; 
}SGlobalData;
//
struct sched_param param;

void callback_about (GtkMenuItem *menuitem, gpointer user_data)
{
    SGlobalData *data = (SGlobalData*) user_data;
    GtkAboutDialog *aboutDialog = NULL;
    aboutDialog = GTK_ABOUT_DIALOG( gtk_about_dialog_new ());
    gtk_about_dialog_set_version (aboutDialog,"1.0");
    gtk_about_dialog_set_license(aboutDialog,"1.0");
    gtk_about_dialog_set_copyright (aboutDialog,"©Sideme");
    gtk_window_set_transient_for (GTK_WINDOW(aboutDialog), GTK_WINDOW(gtk_builder_get_object (data->builder, "window1")));
    gtk_dialog_run (GTK_DIALOG (aboutDialog));
    gtk_widget_destroy (GTK_WIDGET(aboutDialog));
}

void callback_help (GtkMenuItem *menuitem, gpointer user_data) {
	SGlobalData *data = (SGlobalData*) user_data;
	GtkWindow *parent=NULL;
	GtkWidget *dialog=NULL,*label=NULL,*contentArea=NULL;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gchar* message="RTC : \n\tUse \" System syncing\" button to synchonize RTC datetime with system datetime\n\nALARM :\n\tSelect alarm week of day with suitable combobox.\n\tAdjust alarm time using \"+/-\" buttons or typing it in suitable entries.\n\t\"any\" checkboxes are used following ways : \n\t\tIf \"any\" checkboxes are all unchecked, alarm occurs when rtc week day, hours, minutes and seconds\n\t\tare matching respectively with alarm week day, hours, minutes and seconds.\n\t\tIf \"any\" checkbox under alarm week day is checked alarm occurs when rtc hours, minutes and seconds\n\t\tare matching respectively with alarm hours, minutes and seconds.\n\t\tIf \"any\" checkbox under alarm hours is checked alarm occurs when rtc minutes and seconds\n\t\tare matching respectively with alarm minutes and seconds.\n\t\tIf \"any\" checkbox under alarm minutes is checked alarm occurs when rtc seconds\n\t\tare matching respectively with alarm seconds.\n\tThe action to execute when the alarm occurs is defined by the \"action\" combobox.\n\tTo enable or disable alarm, check or uncheck \"enabled\" checkbox.\n\nDELAYS :\n\tUse entries to edit poweron and poweroff delays\n\tLeft entries correspond to RAM values, Right entries correspond to EEPROM values.";
	parent=GTK_WINDOW(gtk_builder_get_object(data->builder,"window1"));
	//dialog = gtk_dialog_new_with_buttons ("CRTCM Help",parent,flags,("_OK"),GTK_RESPONSE_ACCEPT,("_Cancel"),GTK_RESPONSE_REJECT,NULL);
	dialog = gtk_dialog_new_with_buttons ("chgrtc Help",parent,flags,("_CLOSE"),GTK_RESPONSE_NONE,NULL);
    contentArea = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	label = gtk_label_new (message);
	g_signal_connect_swapped (dialog,
                           "response",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog);
	gtk_container_add (GTK_CONTAINER (contentArea), label);
	gtk_widget_show_all (dialog);
}

gboolean checkInput(GtkWidget * p_wid, gpointer p_data) {      										// Entries editing.
	const gchar* ptr=NULL;
	gchar str[3]="ee\0";
	char n=0;
	ptr=gtk_entry_get_text(GTK_ENTRY(p_wid));
	do 
	{
		str[n]=*ptr;
		if (*ptr<48||57<*ptr) {
			str[n]='\0';
			gtk_entry_set_text(GTK_ENTRY(p_wid),str);
		}
		ptr++;n++;
	} while (*ptr);
	//printf("ptr : %s\n",ptr);
	//fflush(stdout);
	return FALSE;
}

gboolean button15_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t myRXBuffer[256];
	time_t now = time(NULL);																		// Read current time.
	struct tm tm_now = *localtime(&now);															// Make local time.
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="RTC updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	myRXBuffer[6]=tm_now.tm_year-100;
	myRXBuffer[5]=tm_now.tm_mon+1;
	myRXBuffer[4]=tm_now.tm_mday;
	myRXBuffer[2]=tm_now.tm_hour;
	myRXBuffer[1]=tm_now.tm_min;
	myRXBuffer[0]=tm_now.tm_sec;
	if (*(gint*)(data->user_data+8)==-1) {														// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_DATA,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,7,myRXBuffer,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x02,_DEXEC_COMPLETE);
	}
	else {																						// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,7,myRXBuffer);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x02);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button7_clicked_cb(GtkWidget * p_wid, gpointer p_data) {       							// RTC_ALARM_HOUR ++
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val++;
	if (val>23) val-=24;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_HOUR_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_HOUR_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button8_clicked_cb(GtkWidget * p_wid, gpointer p_data) {      								// RTC_ALARM_HOUR --
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val--;
	if (val<0) val+=24;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_HOUR_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_HOUR_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button9_clicked_cb(GtkWidget * p_wid, gpointer p_data) {       							// RTC_ALARM_MIN ++
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val++;
	if (val>59) val-=60;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_MIN_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_MIN_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button10_clicked_cb(GtkWidget * p_wid, gpointer p_data) {       							// RTC_ALARM_MIN --
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val--;
	if (val<0) val+=60;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_MIN_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_MIN_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button11_clicked_cb(GtkWidget * p_wid, gpointer p_data) {       							// RTC_ALARM_SEC ++
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val++;
	if (val>59) val-=60;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_SEC_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_SEC_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean button12_clicked_cb(GtkWidget * p_wid, gpointer p_data) {       							// RTC_ALARM_SEC --
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry=NULL;
	gchar* ptr=NULL;
	int8_t val=0;
	char t[3];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	ptr=(gchar*)gtk_entry_get_text(entry);
	sscanf((const char*)ptr,"%d",(int*)&val);
	val--;
	if (val<0) val+=60;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_SEC_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_SEC_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	sprintf(t,"%.2d",val);
	gtk_entry_set_text(entry,t);
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean entry4_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {      		// RTC_ALARM_HOUR edit.
SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint8_t val=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_HOUR_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_HOUR_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean entry5_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {     	 	// RTC_ALARM_MIN edit
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint8_t val=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_MIN_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_MIN_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean entry6_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {      		// RTC_ALARM_SEC edit
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint8_t val=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_SEC_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&val,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_SEC_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)val);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}


gboolean entry7_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {      		// POWEROFF edit in RAM
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint16_t val=0;
	uint8_t myTXBuffer[2];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="POWEROFF updated in RAM.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	myTXBuffer[0]=val&0xFF;
	myTXBuffer[1]=(val>>8)&0xFF;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_HOST_POWEROFF_SECS_L,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_HOST_POWEROFF_SECS_L);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean entry8_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {      		// POWERON edit in RAM
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint16_t val=0;
	uint8_t myTXBuffer[2];
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="POWERON updated in RAM.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	myTXBuffer[0]=val&0xFF;
	myTXBuffer[1]=(val>>8)&0xFF;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_HOST_POWER_ON_SECS_L,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_HOST_POWER_ON_SECS_L);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean entry9_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {			// POWEROFF edit in EEPROM
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint16_t val=0;
	uint8_t myTXBuffer[16]={0x26,0};
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="POWEROFF updated in EEPROM.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	myTXBuffer[2]=val&0xFF;
	myTXBuffer[3]=(val>>8)&0xFF;
	*(guint*)(data->user_data+36)=1;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1E4,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x11,_DEXEC_COMPLETE);
		usleep(500000);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1BC,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer+2,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x10,_DEXEC_COMPLETE);	
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1E4);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x11);
		usleep(500000);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1BC);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,&myTXBuffer[2]);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x10);																		
	}
	*(guint*)(data->user_data+36)=0;
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean entry10_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {			// POWERON edit in EEPROM
	SGlobalData *data =(SGlobalData*) p_data;
	gchar* ptr=NULL;
	uint16_t val=0;
	uint8_t myTXBuffer[16]={0x26,0};
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="POWERON updated in EEPROM.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ptr=(gchar*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&val);
	myTXBuffer[2]=val&0xFF;
	myTXBuffer[3]=(val>>8)&0xFF;
	*(guint*)(data->user_data+36)=1;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1E4,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x11,_DEXEC_COMPLETE);
		usleep(500000);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1BE,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,&myTXBuffer[2],_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x10,_DEXEC_COMPLETE);	
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1E4);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x11);
		usleep(500000);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1BE);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,&myTXBuffer[2]);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x10);																		
	}
	*(guint*)(data->user_data+36)=0;
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return FALSE;
}

gboolean comboboxtext1_changed_cb (GtkWidget * p_wid, gpointer p_data) {							// RTC_ALARM_WDAY changed.
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t alarmDay=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	alarmDay=gtk_combo_box_get_active(GTK_COMBO_BOX(GTK_COMBO_BOX_TEXT(p_wid)));
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_WDAY_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,(uint8_t*)&alarmDay,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_WDAY_B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,(uint8_t)alarmDay);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean comboboxtext2_changed_cb (GtkWidget * p_wid, gpointer p_data) {							// ALARM0_ACTION edit in RAM
	SGlobalData *data =(SGlobalData*) p_data;
	GtkComboBoxText* comboBoxText=NULL;
	uint8_t alarmAction=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar info[100]="Alarm action updated.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info"); 
	comboBoxText=GTK_COMBO_BOX_TEXT(p_wid);
	alarmAction=gtk_combo_box_get_active (GTK_COMBO_BOX(comboBoxText));
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_ALARM0_ACTION_R,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmAction,_DEXEC_COMPLETE);
		if (alarmAction==0) {																		// To use poweron as alarm, bus_board must don't be stopped 
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x120,_DEXEC_COMPLETE);			// and host must be stopped so set POWEROFF at 0x120 to 0.
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmAction,_DEXEC_COMPLETE);
		}
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else { 																							// Else direct acces mode (mySlot==-1).		
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);	
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_ALARM0_ACTION_R);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmAction);
		if (alarmAction==0) {																		// To use poweron as alarm, bus_board must don't be stopped 
			*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x120);															// and host must be stopped so set POWEROFF at 0x120 in RAM to 0.
			*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,0);
		}
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}  

gboolean checkbutton4_toggled_cb(GtkWidget * p_wid, gpointer p_data) {								// ALARM0 enabled in RAM.
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t alarmEnabled=0;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar* info="ooooooooooooooooooooooooooooooooooooooooooo";
    gchar activeStr[100]="Alarm enabled.";
    gchar inactiveStr[100]="Alarm disabled.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		alarmEnabled=1;
		info=&activeStr[0];
	}
	else {
		alarmEnabled=0;
		info=&inactiveStr[0];
	}
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_ALARM0_C,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmEnabled,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
	}
	else {																							// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_ALARM0_C);
		*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmEnabled);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean checkbutton1_toggled_cb(GtkWidget * p_wid, gpointer p_data) {								// RTC_ALARM_WDAY enabled.
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t alarmMatch=0;
	GtkCheckButton* checkButton[3]={NULL,NULL,NULL};
	GtkComboBoxText* comboBoxText=NULL;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar *info="Alarm occurs when week day, hours, minutes and seconds are matching.kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk\0";
    gchar *activeStr="ALARM_MATCH=1 : h, min, sec.";
    gchar *inactiveStr="ALARM_MATCH=0 : day, h, min, sec.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	checkButton[0]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	checkButton[1]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton2"));
	checkButton[2]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton3"));
	comboBoxText=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext1"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {									// If checkbutton1 checked.
		alarmMatch=1;
		gtk_widget_set_sensitive (GTK_WIDGET(comboBoxText),FALSE);
		if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[1])))) {					//  If checkbutton2 unchecked, write alarmMatch.
			if (*(gint*)(data->user_data+8)==-1) {													//   If deamonized access mode (mySlot!=-1).
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
			}
			else {																					//   Else direct acces mode (mySlot==-1).
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
				 *(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);
				 *(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
			}	
			info=activeStr;			
		}
	}
	else {																							// Else, checkbutton1 unchecked.
		alarmMatch=0;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[1]))) {						//  If checkbutton2 checked, uncheck it and call coresponding event.
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[1]),FALSE);
		}
		if (*(gint*)(data->user_data+8)==-1) {														//  If deamonized access mode (mySlot!=-1).
			 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
			 *(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);
			 *(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);
			 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
		}
		else {																						//  Else direct acces mode (mySlot==-1).
			 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
			 *(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);
			 *(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);
			 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
		}
		info=inactiveStr;
		gtk_widget_set_sensitive (GTK_WIDGET(comboBoxText),TRUE);
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean checkbutton2_toggled_cb(GtkWidget * p_wid, gpointer p_data) {								// RTC_ALARM_HOUR enabled.
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t alarmMatch=0;
	GtkCheckButton* checkButton[4]={NULL,NULL,NULL,NULL};
	GtkEntry* ent=NULL;
	GtkButton* button[2]={NULL,NULL};
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar *info="Alarm occurs when week day, hours, minutes and seconds are matching.\0";
    gchar *activeStr="ALARM_MATCH=2 : min, sec.";
    gchar *inactiveStr="ALARM_MATCH=1 : h, min, sec.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	checkButton[0]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	checkButton[1]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton2"));
	checkButton[2]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton3"));
	ent=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	button[0]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button7"));
	button[1]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button8"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {									// If checkbutton2 checked.
		alarmMatch=2;
		gtk_widget_set_sensitive (GTK_WIDGET(ent),FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[0]),FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[1]),FALSE);
		if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[0])))) {					//  If checkbutton0 unchecked, check it and call corresponding event.
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[0]),TRUE);							
		}
		if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[2])))) {					//  If checkbutton3 unchecked, write alarmMatch.
			if (*(gint*)(data->user_data+8)==-1) {													//   If deamonized access mode (mySlot!=-1).
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
			}
			else {																					//   Else direct acces mode (mySlot==-1).
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
				 *(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);
				 *(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
			}
			info=activeStr;
		}
	}
	else {																							// Else checkbutton2 unchecked.
		alarmMatch=1;
		gtk_widget_set_sensitive (GTK_WIDGET(ent),TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[0]),TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[1]),TRUE);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[2]))) {        				// If checkbutton3 checked, uncheck it and call corresponding event. 
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[2]),FALSE);	
		}
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[0]))) {        				// If checkbutton1 checked, write alarmMatch  
			if (*(gint*)(data->user_data+8)==-1) {													//  If deamonized access mode (mySlot!=-1).
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);				
				 *(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);			
				 *(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);	
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);				
			}
			else {																					//  Else direct acces mode (mySlot==-1).
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);								
				 *(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);									
				 *(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);							
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
			}
			info=inactiveStr;
		}
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean checkbutton3_toggled_cb(GtkWidget * p_wid, gpointer p_data) {								// RTC_ALARM_SEC enabled.
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t alarmMatch=0;
	GtkCheckButton* checkButton[4]={NULL,NULL,NULL,NULL};
	GtkEntry* ent={NULL};
	GtkButton* button[2]={NULL,NULL};
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
    gchar *info="Alarm occurs when week day, hours, minutes and seconds are matching.\0";
    gchar *activeStr="ALARM_MATCH=3 : sec.";
    gchar *inactiveStr="ALARM_MATCH=2 : min, sec.";
    statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	checkButton[0]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	checkButton[1]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton2"));
	checkButton[2]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton3"));
	ent=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	button[0]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button9"));
	button[1]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button10"));	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {									// If checkbutton3 checked.
		alarmMatch=3;
		gtk_widget_set_sensitive (GTK_WIDGET(ent),FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[0]),FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[1]),FALSE);
		if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[1])))) {					//  If checkbutton2 unchecked, check it and call corresponding event.
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[1]),TRUE);
		}		
		if (*(gint*)(data->user_data+8)==-1) {														//  If deamonized access mode (mySlot!=-1).
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
		}
		else {																						//  Else direct acces mode (mySlot==-1).
			*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
			*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);
			*(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);
			*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
		}
		info=activeStr;
	}
	else {																							// Else, checkbutton3 unchecked.
		alarmMatch=2;
		gtk_widget_set_sensitive (GTK_WIDGET(ent),TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[0]),TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(button[1]),TRUE);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkButton[1]))) {        				// If checkbutton2 checked, write alarmMatch
			if (*(gint*)(data->user_data+8)==-1) {													//  If deamonized access mode (mySlot!=-1).
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x14C,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,&alarmMatch,_DEXEC_COMPLETE);
				 *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x03,_DEXEC_COMPLETE);
			}
			else {																					//  Else direct acces mode (mySlot==-1).
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
				 *(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x14C);
				 *(gint*)(data->user_data+20)+=CHAN_setByte(_BUS_BOARD,alarmMatch);
				 *(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x03);
			}
			info=inactiveStr;
		}
	}
	gtk_statusbar_push(GTK_STATUSBAR(statusBar), id, info);
	return TRUE;
}

gboolean getRTCDatetime_di(gpointer p_data)
{
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	uint8_t myRXBuffer[256];
	char s_rtc[sizeof "JJ/MM/AAAA HH:MM:SS"];
	struct tm tm_rtc;
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label11"));
	if (!*(guint*)(data->user_data+32)) {											// doesn't update rtc if timerStatusbar is running.
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(_BUS_BOARD,7,&myRXBuffer[0]);
		tm_rtc.tm_sec=myRXBuffer[0];
		tm_rtc.tm_min=myRXBuffer[1];
		tm_rtc.tm_hour=myRXBuffer[2];
		tm_rtc.tm_wday=myRXBuffer[3];
		tm_rtc.tm_mday=myRXBuffer[4];
		tm_rtc.tm_mon=myRXBuffer[5]-1;
		tm_rtc.tm_year=myRXBuffer[6]+100;
		strftime (s_rtc, sizeof s_rtc, "%d/%m/%Y %H:%M:%S", &tm_rtc);
		gtk_label_set_text(GTK_LABEL(label), s_rtc);
	}
	return TRUE;
}

gboolean getRTCDatetime_de(gpointer p_data)
{
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	uint8_t myRXBuffer[256];
	char s_rtc[sizeof "JJ/MM/AAAA HH:MM:SS"];
	struct tm tm_rtc;
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label11"));
	if (!*(guint*)(data->user_data+32)) {											// doesn't update rtc if timerStatusbar is running.
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_DATA,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,7,&myRXBuffer[0],_DEXEC_COMPLETE);
		tm_rtc.tm_sec=myRXBuffer[0];
		tm_rtc.tm_min=myRXBuffer[1];
		tm_rtc.tm_hour=myRXBuffer[2];
		tm_rtc.tm_wday=myRXBuffer[3];
		tm_rtc.tm_mday=myRXBuffer[4];
		tm_rtc.tm_mon=myRXBuffer[5]-1;
		tm_rtc.tm_year=myRXBuffer[6]+100;
		strftime (s_rtc, sizeof s_rtc, "%d/%m/%Y %H:%M:%S", &tm_rtc);
		gtk_label_set_text(GTK_LABEL(label), s_rtc);
	}
	return TRUE;
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

void setCheckConf_cb2(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) *(gint*)(data->user_data)=1;
	else *(gint*)(data->user_data)=0;
}
	
void setCheckConf_cb(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	FILE* fd = NULL;
	uint8_t* ptr=NULL; 
	int8_t str[100]="";
	fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "rb+");
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
	fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "rb+");
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
		fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "rb+");
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
	char *arg[]={"sudo","chdaemon",NULL};
	mySlot=*(gint*)(data->user_data+12);
	//printf("mySlot : %d\n",mySlot);
	//printf("check.........................................................\n");
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gtk_builder_get_object (data->builder, "checkmenuitem11")))) {
		
		CHAN_close();
		pid=create_process();                            //sudo ioDAEMON
		switch (pid) {
			case -1://..............................................(ENOMEM error)
				perror("fork");
				exit(0);
			break;
			case 0://...............................................child_process
				printf("Starting \"chdaemon\" (pid = %d) as child process running in background.\n",getpid());
				fflush(stdout);
				son_process(arg);
				CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);
			break;
			default://..............................................father_process
				sleep(2);
				printf("Continuing main program (pid = %d) \n",getpid());
			break;
		}
		mySlot=CHAN_getDAEMONslot(myId);
		CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);		
		setupError=-1;
		//printf("mySlot : %d\n",mySlot);
		//fflush(stdout);
		/*setupError=CHAN_setup("chgrtc",1);
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
		}*/
	}
	else {
		//CHAN_releaseDAEMONslot(mySlot,myId);
		//sleep(2);
		printf("Killing chdaemon...\n");
		CHAN_commandDAEMON(_DAEMON_SS_QUIT);
		while (CHAN_checkDAEMON(1)!=-1) {printf("*");fflush(stdout);}
		
		fflush(stdout);
		//sleep(2);
		setupError=CHAN_setup("chgrtc",1);
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
	g_source_remove(*(guint*)(data->user_data+24));	
	if ( *(gint*)(data->user_data+8)==-1) {
		*(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_de, data);
	
	}
	else {
	
		*(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_di, data);
	}
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
	setupError=CHAN_setup("chgrtc",1);
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
    fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "r+");
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
						setupError=-1;
						mySlot=CHAN_getDAEMONslot(myId);
						/*
						setupError=CHAN_setup("chgrtc",1);
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
						}  */
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
						//CHAN_releaseDAEMONslot(mySlot,myId);
						//sleep(1);
						CHAN_commandDAEMON(_DAEMON_SS_QUIT);
						printf("Killing chdaemon...\n");
						while (CHAN_checkDAEMON(1)!=-1) {printf("-");fflush(stdout);}
						sleep(1);
						setupError=CHAN_setup("chgrtc",1);
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
		printf("setupError : %d\n",setupError);
		printf("slot : %d\n",mySlot);
		printf("Id : %d\n",myId);
		fflush(stdout);
		if (result==GTK_RESPONSE_OK && !*(gint*)(data->user_data)) {
			fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "rb+");
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
	
void window1_realize_cb2(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* ent[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkCheckButton* checkButton[4]={NULL,NULL,NULL,NULL};
	GtkComboBoxText* comboBoxText[2]={NULL,NULL};
	uint8_t myRXBuffer[256];
	uint8_t myTXBuffer[16]={0x26,0};
	uint16_t poweron=0,poweroff=0,alarmEnabled=0,alarmAction=0,alarmSec=0,alarmMin=0,alarmHour=0,alarmDay=0,alarmMatch=0;
	uint16_t poweronEEPROM=0,poweroffEEPROM=0;//,alarmEnabledEEPROM=0,alarmActionEEPROM=0;
	char t[11][6]={("0A"),("0B"),("0C"),("0D")};
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
	gchar info[100] = "Access errors : 00000";
	statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	ent[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	ent[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	ent[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	ent[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	ent[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	ent[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	ent[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	checkButton[0]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	checkButton[1]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton2"));
	checkButton[2]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton3"));
	checkButton[3]=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton4"));
	comboBoxText[0]=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext1"));
	comboBoxText[1]=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	
	/*if ( *(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_de, data);
	else {
		*(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_di, data);
	}*/
	
	if (*(gint*)(data->user_data+8)==-1) {													// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_HOST_POWEROFF_SECS_L,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,4,myRXBuffer,_DEXEC_COMPLETE);
		poweroff=(myRXBuffer[1]<<8)+myRXBuffer[0];
		poweron=(myRXBuffer[3]<<8)+myRXBuffer[2];
		
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_ALARM0_C,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myRXBuffer,_DEXEC_COMPLETE);
		alarmEnabled=myRXBuffer[0];
		alarmAction=myRXBuffer[1];
		
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x01,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,_BB_RTC_ALARM_SEC_B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,6,myRXBuffer,_DEXEC_COMPLETE);
		alarmSec=myRXBuffer[0];
		alarmMin=myRXBuffer[1];
		alarmHour=myRXBuffer[2];
		alarmDay=myRXBuffer[3];
		alarmMatch=myRXBuffer[5];
			
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1E4,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x11,_DEXEC_COMPLETE);
		usleep(500000);
			
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1BC,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,1,myRXBuffer,_DEXEC_COMPLETE);
		poweroffEEPROM=(myRXBuffer[1]<<8)+myRXBuffer[0];
		poweronEEPROM=(myRXBuffer[3]<<8)+myRXBuffer[2];
		/*myTXBuffer[1]=1;
		CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1E4,_DEXEC_COMPLETE);
		CHAND_setBytes(*(gint*)(data->user_data+12),_BUS_BOARD,2,myTXBuffer,_DEXEC_COMPLETE);
		CHAND_command(*(gint*)(data->user_data+12),_BUS_BOARD,0x11,_DEXEC_COMPLETE);
		usleep(500000);
		CHAND_addr(*(gint*)(data->user_data+12),_BUS_BOARD,0x1B9,_DEXEC_COMPLETE);
		CHAND_getBytes(*(gint*)(data->user_data+12),_BUS_BOARD,4,myRXBuffer,_DEXEC_COMPLETE);
		alarmEnabledEEPROM=myRXBuffer[0];
		alarmActionEEPROM=myRXBuffer[3];*/
		*(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_de, data);
	}
	else {																					// Else direct acces mode (mySlot==-1).
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_HOST_POWEROFF_SECS_L);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(_BUS_BOARD,4,&myRXBuffer[0]);
		poweroff=(myRXBuffer[1]<<8)+myRXBuffer[0];
		poweron=(myRXBuffer[3]<<8)+myRXBuffer[2];
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_ALARM0_C);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(_BUS_BOARD,2,&myRXBuffer[0]);
		alarmEnabled=myRXBuffer[0];
		alarmAction=myRXBuffer[1];
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x01);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,_BB_RTC_ALARM_SEC_B);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(_BUS_BOARD,6,&myRXBuffer[0]);
		alarmSec=myRXBuffer[0];
		alarmMin=myRXBuffer[1];
		alarmHour=myRXBuffer[2];
		alarmDay=myRXBuffer[3];
		alarmMatch=myRXBuffer[5];
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1E4);
		*(gint*)(data->user_data+20)+=CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
		*(gint*)(data->user_data+20)+=CHAN_command(_BUS_BOARD,0x11);
		usleep(500000);
		*(gint*)(data->user_data+20)+=CHAN_addr(_BUS_BOARD,0x1BC);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(_BUS_BOARD,4,myRXBuffer);
		poweroffEEPROM=(myRXBuffer[1]<<8)+myRXBuffer[0];
		poweronEEPROM=(myRXBuffer[3]<<8)+myRXBuffer[2];
		/*myTXBuffer[1]=1;
		CHAN_addr(_BUS_BOARD,0x1E4);
		CHAN_setBytes(_BUS_BOARD,2,myTXBuffer);
		CHAN_command(_BUS_BOARD,0x11);
		usleep(500000);
		CHAN_addr(_BUS_BOARD,0x1B9);
		CHAN_getBytes(_BUS_BOARD,4,myRXBuffer);
		alarmEnabledEEPROM=myRXBuffer[0];
		alarmActionEEPROM=myRXBuffer[3];*/
		*(gint*)(data->user_data+24) = g_timeout_add (1000, getRTCDatetime_di, data);
	}
	
	sprintf(&t[0][0],"%.2d",alarmHour);
	sprintf(&t[1][0],"%.2d",alarmMin);
	sprintf(&t[2][0],"%.2d",alarmSec);
	sprintf(&t[3][0],"%d",poweroff);
	sprintf(&t[4][0],"%d",poweron);
	sprintf(&t[5][0],"%.2d",poweroffEEPROM);
	sprintf(&t[6][0],"%.2d",poweronEEPROM);
	usleep(50000);
	gtk_combo_box_set_active (GTK_COMBO_BOX(comboBoxText[0]),alarmDay);
	usleep(50000);
	gtk_entry_set_text(ent[1],t[0]);
	usleep(50000);
	gtk_entry_set_text(ent[2],t[1]);
	usleep(50000);
	gtk_entry_set_text(ent[3],t[2]);
	usleep(50000);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[3]),alarmEnabled);
	usleep(50000);
	gtk_combo_box_set_active (GTK_COMBO_BOX(comboBoxText[1]),alarmAction);
	usleep(50000);
	gtk_entry_set_text(ent[4],t[3]);
	usleep(50000);
	gtk_entry_set_text(ent[5],t[4]);
	usleep(50000);
	//gtk_entry_set_text(ent[6],t[5]);
	usleep(50000);
	//gtk_entry_set_text(ent[7],t[6]);
	usleep(50000);
	
	switch (alarmMatch) {
		case 1 :
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[0]),TRUE);
		break;
		case 2 :
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[1]),TRUE);
		break;
		case 3 :
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkButton[2]),TRUE);
		break;
		default :
		break;
	}
	sprintf(info,"Access errors : %05d",*(gint*)(data->user_data+20));
	gtk_statusbar_push(statusBar, id, info);
}

gboolean setStatusBar(gpointer p_data) 
{
	SGlobalData *data =(SGlobalData*) p_data;
	GtkStatusbar*	statusBar =NULL;
	guint id =0;
	gchar info[] = "Access errors : 00000\0";
	statusBar = GTK_STATUSBAR(gtk_builder_get_object (data->builder, "StatusBar"));
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusBar), "info");
	sprintf(info,"Access errors : %05d",*(gint*)(data->user_data+20));
	gtk_statusbar_push(statusBar, id, info);
	return FALSE;
}

gboolean resetStatusBar2(GtkStatusbar *statusbar,guint context_id,gchar *text,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWindow* window=NULL;
	window = GTK_WINDOW(gtk_builder_get_object (data->builder, "window1"));
	if (*(guint*)(data->user_data+28)) {															// If a timeout is already existing. 	
		g_source_remove(*(guint*)(data->user_data+28));												//  Cancel current timeout.				
		*(guint*)(data->user_data+28)=0;															//  Reset to 0.							
		if (*(guint*)(data->user_data+36)) {
			*(guint*)(data->user_data+28)=g_timeout_add (5000, setStatusBar, data);
		}
		*(guint*)(data->user_data+36)=0;
	}
	else {
		if (!*(guint*)(data->user_data+36)) {
			*(guint*)(data->user_data+28)=g_timeout_add (5000, setStatusBar, data);
			*(guint*)(data->user_data+36)=1;
		}
	}	
	gtk_window_set_focus (window,NULL);																	// avoid multiple writing when moving window with focus on an entry. 
	return FALSE;  
}


int main(int argc, char **argv)
{
	SGlobalData data;
	gint myDatas[16];
	gchar* filename;
	GError* error=NULL;
	GtkWidget*	window=NULL;
	//myDatas[0]=checkConf;
	//myDatas[1]=setupErrorConf;
	//myDatas[2]=setupError;
	//myDatas[3]=(gint)mySlot;                                                       							
	//myDatas[4]=myId;
	//myDatas[5]=(gint)chanErrors;																	
	//myDatas[6]=timer_RTC;
	//myDatas[7]=timer_statusbar;
	//myDatas[8]=timer_rtc_is_not_running;																				
	//myDatas[9]=timer_statusbar_is_running;
	myDatas[5]=0;
	myDatas[6]=0;	
	myDatas[7]=0;
	myDatas[8]=0;
	myDatas[9]=0;
	data.user_data=myDatas;
	gtk_init(&argc, &argv);
	data.builder = gtk_builder_new();
	filename =  g_build_filename ("/home/pi/code_004/TESTS/chgrtc/chgrtc.glade", NULL);
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

