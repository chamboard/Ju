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
	gchar* message="PWM VALUES : \n\tUse \" PWMn\" scale to adjust PWMn_VAL assuming that PWMn is the PWM applied on OutputA bitn if 0≤n≤7 or OutputB bitn if 8≤n≤15\n\t(PWMn_VAL are bytes at addresses in the range from 0x108 to 0x117 in IO24 RAM).\n\nPWM USER SEQUENCE :\n\tFirst, copy PWMn_VAL to buffer by clicking on \"Copy\" button.\n\tThen, copy buffer to IO24 free RAM by clicking on \"Save in free RAM\" button which is enabled only if address in \"@ 0x\" entry is between 0x04 and 0xF0. \n\tSo choose between PWM values or user PWM sequence to use with servo mode by checking/unchecking the checkbox.\n\nCOMMAND : \n\tWhen \"RUN\" toggle button is pressed, execute IO24 command 0x05 named SERVO_RUN : Starts IO24 (immediate inputs and) servo outputs mode.\n\tWhen \"RUN\" toggle button is released, execute IO24 command 0x006 named STOP : Stops IO24 running mode";
	parent=GTK_WINDOW(gtk_builder_get_object(data->builder,"window1"));
	//dialog = gtk_dialog_new_with_buttons ("CRTCM Help",parent,flags,("_OK"),GTK_RESPONSE_ACCEPT,("_Cancel"),GTK_RESPONSE_REJECT,NULL);
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

void expander1_activate_cb(GtkWidget * p_wid, gpointer p_data) {
	if (!gtk_expander_get_expanded (GTK_EXPANDER(p_wid)))	gtk_expander_set_label(GTK_EXPANDER(p_wid),"hide");
	else gtk_expander_set_label(GTK_EXPANDER(p_wid),"expand");
	
	
}
/*gboolean checkInput3(GtkWidget * p_wid, gpointer p_data) {      										// Entries editing.
	const gchar* ptr=NULL;
	uint8_t str[4]="aaa\0";
	char n=0;
	uint8_t value=0;
	ptr=gtk_entry_get_text(GTK_ENTRY(p_wid));
	printf("1\t");
	printf("str : %s\n",str);
	printf("\tptr : %s\n",ptr);
	fflush(stdout);
	do 
	{
		printf("2\t");
		printf("str : %s\n",str);
		printf("\tptr : %s\n",ptr);
		fflush(stdout);
		fflush(stdout);
		str[n]=*ptr;
		if (*ptr<48||57<*ptr) {
			str[n]='\0';
			
			printf("3\t");
			printf("str : %s\n",str);
			printf("\tptr : %s\n",ptr);
			fflush(stdout);
			fflush(stdout);
			
		}
		
		printf("4\t");
		printf("str : %s\n",str);
		printf("\tptr : %s\n",ptr);
		fflush(stdout);
		//sscanf(str,"%03d",(int*)&value);
		//sprintf(str,"%.3d",value);
		
		ptr++;n++;
	} while (*ptr);
	sscanf((const char*)str,"%d",(int*)&value);
	sprintf((char*)str,"%.3d",value);
	printf("5\t");
	printf("str : %s\n",str);
	printf("\tptr : %s\n\n",ptr);
	fflush(stdout);
	gtk_entry_set_text(GTK_ENTRY(p_wid),(gchar*)str);
	return FALSE;
}*/


/*gboolean checkInput2(GtkWidget * p_wid, gpointer p_data) {      										// Entries editing.
	const gchar* ptr=NULL;
	uint8_t str[4]="000\0";
	char n=0;
	uint8_t value=0;
	ptr=gtk_entry_get_text(GTK_ENTRY(p_wid));
	printf("1\n");
	printf("ptr : %s\n\n",ptr);
	fflush(stdout);
	do 
	{
		printf("2\n");
		fflush(stdout);
		str[n]=*ptr;
		if (*ptr<48||57<*ptr) {
			str[n]='\0';
			
			printf("3\n");
			//printf("text : %s\n",str);
			fflush(stdout);
			
		}
		sscanf((const char*)str,"%d",(int*)&value);
		sprintf((char*)str,"%.3d",value);
		
		printf("str : %s\n\n",str);
		fflush(stdout);
		//sscanf(str,"%03d",(int*)&value);
		//sprintf(str,"%.3d",value);
		
		ptr++;n++;
	} while (*ptr);
	gtk_entry_set_text(GTK_ENTRY(p_wid),(gchar*)str);
	return FALSE;
}*/

gboolean scale16_value_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	gdouble value=0;
	uint8_t value2=0;
	value=gtk_range_get_value(GTK_RANGE(p_wid));
	value2=(uint8_t)value;
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x116,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x115,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x114,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x113,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x112,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x111,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x110,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10F,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10E,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10D,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10C,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10B,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x10A,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
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
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x109,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else { 
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
	if (*(gint*)(data->user_data+8)==-1) {		// If deamonized access mode (mySlot!=-1).
		*(gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x108,_DEXEC_COMPLETE);
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,(uint8_t*)&value2,_DEXEC_COMPLETE);
	}
	else {
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x108);
		*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),(uint8_t)value);
	}
	return TRUE;
}

gboolean button1_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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

gboolean button2_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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

gboolean button3_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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

gboolean button4_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button5_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button6_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button7_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button8_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button9_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button10_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button11_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button12_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button13_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button14_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button15_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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
gboolean button16_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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


gboolean entries_focus_out_event_cb(GtkWidget * p_wid,GdkEvent  *event, gpointer p_data) {
	//SGlobalData *data =(SGlobalData*) p_data;
	const int8_t* ptr=(int8_t*)"eee\0";
	int8_t str[4]="eee\0";
	uint8_t value=0;
	ptr=(const int8_t*)gtk_entry_get_text(GTK_ENTRY(p_wid));
	sscanf((const char*)ptr,"%d",(int*)&value);
	sprintf((char*)str,"%.3d",value);
	//printf("str : %s\n",str);
	gtk_entry_set_text(GTK_ENTRY(p_wid),(const gchar*)str);
	return FALSE;
}






gboolean button17_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
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

gboolean button108_clicked_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	char* t[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
	//char t2[]=""; 
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
}


/*gboolean comboboxtext2_changed_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
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
	printf("active : %d\n",16*gtk_combo_box_get_active(GTK_COMBO_BOX(p_wid))+4);
	return TRUE;
}*/

void button35_clicked_cb (GtkWidget * p_wid, gpointer p_data) {
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
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	offset=16*gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxtext))+4;
	//printf("offset = %d\n",offset);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),offset,_DEXEC_COMPLETE);			//
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),16,&myRXBuffer[0],_DEXEC_COMPLETE);// Write PWM_SOURCE value @ 0x119.
	}
	else {
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),offset);										//
		*(gint*)(data->user_data+20)+=CHAN_getBytes(*(gint*)(data->user_data+40),16,&myRXBuffer[0]);
	}
	for (i=0;i<16;i++) {
		sprintf((char*)t[i],"%.3d",myRXBuffer[i]);
		gtk_entry_set_text(entry[i],(gchar*)t[i]);
	
	}
}

void button36_clicked_cb (GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	GtkComboBoxText* comboboxtext=NULL;
	//GtkEntry* entry[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	char* t[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
	//char t2[]=""; 
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
	comboboxtext=GTK_COMBO_BOX_TEXT(gtk_builder_get_object(data->builder,"comboboxtext2"));
	
	for (i=0;i<16;i++) {
		t[i]=(char*)gtk_entry_get_text(entry[i]);

		sscanf((const char*)t[i],"%d",(int*)&myTXBuffer[i]);
		printf("myTXBuffer [%d]=%d\n",i,myTXBuffer[i]);
	}
	myTXBuffer[16]=16*gtk_combo_box_get_active(GTK_COMBO_BOX(comboboxtext))+4;
	printf("offset : %d\n",myTXBuffer[16]);
	//t[16]=(char*)gtk_entry_get_text(entry[16]);
	//strcpy(t2,t[16]);
	//sscanf((const char*)t[16],"%x",(int*)&myTXBuffer[16]);
	printf("offset : %d\n",myTXBuffer[16]);
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
}

gboolean entry17_changed_cb(GtkWidget * p_wid, gpointer p_data) {      										// Entries editing.
	SGlobalData *data =(SGlobalData*) p_data;
	const gchar* ptr=NULL;
	gchar str[3]="\0";
	char n=0;
	GtkButton* button=NULL;
	uint8_t value=0;
	uint8_t myRXBuffer[256];
	GtkEntry* entry[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
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
	button=GTK_BUTTON(gtk_builder_get_object(data->builder,"button108"));
	ptr=gtk_entry_get_text(GTK_ENTRY(p_wid));
	do 
	{
		str[n]=*ptr;
		if (*ptr<48||(57<*ptr&&*ptr<65)||(70<*ptr&&*ptr<97)||102<*ptr) {
			str[n]='\0';
			gtk_entry_set_text(GTK_ENTRY(p_wid),str);
		}
		ptr++;n++;
	} while (*ptr);
	sscanf((const char*)str,"%2X",(unsigned int*)&value);
	if (0x04<=value&&value<=0xF0) {
		gtk_widget_set_sensitive(GTK_WIDGET(button),TRUE);
		
	
	
	}
	else gtk_widget_set_sensitive(GTK_WIDGET(button),FALSE);
	if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
		* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),value,_DEXEC_COMPLETE);			//
		*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),16,&myRXBuffer[0],_DEXEC_COMPLETE);// Write PWM_SOURCE value @ 0x119.
	}
	else {
		*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),value);										//
		*(gint*)(data->user_data+20)+=CHAN_getBytes(*(gint*)(data->user_data+40),16,&myRXBuffer[0]);
	}
	for (i=0;i<16;i++) {
		sprintf((char*)t[i],"%.3d",myRXBuffer[i]);
		gtk_entry_set_text(entry[i],(gchar*)t[i]);
	
	}
	return FALSE;
}
gboolean radiobutton1_toggled_cb(GtkWidget * p_wid, gpointer p_data) {
//gboolean checkbutton1_toggled_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t source=0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p_wid))) {
		source=0;
		if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
			* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x119,_DEXEC_COMPLETE);			//
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&source,_DEXEC_COMPLETE);// Write PWM_SOURCE value @ 0x119.
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x119);										//
			*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),source);										// Write PWM_SOURCE value @ 0x119.
		}		
	}
	else {
		source=1;
		if (*(gint*)(data->user_data+8)==-1) {															// If deamonized access mode (mySlot!=-1).
			* (gint*)(data->user_data+20)+=CHAND_addr(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x119,_DEXEC_COMPLETE);			//
			*(gint*)(data->user_data+20)+=CHAND_setBytes(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),1,&source,_DEXEC_COMPLETE);// Write PWM_SOURCE value @ 0x119.
		}
		else {
			*(gint*)(data->user_data+20)+=CHAN_addr(*(gint*)(data->user_data+40),0x119);										//
			*(gint*)(data->user_data+20)+=CHAN_setByte(*(gint*)(data->user_data+40),source);										// Write PWM_SOURCE value @ 0x119.
		}		
	}
	
	printf("source : %d\n",source);
	fflush(stdout);
	
	
	return TRUE;
}

gboolean togglebutton1_toggled_cb(GtkWidget * p_wid, gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	GtkLabel* label=GTK_LABEL(gtk_builder_get_object(data->builder,"label2"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p_wid))) {
		gtk_label_set_text(label,"Servos are ON.");
		//if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_doCOMMAND(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x05,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		//else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
		*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x05);
	}
	else {
		gtk_label_set_text(label,"Servos are OFF.");
		//if (*(gint*)(data->user_data+8)==-1) *(gint*)(data->user_data+20)+=CHAND_doCOMMAND(*(gint*)(data->user_data+12),*(gint*)(data->user_data+40),0x01,_DEXEC_COMPLETE);			// If deamonized access mode (mySlot!=-1).
		//else *(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x01);
		*(gint*)(data->user_data+20)+=CHAN_command(*(gint*)(data->user_data+40),0x06);
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
	printf("coucou\n");
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
	//char *arg[]={"sudo","chdaemon","-v","5","s",NULL};
	char *arg[]={"sudo","chdaemon",NULL};
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
		//usleep(1000000);
		//system("sudo chdaemon -v 5 -s &");
		//sleep(2);
		//printf("check2....................................\n");
		fflush(stdout);
		//while (CHAN_checkDAEMON(0)!=-1&&i++<10) {printf("+");fflush(stdout);}
		
		mySlot=CHAN_getDAEMONslot(myId);
		CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);		
		setupError=-1;
		printf("mySlot : %d\n",mySlot);
		fflush(stdout);
		/*setupError=CHAN_setup("chgservo",1);
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
		sleep(2);
		printf("asking 4 Killing chdaemon...\n");
		CHAN_commandDAEMON(_DAEMON_SS_QUIT);
		//system("sudo ipcrm -M 0x1973");
		while (CHAN_checkDAEMON(1)!=-1) {printf("*");fflush(stdout);}
		
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
	if (!i++) g_signal_connect (checkMenuItem, "toggled", G_CALLBACK(checkmenuitem11_toggled_cb),data);			// connects signal first time
}

void window1_realize_cb(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
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
	int32_t setupError=255;
	int8_t mySlot=127;
	int32_t myId=_THIS_APP_ID;
	int32_t setupErrorC=127;
	int32_t checkConf=127;
	pid_t pid;
	char *arg[]={"sudo","chdaemon",NULL};
	setupError=CHAN_setup("chgservo",1);
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
					g_signal_connect(checkButton,"toggled", G_CALLBACK (setCheckConf_cb),data);
					gtk_widget_show_all (dialog);
					result=gtk_dialog_run(GTK_DIALOG(dialog));
					fflush(stdout);
					if (result==GTK_RESPONSE_OK) {
						CHAN_releaseDAEMONslot(mySlot,myId);
						sleep(1);
						CHAN_commandDAEMON(_DAEMON_SS_QUIT);
						printf("Killing chdaemon...\n");
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



void window1_realize_cb2(gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	uint8_t myRXBuffer[256];
	GtkScale* scale[16]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	//GtkEntry* entry[17]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	//GtkCheckButton* checkbutton=NULL;
	
	//checkbutton=GTK_CHECK_BUTTON(gtk_builder_get_object(data->builder,"checkbutton1"));
	GtkRadioButton* radiobutton[2]={NULL,NULL};
	radiobutton[0]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton1"));
	radiobutton[1]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton2"));
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
	if (!myRXBuffer[17]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton[0]),TRUE);
	else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiobutton[1]),TRUE);
}
	
void comboboxtext1_changed_cb(GtkWidget * p_wid,gpointer p_data) {
	SGlobalData *data =(SGlobalData*) p_data;
	char* line=NULL;
	gchar line2[100];
	uint8_t IO24num=0;
	GtkFrame* frame[3]={NULL,NULL,NULL};
	//GtkWindow* window=NULL;
	//window = GTK_WINDOW(gtk_builder_get_object (data->builder, "window1"));
	GtkRadioButton* radiobutton[2]={NULL,NULL};
	radiobutton[0]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton1"));
	radiobutton[1]=GTK_RADIO_BUTTON(gtk_builder_get_object(data->builder,"radiobutton2"));
	frame[0]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame1"));
	frame[1]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame2"));
	frame[2]=GTK_FRAME(gtk_builder_get_object (data->builder, "frame3"));
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
		gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[0]),TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[1]),TRUE);
		
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(frame[0]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[1]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(frame[2]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[0]),FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(radiobutton[1]),FALSE);
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
