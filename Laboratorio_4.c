#use define_ucos2.lib
#use ucos2.lib
#use EVENTO.LIB

void print_consola(char *p_string,void *ptr );
void print_ethernet(char *p_string, tcp_Socket* ptr );
int get_consola(char *p_string,void *ptr );
int get_ethernet(char *p_string, tcp_Socket* ptr );
void MENU_USUARIO(void);
void MENU_ETHERNET(void *data);
void TICKS(void *data);
void ALARMA (void);
struct Event evento [CANTIDAD_EVENTOS+1];

OS_EVENT *Semaforo;
void main()
{

	int i;
	HW_init();
	for ( i = 0 ; i<CANTIDAD_EVENTOS; i++)
	{
		evento[i].numero = VACIO;
    }

	OSInit();

	OSTaskCreate(LED_OS_Prender_Led_Rojo,NULL,TASK_STK_SIZE_256,PRIO_LED);
	OSTaskCreate(MENU_USUARIO,NULL,TASK_STK_SIZE_1024,PRIO_USUARIO_PC);
	OSTaskCreate(ALARMA,NULL,TASK_STK_SIZE_1024,PRIO_ALARMA);
	for (i = 0 ; i<CANTIDAD_USUARIOS; i++)
	{
		OSTaskCreate(TICKS,&echosock[i],TASK_STK_SIZE_2048,PRIO_TICKS);
		OSTaskCreate(MENU_ETHERNET,&echosock[i],TASK_STK_SIZE_2048,PRIO_USUARIO_ETH);
	}
	Semaforo = OSSemCreate(1);

	OSStart();

}

void print_consola(char *p_string,void *ptr )
{
	printf(p_string);
}
void print_ethernet(char *p_string, tcp_Socket* ptr )
{
	sock_puts (ptr, p_string);
}

int get_consola(char *p_string,void *ptr )
{
	while (!getswf(p_string))
	{
		OSTimeDlyHMSM (0,0,0,100);
	}
	return 1;
}
int get_ethernet(char *p_string, tcp_Socket* ptr )
{
	if (tcp_tick(ptr) != 0)
	{
		if (sock_bytesready(ptr)>0)
		{
			if(sock_gets(ptr,p_string,512))
			{
				sock_puts(ptr,p_string);
				return 1;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return ERROR_CONEXION; // TIENE QUE VALER -1
	}

}
void MENU_USUARIO(void)
{
	while(1)
	{
		EVENTO_Menu(get_consola, print_consola, evento ,USUPC,NULL);
	}
}
void MENU_ETHERNET(void *data)
{
	tcp_Socket *ptr;
	ptr = (tcp_Socket*)data;
	while(1)
	{
		if (tcp_tick(ptr))
		{
			EVENTO_Menu(get_ethernet,print_ethernet, evento,ETH,ptr);
		}
	}

}
void TICKS(void *data)
{
	tcp_Socket *ptr;

	ptr = (tcp_Socket*)data;
	while(1)
	{
		tcp_listen(ptr,PORT,0,0,NULL,0);
		while (!sock_established(ptr))
		{
			tcp_tick(NULL);
			OSTimeDlyHMSM (0,0,0,100);
		}
		printf("Usuario Conectado\n");
		sock_mode(ptr,TCP_MODE_ASCII);
		while (tcp_tick(ptr)!=0)
		{
			OSTimeDlyHMSM (0,0,0,100);
		}
	}
}

void ALARMA (void)
{
	unsigned long int time_2;
	int i;
	char led;
	char frecuencia;
	char buffer_print[256];
       while (1)
       {
			for(i = 0; i<CANTIDAD_EVENTOS;i++)
			{
            	time_2 = read_rtc();
				if (evento[i].numero != VACIO)
				{
					if (evento[i].time == time_2)
					{
						sprintf(buffer_print,"\n Evento!! %d\n",evento[i].numero);
						print_consola(buffer_print,NULL);
						led = evento[i].param;
						frecuencia = evento[i].frec;
						LED_Prender_Led_frec_cant_veces(led,  frecuencia);
						evento[i].numero = VACIO;


					}
				}
           	OSTimeDlyHMSM (0,0,0,100);
			}
		}
}