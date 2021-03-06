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
#define _IO24		    0x02
#define _THIS_APP_ID 	0x10
#define _THIS_VERS 		0x01

typedef struct
{
	GtkBuilder *builder;
    gpointer user_data; 
}SGlobalData;
//
struct sched_param param;

gboolean checkInput(GtkWidget * p_wid, gpointer p_data) {      										// Entries editing. 
	const gchar* ptr=NULL;																			// Accepts only numeric characters.
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
	gchar* message="IO24 servo outputs mode can apply PWM VALUES or a PWM USER SEQUENCE on outputs.\n\nIO24 CORE : \n\tSelect an IO24 core which is connected on the chantilly stack with the \"IO24 CORE\" combobox.\n\nPWM VALUES : \n\tUse scale(s) to adjust and set PWMn_VAL(s) assuming that PWMn is the PWM applied on OutputA bitn if 0≤n≤7 or OutputB bitn if 8≤n≤15\n\t(PWMn_VALs are bytes at addresses in the range from 0x108 to 0x117 in IO24 RAM).\n\nBUFFER :\n\tClick suitable \"↑\" button(s) above the entries to get PWMn_VAL(s) from the buffer.\n\tSet PWMn_VAL(s) to the buffer by clicking suitable \"↓\" button(s) above the entries or type values into entries.\n\tClick the \"↑\" button below the entries to get the buffer from selected user sequence.\n\tClick the \"↓\" button below the entries to set the buffer to selected user sequence.\n\nPWM USER SEQUENCE : \n\tSelect the sequence where to store the buffer with the \"a, b, c, d, ...o\" toggle buttons.\n\t (These toggles can also be released by using the \"clear\" button.)\n\nCOMMAND : \n\tChoose to run the PWM VALUES or play a PWM USER SEQUENCE with suitable toggle button. \n\tWhen a toggle button is pressed, IO24 command 0x05 named SERVO_RUN is executed : IO24 (immediate inputs and) servo outputs mode starts.\n\tWhen a toggle button is released,  IO24 command 0x06 named STOP is executed : IO24 running mode stops\n\t (These toggles can also be released with the \"stop\" button.)";
	parent=GTK_WINDOW(gtk_builder_get_object(data->builder,"window1"));
	dialog = gtk_dialog_new_with_buttons ("chgservo Help",parent,flags,("_CLOSE"),GTK_RESPONSE_NONE,NULL);
    contentArea = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	label = gtk_label_new (message);
	g_signal_connect_swapped (dialog,
                           "response",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog);
	gtk_container_add (GTK_CONTAINER (contentArea), label);
	gtk_widget_show_all (dialog);
}


gboolean scale16_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x117,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x117);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale15_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x116,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x116);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale14_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x115,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x115);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale13_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x114,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x114);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale12_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x113,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x113);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale11_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x112,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x112);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale10_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x111,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x111);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale9_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x110,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x110);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale8_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10F,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10F);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale7_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10E,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10E);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale6_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10D,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10D);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale5_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10C,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10C);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale4_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	//printf("value = %d \n",(uint8_t*)&value);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10B);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale3_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10A,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x10A);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale2_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x109,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x109);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean scale1_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (setupError==-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x108,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {																							// Else direct access mode.
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x108);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean button1_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM0_VAL (scale1) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale1"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}

gboolean button2_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM1_VAL (scale2) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale2"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}

gboolean button3_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM2_VAL (scale3) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale3"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}

gboolean button4_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM3_VAL (scale4) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale4"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button5_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM4_VAL (scale5) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale5"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button6_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM5_VAL (scale6) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale6"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button7_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM6_VAL (scale7) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale7"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button8_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM7_VAL (scale8) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale8"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button9_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM8_VAL (scale9) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale9"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button10_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM9_VAL (scale10) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale10"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button11_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM10_VAL (scale11) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale11"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button12_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM11_VAL (scale12) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale12"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button13_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM12_VAL (scale13) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale13"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button14_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM13_VAL (scale14) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale14"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button15_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM14_VAL (scale15) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale15"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}
gboolean button16_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set PWM15_VAL (scale16) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale16"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	gdouble value=0;
	char t[4]="\0";
	value=gtk_range_get_value(GTK_RANGE(scale));
	sprintf(t,"%f",value);
	gtk_entry_set_text(entry,t);
	return TRUE;
}

gboolean button17_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// set all PWM VALUES (all scales) to buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	gdouble value[16]={0};
	char t[16][4]={"\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0"};
	uint8_t i=0;
	scale[0]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale1"));
	scale[1]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale2"));
	scale[2]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale3"));
	scale[3]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale4"));
	scale[4]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale5"));
	scale[5]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale6"));
	scale[6]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale7"));
	scale[7]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale8"));
	scale[8]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale9"));
	scale[9]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale10"));
	scale[10]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale11"));
	scale[11]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale12"));
	scale[12]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale13"));
	scale[13]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale14"));
	scale[14]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale15"));
	scale[15]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale16"));
	entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	for (i=0;i<16;i++) {
		value[i]=gtk_range_get_value(GTK_RANGE(scale[i]));
		sprintf(t[i],"%f",value[i]);
		gtk_entry_set_text(entry[i],t[i]);
	}
	return TRUE;
}

gboolean button18_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM0_VAL (scale1) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale1"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button19_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM1_VAL (scale2) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale2"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button20_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM2_VAL (scale3) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale3"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button21_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM3_VAL (scale4) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale4"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button22_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM4_VAL (scale5) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale5"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button23_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM5_VAL (scale6) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale6"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button24_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM6_VAL (scale7) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale7"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button25_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM7_VAL (scale8) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale8"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button26_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM8_VAL (scale9) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale9"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button27_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM9_VAL (scale10) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale10"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button28_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM10_VAL (scale11) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale11"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button29_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM11_VAL (scale12) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale12"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button30_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM12_VAL (scale13) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale13"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button31_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM13_VAL (scale14) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale14"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button32_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM14_VAL (scale15) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale15"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

gboolean button33_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get PWM15_VAL (scale16) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale=GTK_SCALE(gtk_builder_get_object(data->builder,"scale16"));
	GtkEntry* entry=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	uint8_t value=0;
	uint8_t* t=NULL; 
	t=(uint8_t*)gtk_entry_get_text(entry);
	sscanf((const char*)t,"%d",(int*)&value);
	gtk_range_set_value(GTK_RANGE(scale),(gdouble)value);
	return TRUE;
}

/*gboolean entries_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {			
	const int8_t* ptr=(int8_t*)"eee\0";
	int8_t str[4]="eee\0";
	uint8_t value=0;
	ptr=(const int8_t*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&value);
	sprintf((char*)str,"%.3d",value);
	gtk_entry_set_text(GTK_ENTRY(p_wid),(const gchar*)str);
	return FALSE;
}*/

gboolean button34_clicked_cb(GtkWidget * p_wid, gpointer p_data) {									// get all PWM VALUES (all scales) from buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkScale* scale[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	int8_t value[16]={0};
	int8_t* t[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}; 
	uint8_t i=0;
	scale[0]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale1"));
	scale[1]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale2"));
	scale[2]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale3"));
	scale[3]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale4"));
	scale[4]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale5"));
	scale[5]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale6"));
	scale[6]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale7"));
	scale[7]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale8"));
	scale[8]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale9"));
	scale[9]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale10"));
	scale[10]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale11"));
	scale[11]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale12"));
	scale[12]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale13"));
	scale[13]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale14"));
	scale[14]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale15"));
	scale[15]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale16"));
	entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	for (i=0;i<16;i++) {
		t[i]=(int8_t*)gtk_entry_get_text(entry[i]);
		sscanf((const char*)t[i],"%d",(int*)(value+i));
		gtk_range_set_value(GTK_RANGE(scale[i]),(gdouble)value[i]);
	}
	return TRUE;
}

/*gboolean button108_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	char* t[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t myTXBuffer[17];
	uint8_t i=0;
	entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	entry[16]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry17"));
	for (i=0;i<16;i++) {
		t[i]=(char*)gtk_entry_get_text(entry[i]);

		sscanf((const char*)t[i],"%d",(int*)&myTXBuffer[i]);
		printf("myTXBuffer [%d]=%d\n",i,myTXBuffer[i]);
	}
	t[16]=(char*)gtk_entry_get_text(entry[16]);
	//strcpy(t2,t[16]);
	sscanf((const char*)t[16],"%x",(int*)&myTXBuffer[16]);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),myTXBuffer[16],_DEXEC_COMPLETE);	//
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),16,myTXBuffer,_DEXEC_COMPLETE);		// Write user sequence in free RAM.
			* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			//
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&myTXBuffer[16],_DEXEC_COMPLETE);// Write PWM_SEQ_INDEX value @ 0x108.
		}
		else {	
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),myTXBuffer[16]);								//
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),16,myTXBuffer);							// Write user sequence in free RAM.
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										//
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),1,&myTXBuffer[16]);						// Write PWM_SEQ_INDEX value @ 0x118.
	}		
	return TRUE;
}*/

gboolean comboboxtext2_changed_cb(GtkWidget * p_wid, gpointer p_data) {										// Hiden combobox "comboboxtext2" changed. 
	SGlobalData *data =(SGlobalData*) p_data;																// (When selecting a sequence to store the buffer by clicking 
	GtkButton* button[2]={NULL,NULL};																		// a toggle button in PWM USER SEQUENCE, this combobox is set 
	button[0]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button35"));									// to index of the sequence). 
	button[1]=GTK_BUTTON(gtk_builder_get_object(data->builder,"button36"));									
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(p_wid))>0) {
		gtk_widget_set_sensitive(GTK_WIDGET(button[0]),TRUE);												// unfreeze buttons 35 and 36 because sequence index is between 0x04 and 0xE4.
		gtk_widget_set_sensitive(GTK_WIDGET(button[1]),TRUE);												// (hiden combobox between 1 and 15).
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(button[0]),FALSE);												// freeze buttons 35 and 36 because sequence index is not between 0x04 and 0xE4.
		gtk_widget_set_sensitive(GTK_WIDGET(button[1]),FALSE);												// (hiden combobox to 0).
	}
	return TRUE;
}

/*void test (GtkWidget * p_wid,   GParamSpec *child_property,gpointer p_data) {
	printf("coucoucou**********************************\n");
}*/

void releaseToggleButton (GtkWidget * p_wid, gpointer p_data) {												// releases pressed toggle button in PWM USER SEQUENCE 
	SGlobalData *data =(SGlobalData*) p_data;																// when clicking another toggle button.
	GtkToggleButton* togglebutton[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t i=0;
	togglebutton[0]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton2"));
	togglebutton[1]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton3"));
	togglebutton[2]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton4"));
	togglebutton[3]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton5"));
	togglebutton[4]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton6"));
	togglebutton[5]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton7"));
	togglebutton[6]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton8"));
	togglebutton[7]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton9"));
	togglebutton[8]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton10"));
	togglebutton[9]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton11"));
	togglebutton[10]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton12"));
	togglebutton[11]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton13"));
	togglebutton[12]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton14"));
	togglebutton[13]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton15"));
	togglebutton[14]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton16"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		for (i=0;i<15;i++) {
			if (p_wid!=GTK_WIDGET(togglebutton[i])) gtk_toggle_button_set_active(togglebutton[i],FALSE);
		}
	}
}

void releaseToggleButton2 (GtkWidget * p_wid, gpointer p_data) {											// releases pressed toggle button in PWM USER SEQUENCE 
	SGlobalData *data =(SGlobalData*) p_data;																// when clicking the "clear" button.
	GtkToggleButton* togglebutton[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t i=0;
	togglebutton[0]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton2"));
	togglebutton[1]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton3"));
	togglebutton[2]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton4"));
	togglebutton[3]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton5"));
	togglebutton[4]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton6"));
	togglebutton[5]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton7"));
	togglebutton[6]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton8"));
	togglebutton[7]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton9"));
	togglebutton[8]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton10"));
	togglebutton[9]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton11"));
	togglebutton[10]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton12"));
	togglebutton[11]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton13"));
	togglebutton[12]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton14"));
	togglebutton[13]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton15"));
	togglebutton[14]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton16"));
	for (i=0;i<15;i++) {
		gtk_toggle_button_set_active(togglebutton[i],FALSE);
	}
}

void releaseToggleButton3 (GtkWidget * p_wid, gpointer p_data) {											// releases pressed toggle button in COMMANDS 
	SGlobalData *data =(SGlobalData*) p_data;																// when clicking another toggle button.
	GtkToggleButton* togglebutton[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t i=0;
	togglebutton[0]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton17"));
	togglebutton[1]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton18"));
	togglebutton[2]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton19"));
	togglebutton[3]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton20"));
	togglebutton[4]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton21"));
	togglebutton[5]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton22"));
	togglebutton[6]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton23"));
	togglebutton[7]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton24"));
	togglebutton[8]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton25"));
	togglebutton[9]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton26"));
	togglebutton[10]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton27"));
	togglebutton[11]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton28"));
	togglebutton[12]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton29"));
	togglebutton[13]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton30"));
	togglebutton[14]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton31"));
	togglebutton[15]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton1"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		for (i=0;i<16;i++) {
			if (p_wid!=GTK_WIDGET(togglebutton[i])) gtk_toggle_button_set_active(togglebutton[i],FALSE);
		}
	}
}

void releaseToggleButton4 (GtkWidget * p_wid, gpointer p_data) {											// releases pressed toggle button in COMMANDS 
	SGlobalData *data =(SGlobalData*) p_data;																// when clicking the "stop" button.
	GtkToggleButton* togglebutton[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t i=0;
	togglebutton[0]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton17"));
	togglebutton[1]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton18"));
	togglebutton[2]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton19"));
	togglebutton[3]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton20"));
	togglebutton[4]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton21"));
	togglebutton[5]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton22"));
	togglebutton[6]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton23"));
	togglebutton[7]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton24"));
	togglebutton[8]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton25"));
	togglebutton[9]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton26"));
	togglebutton[10]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton27"));
	togglebutton[11]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton28"));
	togglebutton[12]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton29"));
	togglebutton[13]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton30"));
	togglebutton[14]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton31"));
	togglebutton[15]=GTK_TOGGLE_BUTTON(gtk_builder_get_object(data->builder,"togglebutton1"));
	for (i=0;i<16;i++) {
		gtk_toggle_button_set_active(togglebutton[i],FALSE);
	}
}


void togglebutton2_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence a @0x04 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x04");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),1);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton3_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence b @0x14 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x14");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),2);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton4_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence c @0x24 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x24");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),3);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton5_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence d @0x34 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x34");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),4);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton6_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence e @0x44 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x44");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),5);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton7_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence f @0x54 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x54");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),6);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton8_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence g @0x64 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x64");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),7);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton9_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence h @0x74 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x74");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),8);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton10_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence i @0x84 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x84");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),9);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton11_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence j @0x94 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0x94");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),10);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton12_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence k @0xA4 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0xA4");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),11);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton13_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence l @0xB4 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0xB4");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),12);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton14_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence m @0xC4 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0xC4");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),13);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton15_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence n @0xD4 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0xD4");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),14);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}

void togglebutton16_toggled_cb (GtkWidget * p_wid, gpointer p_data) {										// select sequence o @0xE4 to store buffer.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=NULL;
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	label=GTK_LABEL(gtk_builder_get_object(data->builder,"label26"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"0xE4");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),15);
	}
	else {
		gtk_label_set_text(label,"0x00");
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxtext),0);
	}
}




void button35_clicked_cb (GtkWidget * p_wid, gpointer p_data) {												// get buffer from the selected sequence.
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkComboBoxText* comboboxtext=NULL;
	uint8_t myRXBuffer[256];
	uint8_t offset=0;
	uint8_t t[16][4]={"\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0"};
	uint8_t i=0;
	entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));					// the hidden combobox change when a toggle is pressed in PWM USER SEQUENCE
	offset=16*(gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxtext))-1)+4;									// Calculates offset from the hidden combobox.
	if (*(gint*)(data->user_data+8)==-1) {																	// If deamonized access mode (setupError==-1).
		* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),offset,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),16,&myRXBuffer[0],_DEXEC_COMPLETE);
	}
	else {
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),offset);								
		*(gint*)(data->user_data+20)+=CHAN_getBytes(*(gint*)(data->user_data+40),16,&myRXBuffer[0]);
	}
	for (i=0;i<16;i++) {
		sprintf((char*)t[i],"%.3d",myRXBuffer[i]);
		gtk_entry_set_text(entry[i],(gchar*)t[i]);
	}
}

void button36_clicked_cb (GtkWidget * p_wid, gpointer p_data) {												// Set buffer to the selected sequence.
	SGlobalData *data =(SGlobalData*) p_data;																
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkComboBoxText* comboboxtext=NULL;
	char* t[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	uint8_t myTXBuffer[17];
	uint8_t i=0;
	entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));					// The hidden combobox which change when a toggle is pressed in PWM USER SEQUENCE
	for (i=0;i<16;i++) {
		t[i]=(char*)gtk_entry_get_text(entry[i]);
		sscanf((const char*)t[i],"%d",(int*)&myTXBuffer[i]);
	}
	myTXBuffer[16]=16*(gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxtext))-1)+4;							// Calculates offset from the hidden combobox.
	if (*(gint*)(data->user_data+8)==-1) {																	// If deamonized access mode (setupError==-1).
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),myTXBuffer[16],_DEXEC_COMPLETE);	
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),16,myTXBuffer,_DEXEC_COMPLETE);		// Set buffer @ address of the selected user sequence.
			* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&myTXBuffer[16],_DEXEC_COMPLETE);	// Write PWM_SEQ_INDEX value @ 0x118.
		}
		else {																								// else direct access mode.
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),myTXBuffer[16]);
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),16,myTXBuffer);													// Set buffer @ address of the selected user sequence.
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),1,&myTXBuffer[16]);												// Write PWM_SEQ_INDEX value @ 0x118.
	}		
}

void togglebutton17_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence a 
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x04,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {											// If toggle is pressed
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {																// If deamonized access mode (setupError==-1).
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);	
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {																								// Else direct access mode.
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);									
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {																									// else toggle is released
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton18_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence b
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x14,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {											
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {																
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {																								
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {																									
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton19_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence c 
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x24,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {											
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {																
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}
	
void togglebutton20_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence d
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x34,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}	

void togglebutton21_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence e
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x44,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}	

void togglebutton22_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence f
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x54,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton23_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence g
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x64,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton24_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence h
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x74,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton25_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence i
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x84,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton26_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence j
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0x94,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton27_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence k
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0xA4,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton28_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence l
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0xB4,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton29_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence m
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0xC4,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton30_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence n
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0xD4,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

void togglebutton31_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// play sequence o
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t myTXBuffer[2]={0xE4,1};
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x118,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),2,myTXBuffer,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x118);										
			*(gint*)(data->user_data+20)+=CHAN_setBytes(*(gint*)(data->user_data+40),2,myTXBuffer);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
	}
}

gboolean togglebutton1_toggled_cb(GtkWidget * p_wid, gpointer p_data) {										// run PWM VALUES
	SGlobalData *data =(SGlobalData*) p_data;																// same as togglebutton17_toggled_cb
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	uint8_t source=0;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		if (*(gint*)(data->user_data+8)==-1) {
			*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x119,_DEXEC_COMPLETE);			
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&source,_DEXEC_COMPLETE);
			*(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x119);										
			*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),0);
			*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		}
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_command(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
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
		printf("error execv :");
		perror("execv");
		CHAN_close();
		exit(EXIT_FAILURE);
		
    }
}

void setCheckConf_cb2(GtkWidget * p_wid,gpointer p_data) {                                          // checkbutton "warn me ..." callback in startup popup window
	SGlobalData *data =(SGlobalData*) p_data;														// set suitable check_conf value which is written only if popup window response is OK.
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) *(gint*)(data->user_data)=1;		// see window1_realise_cb.
	else *(gint*)(data->user_data)=0;																
}
	
void setCheckConf_cb(GtkWidget * p_wid,gpointer p_data) {											// checkbox "warn me ...." callback in preferences popup window
	SGlobalData *data =(SGlobalData*) p_data;
	FILE* fd = NULL;
	uint8_t* ptr=NULL; 
	int8_t str[100]="";
	fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf", "rb+");											// parse common conf file to read check_conf parameter.
	if (fd != NULL) {
		do {
			fgets((char*)str, 100, fd);
		} while(strstr((const char*)str,"check_conf")==NULL);										
		fseek(fd,-strlen((const char*)str), SEEK_CUR);
		ptr=(uint8_t*)str;
		while (*ptr++!='=');
		while (*ptr==' ') ptr++;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {								// switch checkbox state 
			*ptr='1';																				// write suitable check_conf value in conf file
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

/*void setSetupError_cb2(GtkWidget * p_wid,gpointer p_data) {										// checkbox "warn me ...." callback in startup popup window
	SGlobalData *data =(SGlobalData*) p_data;														
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) *(gint*)(data->user_data+4)=0;		
	else *(gint*)(data->user_data+4)=-1;
}*/

void setSetupError_cb(GtkWidget * p_wid,gpointer p_data) {											// radio buttons callback in preferences popup window.
	SGlobalData *data =(SGlobalData*) p_data;
	FILE* fd = NULL;
	uint8_t* ptr=NULL; 
	int8_t str[255]="";
	int8_t str2[127]="";
	uint8_t i=0;
	fd = fopen("/usr/local/bin/chgtkt/chgtkt.conf","rb+");											// parse common conf file to read setup_error parameter.
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
			while(*(str2+i)) *ptr++=str2[i++];														// switch radio button states 
			*(gint*)(data->user_data+4)=0;															// write suitable setup_error value in conf file
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

void imagemenuitem12_activate_cb(GtkWidget * p_wid,gpointer p_data) {								// _Preferences button in menu bar to open preferences popup window
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWindow *parent=NULL;
	GtkWidget *dialog=NULL,*label=NULL,*checkButton=NULL,*contentArea=NULL,*radio1=NULL,*radio2=NULL;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gchar* message="settings \n";
	parent=GTK_WINDOW(gtk_builder_get_object(data->builder,"window1"));
	dialog = gtk_dialog_new_with_buttons ("chrtcf Preferences",parent,flags,("_CLOSE"),GTK_RESPONSE_NONE,NULL);
    contentArea = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	label = gtk_label_new (message);
	radio1 = gtk_radio_button_new_with_label(NULL,"Should use direct access mode.");
	radio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (radio1),"Should use daemonized access mode.");
	checkButton= gtk_check_button_new_with_label("Warn me at startup if current acces mode differs from predefined access mode in conf file.");
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
}

void checkmenuitem11_toggled_cb(GtkWidget * p_wid,gpointer p_data) {								// _chdaemon checkbox in menu bar to start/kill chdaemon	
	SGlobalData *data =(SGlobalData*) p_data;
	pid_t pid;
	int32_t setupError=255;
	int8_t mySlot=127;
	int32_t myId=_THIS_APP_ID;
	//char *arg[]={"sudo","chdaemon","-v","5","s",NULL};
	char *arg[]={"sudo","chdaemon",NULL};
	mySlot=*(gint*)(data->user_data+12);
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gtk_builder_get_object (data->builder, "checkmenuitem11")))) { // if checked start chdaemon.
		
		CHAN_close();
		usleep(1000000);
		pid=create_process();                            											//sudo chdaemon
		switch (pid) {
			case -1://..............................................(ENOMEM error)
				perror("fork");
				exit(0);
			break;
			case 0://...............................................child_process
				printf("Starting \"chdaemon\" (pid = %d) as child process running in background.\n",getpid());
				fflush(stdout);
				son_process(arg);
				
			break;
			default://..............................................father_process
				sleep(2);
				
				printf("Continuing main program (pid = %d) \n",getpid());
			break;
		}
		mySlot=CHAN_getDAEMONslot(myId);
		CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);		
		setupError=-1;
	}
	else {																													// else, unchecked, kill chdaemon.
		printf("Killing chdaemon...\n");
		CHAN_commandDAEMON(_DAEMON_SS_QUIT);
		while (CHAN_checkDAEMON(1)!=-1) {printf("*");fflush(stdout);}
		fflush(stdout);
		setupError=CHAN_setup("chgservo",1);
		switch (setupError) {
			case 0:																						 
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

/*void checkmenuitem11_toggled_cb2(GtkWidget * p_wid,gpointer p_data) {									// _ioDAEMON	
	SGlobalData *data =(SGlobalData*) p_data;
	pid_t pid;
	int32_t setupError=255;
	int8_t mySlot=127;
	int32_t myId=_THIS_APP_ID;
	char *arg[]={"sudo","chdaemon","-v","5","s",NULL};
	uint8_t i=1;
	mySlot=*(gint*)(data->user_data+12);
	printf("mySlot : %d\n",mySlot);
	printf("check.........................................................\n");
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gtk_builder_get_object (data->builder, "checkmenuitem11")))) {
		
		CHAN_close();
		usleep(1000000);
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
				
			break;
			default://..............................................father_process
				sleep(2);
				
				printf("Continuing main program (pid = %d) \n",getpid());
			break;
		}
		usleep(1000000);
		//system("sudo chdaemon -v 5 -s &");
		sleep(2);
		printf("check2....................................\n");
		fflush(stdout);
		while (CHAN_checkDAEMON(0)!=-1&&i++<10) {printf("+");fflush(stdout);}
		
		mySlot=CHAN_getDAEMONslot(myId);
		CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);		
		setupError=-1;
		printf("mySlot : %d\n",mySlot);
		fflush(stdout);
		//setupError=CHAN_setup("chgservo",1);
		//switch (setupError) {
		//	case 0:																						// CHAN_SETUP() is successfull 
		//		printf("Direct access mode is set.\n");
		//	break;
		//	case -1:
		//		myId=_THIS_APP_ID;
		//		mySlot=CHAN_getDAEMONslot(myId);
		//		printf("Slot %d is open.\n\n",mySlot);
		//		printf("Daemonized access mode is set.\n");
		//	break;
		//	case -2:																					// Other errors
		//		printf("Something wrong has happened, program will close now.\n");
		//		exit(EXIT_FAILURE);
		//	break;
		//	default :																					// The process, which PID is returned, is allready uses immediat mode.
		//		printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",setupError);
		//		exit(EXIT_SUCCESS);
		//	break;
		//}
	}
	else {
		//CHAN_releaseDAEMONslot(mySlot,myId);
		sleep(2);
		printf("asking 4 Killing chdaemon...\n");
		CHAN_commandDAEMON(_DAEMON_SS_QUIT);
		while (CHAN_checkDAEMON(1)!=0) {printf("*");fflush(stdout);}
		
		fflush(stdout);
		sleep(2);
		setupError=CHAN_setup("chgservo",1);
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
}*/

void menuitem1_activate_cb(GtkWidget * p_wid,gpointer p_data) {													// _File in menu 	
	SGlobalData *data =(SGlobalData*) p_data;
	GtkWidget* checkMenuItem=NULL;
	static uint32_t i=0;
	checkMenuItem=GTK_WIDGET(gtk_builder_get_object (data->builder, "checkmenuitem11"));
	if (*(gint*)(data->user_data+8)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkMenuItem),TRUE);	// checks if ioDAEMON is running, else uncheck. 
	if (!i++) g_signal_connect (checkMenuItem, "toggled", G_CALLBACK(checkmenuitem11_toggled_cb),data);			// connects signal first time only else callbacks 
}																												// are added each time and executed twice or more

void window1_realize_cb(GtkWidget * p_wid,gpointer p_data) {													// first call_back to initialize the app according 
	SGlobalData *data =(SGlobalData*) p_data;																	// to the conf file.
	GtkWindow *parent=NULL;
	GtkWidget *dialog=NULL,*label=NULL,*checkButton=NULL,*contentArea=NULL;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gchar* message2="According to \"chgtkt.conf\", this application should use direct access mode. Do you want to kill chdaemon now ?\n\0";
	gchar* message1="According to \"chgtkt.conf\", this application should use daemonized access mode. Do you want to start chdaemon now ?\n\0";
	int32_t result=0;
	uint8_t buf[100]={0};
	uint8_t i=0;
	uint8_t* ptr=NULL; 
	int8_t str[255]="";
	uint8_t savedC='\0';
	FILE* fd = NULL;
	int32_t setupError=255;																			// current access mode.
	int8_t mySlot=127;																				// current slot if deamonized access mode else set to 127.
	int32_t myId=_THIS_APP_ID;																		// THIS_APP_ID
	int32_t setupErrorC=127;																		// access mode in conf file.
	int32_t checkConf=127;																			// wether the app verify conf file or not.
	pid_t pid;
	char *arg[]={"sudo","chdaemon",NULL};
	setupError=CHAN_setup("chgservo",1);
	switch (setupError) {
		case 0:																						
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
		if (checkConf) {																			// "Warn me .... " is avalaible.
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
						setupError=-1;
						mySlot=CHAN_getDAEMONslot(myId);
						/*setupError=CHAN_setup("chgservo",1);
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
						}*/  
					}
				}
				else {																// start with daemonizedt access mode while direc access mode is awaited.
					label = gtk_label_new (message2);
					gtk_container_add (GTK_CONTAINER (contentArea), label);
					checkButton= gtk_check_button_new_with_label("Always ask at startup.");
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton),TRUE);
					gtk_container_add (GTK_CONTAINER (contentArea), checkButton);
					g_signal_connect(checkButton,"toggled", G_CALLBACK (setCheckConf_cb2),data);
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
						setupError=CHAN_setup("chgservo",1);
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
		//*(gint*)(data->user_data)=checkConf;
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
		else *(gint*)(data->user_data)=checkConf;
	}
	else exit(0);
}



void window1_realize_cb2(gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t myRXBuffer[256];
	GtkScale* scale[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	//GtkEntry* entry[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	//GtkCheckButton* checkbutton=NULL;
	
	//checkbutton=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	/*GtkRadioButton* radiobutton[2]={NULL,NULL};
	radiobutton[0]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton1"));
	radiobutton[1]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton2"));*/
	uint8_t t[3]="\0";
	/*entry[0]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry1"));
	entry[1]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry2"));
	entry[2]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry3"));
	entry[3]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry4"));
	entry[4]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry5"));
	entry[5]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry6"));
	entry[6]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry7"));
	entry[7]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry8"));
	entry[8]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry9"));
	entry[9]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry10"));
	entry[10]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry11"));
	entry[11]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry12"));
	entry[12]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry13"));
	entry[13]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry14"));
	entry[14]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry15"));
	entry[15]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry16"));
	entry[16]=GTK_ENTRY(gtk_builder_get_object(data->builder,"entry17"));  */        
	scale[0]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale1"));
	scale[1]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale2"));
	scale[2]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale3"));
	scale[3]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale4"));
	scale[4]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale5"));
	scale[5]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale6"));
	scale[6]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale7"));
	scale[7]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale8"));
	scale[8]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale9"));
	scale[9]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale10"));
	scale[10]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale11"));
	scale[11]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale12"));
	scale[12]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale13"));
	scale[13]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale14"));
	scale[14]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale15"));
	scale[15]=GTK_SCALE(gtk_builder_get_object(data->builder,"scale16"));
	if (*(gint*)(data->user_data+8)==-1) {	
		/*printf("ok**********************************\n");
		fflush(stdout);		
		printf(	"slot : %d\n",*(gint*)(data->user_data+12));
		printf(	"corenum : %d\n",*(gint*)(data->user_data+40));
		fflush(stdout);*/											// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x108,_DEXEC_COMPLETE);
		//*(gint*)(data->user_data+20)+=CHAND_addr(0,1,0x108,_DEXEC_COMPLETE);
		
		//printf("ok**********************************\n");
		//fflush(stdout);	
		*(gint*)(data->user_data+20)+=CHAND_getBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),18,&myRXBuffer[0],_DEXEC_COMPLETE);
		//printf("ok**********************************\n");
		//fflush(stdout);
	}
	else {
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x108);
		*(gint*)(data->user_data+20)+=CHAN_getBytes(*(gint*)(data->user_data+40),18,&myRXBuffer[0]);
	}
	gtk_range_set_value (GTK_RANGE(scale[0]),(gdouble)myRXBuffer[0]);
	gtk_range_set_value (GTK_RANGE(scale[1]),(gdouble)myRXBuffer[1]);
	gtk_range_set_value (GTK_RANGE(scale[2]),(gdouble)myRXBuffer[2]);
	gtk_range_set_value (GTK_RANGE(scale[3]),(gdouble)myRXBuffer[3]);
	gtk_range_set_value (GTK_RANGE(scale[4]),(gdouble)myRXBuffer[4]);
	gtk_range_set_value (GTK_RANGE(scale[5]),(gdouble)myRXBuffer[5]);
	gtk_range_set_value (GTK_RANGE(scale[6]),(gdouble)myRXBuffer[6]);
	gtk_range_set_value (GTK_RANGE(scale[7]),(gdouble)myRXBuffer[7]);
	gtk_range_set_value (GTK_RANGE(scale[8]),(gdouble)myRXBuffer[8]);
	gtk_range_set_value (GTK_RANGE(scale[9]),(gdouble)myRXBuffer[9]);
	gtk_range_set_value (GTK_RANGE(scale[10]),(gdouble)myRXBuffer[10]);
	gtk_range_set_value (GTK_RANGE(scale[11]),(gdouble)myRXBuffer[11]);
	gtk_range_set_value (GTK_RANGE(scale[12]),(gdouble)myRXBuffer[12]);
	gtk_range_set_value (GTK_RANGE(scale[13]),(gdouble)myRXBuffer[13]);
	gtk_range_set_value (GTK_RANGE(scale[14]),(gdouble)myRXBuffer[14]);
	gtk_range_set_value (GTK_RANGE(scale[15]),(gdouble)myRXBuffer[15]);
	sprintf((char*)t,"%02X",myRXBuffer[16]);
	//gtk_entry_set_text(entry[16],(const gchar*)t);
	//if (myRXBuffer[17]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),TRUE);
	/*if (!myRXBuffer[17]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton[0]),TRUE);
	else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton[1]),TRUE);*/
}
	
void comboboxtext1_changed_cb(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	char* line=NULL;
	gchar line2[100];
	uint8_t IO24num=0;
	GtkFrame* frame[4]={NULL,NULL,NULL,NULL};
	//GtkWindow* window=NULL;
	//window = GTK_WINDOW(gtk_builder_get_object (data->builder, "window1"));
	/*GtkRadioButton* radiobutton[2]={NULL,NULL};
	radiobutton[0]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton1"));
	radiobutton[1]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton2"));*/
	frame[0]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame1"));
	frame[1]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame2"));
	frame[2]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame3"));
	frame[3]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame4"));
	//sleep(1);
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(p_wid))) line=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(p_wid));
	//printf("line : %s\n",line);
	//while(*line++) printf(".");
	//printf("len : %d\n",strlen(line));
	if (line) {
		strcpy(line2,line);
		//line[4]='\0';
		//printf("line2 : %s\n",line);
		sscanf((const char*)line2,"%d",(gint*)&IO24num);
		//printf("IO24num=%d\n",IO24num);
		*(gint*)(data->user_data+40)=(gint)IO24num;
		//printf("*(gint*)(data->user_data+40)=%d\n",*(gint*)(data->user_data+40));
		//usleep(500000);
		window1_realize_cb2(data);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[0]),TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[1]),TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[2]),TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[3]),TRUE);
		//gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[0]),TRUE);
		//gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[1]),TRUE);
		
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(frame[0]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[1]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[2]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[3]),FALSE);
		//gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[0]),FALSE);
		//gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[1]),FALSE);
	}
}

void window1_realize_cb3(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	bus_struct chantilly_stack;
	//GtkWindow* window=NULL;
	//window = GTK_WINDOW(gtk_builder_get_object (data->builder, "window1"));
	GtkComboBoxText* comboboxtext=NULL;
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext1"));
	
	uint8_t m=0;
	gchar titles[100]="";
	gchar lines[7][100]={"","","","","","",""};
	/*printf("Core number | Core firmware.access version\n");
	for (m=1;m<8;m++) {
		printf("tab[%d]=%s\n",m,&lines[m][0]);
	}*/
	
	
	
	//sprintf(titles,"%3s\t%3s-%3s","num","gfw","ffw");
	sprintf(titles,"%s","select");
	
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboboxtext), NULL, titles);
	if (*(gint*)(data->user_data+8)==-1) {
		for (m=1;m<8;m++) {
			//CHAND_getCOREINFO(i,&myCORES[m][0]);
			CHAND_checkBUS(&chantilly_stack,m);
			if (chantilly_stack.core_id[m]) {	
				printf("| \t %d| \t %d| \t %d| \t %d|\n",m,chantilly_stack.core_id[m],chantilly_stack.core_access_v[m],chantilly_stack.core_firmware_v[m]);
				//sprintf(lines[m],"%03d\t\t%.3d-%.3d",m,chantilly_stack.core_access_v[m],chantilly_stack.core_firmware_v[m]);
				sprintf(lines[m],"%03d",m);
				
				if (chantilly_stack.core_id[m]==0x10) gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboboxtext), NULL, lines[m]);
			}
		}
	}
	else {
		for (m=1;m<8;m++) {
			CHAN_checkBUS(&chantilly_stack,m,0);
			if (chantilly_stack.core_id[m]) {	
				//printf("| \t %d| \t %d| \t %d| \t %d|\n",m,chantilly_stack.core_id[m],chantilly_stack.core_access_v[m],chantilly_stack.core_firmware_v[m]);
				//sprintf(lines[m],"%03d\t\t%.3d-%.3d",m,chantilly_stack.core_access_v[m],chantilly_stack.core_firmware_v[m]);
				sprintf(lines[m],"%03d",m);
				if (chantilly_stack.core_id[m]==0x10) gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboboxtext), NULL, lines[m]);
			}
		}
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX(comboboxtext),0);
    //g_signal_connect(comboboxtext,"changed",G_CALLBACK(comboboxtext1_changed_cb),data);                    
	//window1_realize_cb2(data);
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
	//int32_t errSetup=3;
	//int8_t mySlot=-1;
	//int32_t myId=6;
	//uint32_t chanErrors=0;
	//gint func_ref =0;
	//gint func_ref2 =0;
	
	//myDatas[0]=checkConf;
	//myDatas[1]=setupErrorConf;
	//myDatas[2]=setupError;
	//myDatas[3]=(gint)mySlot;                                                       							
	//myDatas[4]=myId;
	//myDatas[5]=(gint)chanErrors;																	
	//myDatas[6]=NULL;
	//myDatas[7]=timer_statusbar;
	//myDatas[8]=NULL;																				
	//myDatas[9]=timer_statusbar_is_running;
	//myDatas[10]=IO24num;
	myDatas[5]=0;
	myDatas[6]=0;	
	myDatas[7]=0;
	myDatas[8]=0;
	myDatas[9]=0;
	myDatas[10]=0;
	data.user_data=myDatas;
	gtk_init(&argc, &argv);
	data.builder = gtk_builder_new();
	filename =  g_build_filename ("./chgservo.glade", NULL);
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
	/*errSetup=CHAN_setup("chservo",1);
	switch (errSetup) {
		case 0:																						// CHAN_SETUP() is successfull 
			printf("Direct access mode is set.\n");
		break;
		case -1:
			//myId=_THIS_APP_ID;
			//mySlot=CHAN_getDAEMONslot(myId);
			printf("Slot %d is open.\n\n",mySlot);
			printf("Deamonized access mode is set.\n");
		break;
		case -2:																					// Other errors
			printf("Something wrong has happened, program will close now.\n");
			exit(EXIT_FAILURE);
		break;
		default :																					// The process, which PID is returned, is allready uses immediat mode.
			printf("Process with PID=%d is using chantilly in direct mode, please kill it before running this program.\n",errSetup);
			exit(EXIT_SUCCESS);
		break;
	}
	myDatas[0]=errSetup;
	myDatas[1]=(gint)mySlot;
	myDatas[2]=(gint)myId;
	//myDatas[3]=1;                                                       							// Timer get_RTC or get_RTC_I updates display.
	//myDatas[4]=func_ref2;																			// Timer executed once when day_selected by simple clic.
	myDatas[5]=(gint)chanErrors;																	// Chantilly errors.
	myDatas[6]=0;																					// Timeout "any" checkboxes in statusBar, temp message.
	data.user_data=myDatas;
	printf("Setup error : %d\n",*((gint*)data.user_data));
	printf("slot : %d\n",*((gint*)data.user_data+1));
	printf("Id : %d\n",*((gint*)data.user_data+2));
	fflush(stdout);
	gtk_init(&argc, &argv);
	//gdk_init(&argc, &argv);
	data.builder = gtk_builder_new();
	filename =  g_build_filename ("/home/pi/code_004/TESTS/chservo/chservo.glade", NULL);
	gtk_builder_add_from_file(data.builder,filename,&error);
	g_free (filename);
	if (error) 
	{ 
		gint code = error->code; 
		g_printerr("%s\n", error->message); 
		g_error_free (error); 
		return code; 
	} 
	window = GTK_WIDGET(gtk_builder_get_object (data.builder, "window1"));
	gtk_builder_connect_signals (data.builder,&data);
	g_signal_connect (G_OBJECT (window), "destroy", (GCallback)gtk_main_quit, NULL);
	//if (mySlot!=-1) func_ref = g_timeout_add (1000, getRTCDatetime_de, &data);
	//else {
	//	func_ref = g_timeout_add (1000, getRTCDatetime_di, &data);
	//}
	gtk_widget_show_all(window);
	gtk_main();
	//if (mySlot!=-1) CHAN_releaseDAEMONslot(mySlot,myId);
	//else CHAN_close();
	CHAN_close();
	//g_source_remove (func_ref);
	printf("\naccess errors : %d\n\n",(uint32_t)myDatas[5]);		// Displays access errors (optionnal for debug).
	fflush(stdout);
	return 0;
}*/
