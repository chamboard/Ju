uint8_t* strtoupper(uint8_t *s) {
	uint8_t* p=s;
    while (*s) {
        if ((*s>='a')&&(*s<='z')) *s-=('a'-'A');
        s++;
    }
    return p;
}
//
uint8_t *str_sub(uint8_t *s,unsigned int start,unsigned int end) {
   uint8_t *new_s=NULL;
   if (s!=NULL&&start<end)
   {
      new_s=malloc(sizeof(*new_s)*(end-start+2));
      if (new_s!=NULL)
      {
         int i;
         for (i=start;i<=end;i++)
         {
            new_s[i-start]=s[i];
         }
         new_s[i-start]='\0';
      }
      else
      {
         fprintf(stderr,"Memoire insuffisante\n");
         CHAN_close();
         exit(EXIT_FAILURE);
      }
   }
   return new_s;
}
//
void parse_core_list() {
	uint8_t buf[1000]={0};
	uint8_t txt[7][40]={"\0"};
	uint8_t txt2[7][40]={"\0"};
	uint8_t key[7][10]={"\0"};
	uint8_t val[7][20]={"\0"};
	uint8_t i=0,j=0,k=0,l=0;
	uint8_t last_j;
	uint8_t * str=NULL;
	FILE* fd=NULL;
	int8_t nb=0;
	fd=fopen("/home/pi/code_004/common/CHAN/core_list.h","r+");
	//fd=fopen("./firmwares/core_list.h","r+");
	if (fd!=NULL) {
		nb=fread(buf,sizeof(char),1000,fd);
		fclose(fd);
		i=0;
		j=0;
		k=0;
		while(1) {
			if (i<nb) {
				k=i;
				while (buf[(i++)]!=0xA) continue;
				if (k<i-2) {
					str=str_sub(buf,k,i-2);           					// set to i-2 to prevent carriage return to be read
					strcat((char*)str,"\0");
					strcpy((char*)&txt[j][0],(char*)str);
				}
			}
			else {break;}
			j++;
		}
		last_j=j+1;
		if (debugLevel&0x01) {
			printf("total lines J = %d\n",last_j);
			fflush(stdout);
		}
		l=0;
		for (j=0;j<last_j;j++){
			if (strstr((char*)&txt[j][0],"#define")) {
				strcpy((char*)&txt2[l][0],(char*)&txt[j][0]);
				l++;
			}
		}
		last_j=l;
		if (debugLevel&0x01) {
			for (i=0;i<l;i++) printf("txt2[%d] = %s \n",i,txt2[i]);
		}
		for (i=0;i<l;i++)
		{
			j=0;k=0;
			while ((txt2[i][(j)]!=' '&&txt2[i][(j)]!='\t')&&j<32) j++; 	//find '#'
			k=j;
			j+=1;
			while ((txt2[i][(j)]!=' '&&txt2[i][(j)]!='\t')&&j<32) j++;	//find end of define
			str=str_sub(&txt2[i][0],k,j);            // 
			strcpy((char*)&key[i][0],(char*)str);
			j+=1;
			while ((txt2[i][(j)]==' '||txt2[i][(j)]=='\t')&&j<32) j++;
			k=j-1;
			j+=3;
			while((txt2[i][j]!='"')&&j<32) j++;
			str=str_sub(&txt2[i][0],k+1,j+1);
			strcpy((char*)&val[i][0],(char*)str);
			sprintf((char*)&CORE_NAME[i][0],"%s",str_sub(val[i],1,strlen((const char*)val[i])-2));
			str=str_sub(key[i],2,strlen((const char*)key[i]));
			CORE_ID[i]=(uint8_t)strtol((const char*)str,NULL,0);
		}
		if (debugLevel&0x01) {
			for (j=0;j<last_j;j++)	{printf("key %d: %s \t" ,j,&key[j][0]);printf("val %d: %s \n",j,&val[j][0]);}
			for (j=0;j<8;j++) {printf("CORE_NAME[%d] :%s \n",j,CORE_NAME[j]);printf("len : %d \n",strlen((const char*)CORE_NAME[j]));}	
			for (j=0;j<8;j++) {printf("CORE_ID[%d] :%d \n",j,CORE_ID[j]);}
		}
	}
	else {
		printf("ERROR : can't open core_list.h. \n");
		CHAN_close();
		exit(EXIT_FAILURE);
	}
}
//
void parse_coredef(uint8_t* id, pcui* ppp){
	uint8_t coredef_path[256]="\0";
	uint8_t buf[40000]={0};
	uint8_t txt[1000][200]={"\0"};
	uint8_t txt2[1000][200]={"\0"};
	uint8_t key[800][50]={"\0"};
	uint8_t val[800][150]={"\0"};
	uint16_t i=0, j=0, k=0,l=0;
	uint16_t last_j;
	int countRadio =0;
	uint8_t * str=NULL;
	uint8_t str1[150];
	char * ptr =NULL;
	//uint8_t update=0;
	//pid_t pid;
	//char *arg[]={"sudo","coresdefs_installer.sh",NULL};
	sprintf((char*)coredef_path,_COREDEF_PATH,id);
	printf("\nParsing file : %s...\n",coredef_path);
	FILE* fd = NULL;
    fd = fopen((char*)coredef_path, "r+");
    if (fd != NULL)
    {
		int nb=fread(buf,sizeof(char),40000,fd);
		fclose(fd);
		//fprintf(stderr, "freadg returned %d\n", nb);
		memset(txt,0,(500*200)*sizeof(char));
		i=0;
		j=0;
		/*while (1)                                 						//1st loop : all lines of config_*.h are filled in txt[]//////////
		{
			if (i<nb) {
				k=i;
				while (buf[(i++)]!=0xA) continue;
				if (k<i-2) 
				{
					str=str_sub(buf,k,i-2);            					// set to i-2 to prevent carriage return to be read
					strcat((char*)str,"\0");
					strcpy((char*)&txt[j][0],(char*)str);
				}
			}
			else {break;}
			j++;
		}*/
		do {
			k=i;
			while (buf[(i++)]!=0xA) continue;
			if (k<i-2) 
			{
				str=str_sub(buf,k,i-2);            					// set to i-2 to prevent carriage return to be read
				strcat((char*)str,"\0");
				strcpy((char*)&txt[j++][0],(char*)str);
			}
		} while (i<nb);
		last_j=j+1;
		if (debugLevel&0x08) {
			printf("total lines J=%d \n",last_j);
			fflush(stdout);
		}
		l=0;
		for (j=0;j<last_j;j++) {
			if (strstr((char*)&txt[j][0],"#define")) {
				strcpy((char*)&txt2[l][0],(char*)&txt[j][0]);
				l++;
			}
		}
		last_j=l;
		if (debugLevel&0x08) {
			for (i=0;i<l;i++) printf("txt[%d] = %s \n",i,txt[i]);
		}
		for (i=0;i<l;i++)
		{
			j=0;k=0;
			while ((txt2[i][(j)]!=' '&&txt2[i][(j)]!='\t')&&j<200) j++; //find '#'
			k=j;
			j+=1;
			while ((txt2[i][(j)]!=' '&&txt2[i][(j)]!='\t')&&j<200) j++; //find end of define
			str=str_sub(&txt2[i][0],k,j);            // 
			strcat((char*)str,"\0");
			strcpy((char*)&key[i][0],(char*)str);
			j+=1;
			while ((txt2[i][(j)]==' ' || txt2[i][(j)]=='\t' )&&j<200) j++;
			k=j-1;
			j+=1;
			while ((txt2[i][(j)]!='\t'&&txt2[i][(j)]!=0xA&&txt2[i][(j)]!='/')&&j<200) j++;
			str=str_sub(&txt2[i][0],k+1,j-1);
			strcat((char*)str,"\0");
			strcpy((char*)&val[i][0],(char*)str);
			j+=1;
		}
		if (debugLevel&0x08) {
			for (j=0;j<last_j;j++)	{printf("key %d: %s \t",j,&key[j][0]);printf("val %d: %s \n",j,&val[j][0]);}
		}
		j=0;
		for (i=0;i<last_j;i++)
		{
			if (strstr((char*)&key[i][0],"_L ")||strstr((char*)&key[i][0],"_L\t")) {
				sscanf((char*)val[i],"%X",(unsigned int*)&j);j-=256;
				ppp->type[j]=1;
				ppp->type[j+1]=1;
				strcpy((char*)&ppp->label[j][0],(char*)val[i+2]);		/////??????????????????????????????????
				while((ptr=strpbrk((char*)&ppp->label[j],"\"")))		
				{ 
					*ptr=' '; 											
				}
				strcpy((char*)ppp->label[j+1],(const char*)ppp->label[j]);
				sscanf((char*)val[i],"%X",(unsigned int*)&(ppp->addr0[j]));
				ppp->addr1[j+1]=ppp->addr0[j];
				sscanf((char*)val[i+1],"%x",(unsigned int*)&(ppp->addr0[j+1]));
				ppp->addr1[j]=ppp->addr0[j+1];
				sscanf((char*)val[i+3],"%s",(char*)&(ppp->format[j]));
				strcpy((char*)ppp->format[j+1],(const char*)ppp->format[j]);
				sscanf((char*)val[i+4],"%f",&(ppp->gain[j]));
				ppp->gain[j+1]=ppp->gain[j];
				sscanf((char*)val[i+5],"%f",&(ppp->step[j]));
				ppp->step[j+1]=ppp->step[j];
				sscanf((char*)val[i+6],"%f",&(ppp->high_limit[j]));
				ppp->high_limit[j+1]=ppp->high_limit[j];
				sscanf((char*)val[i+7],"%f",&(ppp->low_limit[j]));
				ppp->low_limit[j+1]=ppp->low_limit[j];
				sscanf((char*)val[i+8],"%s",(char*)&(ppp->onlyRead[j]));
				strcpy((char*)ppp->onlyRead[j+1],(const char*)ppp->onlyRead[j]);
				sscanf((char*)val[i+9],"%x",(unsigned int*)&ppp->eepromOffset[j]);
				if (ppp->eepromOffset[j]!=255) ppp->eepromOffset[j+1]=ppp->eepromOffset[j];
				sscanf((char*)key[i],"%100s",(char*)&ppp->txt0[j]);
				strcpy((char*)&ppp->txt1[j+1][0],(const char*)&ppp->txt0[j][0]);
				sscanf((char*)key[i+1],"%100s",(char*)&(ppp->txt0[j+1]));
				strcpy((char*)ppp->txt1[j],(const char*)ppp->txt0[j+1]);
				i+=10;
			}
			else if (strstr((char*)&key[i][0],"_R ")||strstr((char*)&key[i][0],"_R\t")) {
				sscanf((char*)val[i],"%x",(unsigned int*)&j);
				j-=256;
				ppp->type[j]=2;
				strcpy((char*)&ppp->label[j][0],(char*)val[i+1]);/////??????????????????????????????????
				while((ptr=strpbrk((char*)&ppp->label[j],"\"")))		//
				{ 
					*ptr=' '; 					//
				}
				sscanf((char*)&val[i+2][0],"%d",(int*)&ppp->count_params[j]);
				k=0;
				for (k=0;k<ppp->count_params[j];k++)
				{
					strcpy((char*)str1,(char*)val[i+3+ppp->count_params[j]+k]);
					while((ptr = strpbrk((char*)str1,"\"")))		//find the first tab in input
					{ 
						*ptr=' '; 					//replace the tab with a emty-char to make the string filename of config_*.h
					}
					strcpy((char*)ppp->params[j][k],(char*)str1);	
				}
				sscanf((char*)val[i],"%x",(unsigned int*)&(ppp->addr0[j]));
				sscanf((char*)key[i],"%100s",(char*)&(ppp->txt0[j]));
				sscanf((char*)val[i+2*ppp->count_params[j]+3],"%s",(char*)&(ppp->onlyRead[j]));
				sscanf((char*)val[i+2*ppp->count_params[j]+4],"%x",(unsigned int*)&ppp->eepromOffset[j]);
				i+=5+2*countRadio;
			}
			else if (strstr((char*)&key[i][0],"_T ")||strstr((char*)&key[i][0],"_T\t")||strstr((char*)&key[i][0],"_C ")||strstr((char*)&key[i][0],"_C\t"))
			{
				sscanf((char*)val[i],"%x",(unsigned int*)&j);j-=256;
				strcpy((char*)str1,(char*)val[i+1]);
				while((ptr = strpbrk((char*)str1,"\"")))		//find the first tab in input
				{ 
					*ptr=' '; 								//replace the tab with a emty-char to make the string filename of config_*.h
				}
				ppp->type[j]=3;
				strcpy((char*)&ppp->label[j],(const char*)str1);
				sscanf((char*)val[i],"%x",(unsigned int*)&(ppp->addr0[j]));
				sscanf((char*)key[i],"%100s",(char*)&(ppp->txt0[j]));
				sscanf((char*)val[i+2],"%s",&(ppp->onlyRead[j][0]));
				sscanf((char*)val[i+3],"%x",(unsigned int*)&ppp->eepromOffset[j]);
				i+=4;
			}
			else if (strstr((char*)&key[i][0],"_B ")||strstr((char*)&key[i][0],"_B\t"))
			{
				sscanf((char*)val[i],"%x",(unsigned int*)&j);
				j-=256;
				strcpy((char*)str1,(char*)val[i+1]);
				while((ptr = strpbrk((char*)str1,"\"")))		//find the first tab in input
				{ 
					*ptr=' '; 								//replace the tab with a emty-char to make the string filename of config_*.h
				}
				ppp->type[j]=4;
				strcpy((char*)&ppp->label[j],(const char*)str1);
				sscanf((char*)val[i],"%x",(unsigned int*)&(ppp->addr0[j]));
				sscanf((char*)key[i],"%100s",(char*)&(ppp->txt0[j]));
				sscanf((char*)val[i+2],"%s",&(ppp->onlyRead[j][0]));
				sscanf((char*)val[i+3],"%x",(unsigned int*)&ppp->eepromOffset[j]);
				i+=4;
			}
			else if (strstr((char*)&key[i][0],"__"))
			{
				sscanf((char*)val[i],"%x",(unsigned int*)&j);
				ppp->func[j]=1;
				sscanf((char*)key[i],"%s",(char*)&(ppp->txtFunc[j]));
			}
		}
		if (debugLevel&0x08) {
			for (i=0;i<128;i++) printf("label[%d]=%s \n",i,ppp->label[i]); 	
		}
	}
	else {
		printf("ERROR : can't open %s.\n",coredef_path);
		printf("You should download www.http://chantilly.sideme-electronique.com/download/chc/chc_installer.sh to update tools");
		printf("or run this command as root using \"sudo\".");
		CHAN_close();
		exit(EXIT_FAILURE);
	}
}
//
void list_connected_CORES(bus_struct* c_s){
	char core_info[64]="fedcba987654321\0";
	uint8_t ii=0,jj=0;
	printf(" __________________________________________________\n");
	printf("|Name           \t|Identifier|Address|gfw-ffw| \n");
	printf("|_______________________|__________|_______|_______|\n");
	for(ii=1;ii<8;ii++)
	{
		//if(myCORES[ii][2]!=0x00)
		if (c_s->core_id[ii]!=0)
		{
			//while (myCORES[ii][2]!=CORE_ID[jj++]);rrrrrrrrrrrrrrrrrrrrrrrrrrr
			while (c_s->core_id[ii]!=CORE_ID[jj++]){printf("jj=%d\n",jj);}
			;
			//sprintf(core_info,"|%-16s\t|0x%02X      |%1d      |%03d-%03d|",CORE_NAME[jj-1],myCORES[ii][2],myCORES[ii][0],myCORES[ii][1],myCORES[ii][3]);rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
			sprintf(core_info,"|%-16s\t|0x%02X      |%1d      |%03d-%03d|",CORE_NAME[jj-1],c_s->core_id[ii],ii,c_s->core_access_v[ii],c_s->core_firmware_v[ii]);
			
			printf("%s \n",core_info);	
		}
	}
	printf("|_______________________|__________|_______|_______|\n");
}
//
void dumpRam_CORE_I(uint8_t coreAddress){
	uint8_t i=0,j=0;
	chanErrors+=CHAN_addr4(coreAddress,0);
	chanErrors+=CHAN_getBytes(coreAddress,512,myRXbuffer);
	printf(" ADDR | 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F\n");
	printf("______  ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____\n"); 
	printf("\n");
	for (j=0;j<32;j++) {
		printf("0x%03X | ",j*16);
		for (i=0;i<16;i++) {
			printf("0x%02X ",myRXbuffer[(j*16)+i]);
		}
		printf("\n");
	}
}
//
void dumpRam_CORE(uint8_t slot,uint8_t coreAddress){
	uint8_t i=0,j=0;
	CHAND_setPRECISE(slot,coreAddress,0,_DEXEC_COMPLETE);
	CHAND_readBYTES(slot,coreAddress,255,myRXbuffer,_DEXEC_COMPLETE);
	CHAND_setPRECISE(slot,coreAddress,256,_DEXEC_COMPLETE);
	CHAND_readBYTES(slot,coreAddress,255,&myRXbuffer[256],_DEXEC_COMPLETE);
	printf(" ADDR | 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0B 0x0C 0x0D 0x0E 0x0F\n");
	printf("______  ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____\n"); 
	printf("\n");
	for (j=0;j<32;j++) {
		printf("0x%03X | ",j*16);
		for (i=0;i<16;i++) {
			printf("0x%02X ",myRXbuffer[(j*16)+i]);
		}
	printf("\n");
	}
}
//
void mapRam_CORE(uint8_t slot,uint8_t coreAddress,pcui* ppp){
	read_CORE(slot,coreAddress,256,255,ppp);
}
//
void mapRam_CORE_I(uint8_t coreAddress,pcui* ppp){
	read_CORE_I(coreAddress,256,255,0,ppp);
}
//
void read_CORE(uint8_t slot,uint8_t coreAddress,uint16_t ramAddress,uint8_t count,pcui* ppp){
	uint8_t i=0,j=0;
	j=ramAddress-256;
	CHAND_setPRECISE(slot,coreAddress,ramAddress,_DEXEC_COMPLETE);
	//CHAND_addr(slot,coreAddress,ramAddress,_DEXEC_COMPLETE);
	CHAND_readBYTES(slot,coreAddress,count,myRXbuffer,_DEXEC_COMPLETE);
	printf(" ______________________________________________________________\n");
	printf("|ram offset\t|header definition\t\t\t|value |\n");
	printf("|_______________|_______________________________________|______|\n");
	if (ramAddress>=256) for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,ppp->txt0[j++],myRXbuffer[i]);
	else for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,"\0",myRXbuffer[i]);
	printf("|_______________|_______________________________________|______|\n");
}
//
/*
void read_CORE_I(uint8_t coreAddress,uint32_t ramAddress,uint8_t count,pcui* ppp){
	int32_t i=0,j=0;
	j=ramAddress-256;
	//CHAN_precise(coreAddress,ramAddress);
	chanErrors+=CHAN_addr(coreAddress,ramAddress);
	//CHAN_readTO(coreAddress,count,myRXbuffer);
	chanErrors+=CHAN_getBytes(coreAddress,count,myRXbuffer);
	printf(" ______________________________________________________________\n");
	printf("|ram offset\t|header definition\t\t\t|value |\n");
	printf("|_______________|_______________________________________|______|\n");
	if (ramAddress>=256) for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,ppp->txt0[j++],myRXbuffer[i]);
	else for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,"\0",myRXbuffer[i]);
	printf("|_______________|_______________________________________|______|\n");
}*/
void read_CORE_I(uint8_t coreAddress,uint32_t ramAddress,uint8_t count,uint8_t decimal,pcui* ppp){
	int32_t i=0,j=0;
	j=ramAddress-256;
	//CHAN_precise(coreAddress,ramAddress);
	chanErrors+=CHAN_addr(coreAddress,ramAddress);
	//CHAN_readTO(coreAddress,count,myRXbuffer);
	chanErrors+=CHAN_getBytes(coreAddress,count,myRXbuffer);
	printf(" ______________________________________________________________\n");
	printf("|ram offset\t|header definition\t\t\t|value |\n");
	printf("|_______________|_______________________________________|______|\n");
	if (!decimal) {
	if (ramAddress>=256) for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,ppp->txt0[j++],myRXbuffer[i]);
	else for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|0x%02X  |\n",ramAddress++,"\0",myRXbuffer[i]);
	printf("|_______________|_______________________________________|______|\n");
	}
	else {
	if (ramAddress>=256) for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|%02d    |\n",ramAddress++,ppp->txt0[j++],myRXbuffer[i]);
	else for (i=0;i<count;i++) printf("|0x%03X   \t|%-32s\t|%02d    |\n",ramAddress++,"\0",myRXbuffer[i]);
	printf("|_______________|_______________________________________|______|\n");	
	}
}
//
void mapFunc_CORE(pcui* ppp){
	uint8_t i=0;
	printf(" __________________________________________________\n");
	printf("|function offset|function name\t\t\t   |\n");
	printf("|_______________|__________________________________|\n");
	for (i=0;i<64;i++) printf("|0x%02X\t\t|%-32s  |\n",i,ppp->txtFunc[i]);
	printf("|_______________|__________________________________|\n");
}
//
void write_CORE(uint8_t slot,uint8_t coreAddress,uint16_t ramAddress,uint8_t val){
	CHAND_setPRECISE(slot,coreAddress,ramAddress,_DEXEC_COMPLETE);
	CHAND_writeBYTES(slot,coreAddress,1,&val,_DEXEC_COMPLETE);
}
//
void write_CORE_I(uint8_t coreAddress,uint16_t ramAddress,uint8_t val){
	//CHAN_setBYTE(coreAddress,ramAddress,val);
	chanErrors+=CHAN_addr(coreAddress,ramAddress);
	chanErrors+=CHAN_setByte(coreAddress,val);
}
//
void formatWrite_CORE(uint8_t slot,uint8_t coreAddress,uint16_t ramAddress,uint8_t val_l,uint8_t val_h){
	uint8_t buf[2];
	buf[0]=val_l;
	buf[1]=val_h;
	CHAND_setPRECISE(slot,coreAddress,ramAddress,_DEXEC_COMPLETE);
	CHAND_writeBYTES(slot,coreAddress,2,buf,_DEXEC_COMPLETE);
}
//
void formatWrite_CORE_I(uint8_t coreAddress,uint16_t ramAddress,uint8_t val_l,uint8_t val_h){
	//CHAN_setBYTE(coreAddress,ramAddress,val_l);
	write_CORE_I(coreAddress,ramAddress,val_l);
	//CHAN_setBYTE(coreAddress,ramAddress+1,val_h);
	write_CORE_I(coreAddress,ramAddress+1,val_h);
}
//
pid_t create_process(void)
{
    pid_t pid;
    do {
		pid=fork();
    } while ((pid==-1) && (errno==EAGAIN));
    return pid;
}
//
void son_process(char *arg[])
{
	if (execv("/usr/bin/sudo",arg)==-1) {
	//if (execv("./coresdefs_installer.sh",arg)==-1) {
		perror("execv");
		CHAN_close();
		exit(EXIT_FAILURE);
    }
    
}
//
