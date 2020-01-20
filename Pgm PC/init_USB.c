#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>

/* Caractéristiques du périphérique */
#define VENDOR_ID	0x2341
#define PRODUCT_ID	0x0001
#define TAB_PA_SIZE	10

libusb_device_handle *handle=NULL;
libusb_device *device=NULL;
libusb_device *found=NULL; //notre périphérique
libusb_context *context;


/*énumération des périphériques USB*/
void enum_periph(){
    libusb_device **list;
    
    /* On compte le nombre de périphérique connectés... */
    ssize_t count=libusb_get_device_list(context,&list);
    if(count<0) {perror("libusb_get_device_list"); exit(-1);}
    ssize_t i=0;
    
    /* et on en établit la liste complète */
    for(i=0;i<count;i++){
        libusb_device *device=list[i];      //enregistre l'appareil i dans la liste list
        struct libusb_device_descriptor desc;
        int status=libusb_get_device_descriptor(device,&desc);      //enregistre le descripteur d'appareil
        if(status!=0) continue;
        
        /* On capture et affiche les numéros de bus et d'addresse */
        uint8_t bus=libusb_get_bus_number(device);
        uint8_t address=libusb_get_device_address(device);
        printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
        printf("Vendor ID 0x0%x\n",desc.idVendor);
        printf("Product ID 0x0%x\n\n",desc.idProduct);
        
        /* Recherche de notre périphérique et sauvegarde dans 'found' */
        if(desc.idVendor == VENDOR_ID && desc.idProduct == PRODUCT_ID){
        	found = device;
        	printf("\nPériphérique trouvé @ (Bus:Adresse) %d:%d\n", bus, address);
        	//printf("handle %d\n",found);
        	break;
        }
    }
    if (found == NULL) printf("Périphérique non trouvé\n");
    libusb_free_device_list(list,1);
}


void config_periph(unsigned char tab_PA[TAB_PA_SIZE]){

	/* Récupération de la 1ère configuration du périphérique (indice 0) */
    struct libusb_config_descriptor *config_desc=NULL;
    int status = libusb_get_config_descriptor(found,0,&config_desc); //config d'indice 0
    printf("La valeur de la configuration d'indice 0 est %d\n",config_desc->bConfigurationValue);
    
    int interface;
    
    /* Si le noyau est passé avant nous,
        On le détache pour chaque alternative d'interface numéro 0 */
    for(int indice_intf=0;indice_intf < config_desc->bNumInterfaces ;indice_intf++){        
        
        interface=config_desc->interface[indice_intf].altsetting[0].bInterfaceNumber;
        
        if(libusb_kernel_driver_active(handle,interface)){
        	int status=libusb_detach_kernel_driver(handle,interface);
        	if(status!=0){ perror("libusb_detach_kernel_driver"); exit(-1); } //erreur si status!=0
        }
    	//printf("indice intf trouvée %d\n",interface);
    }
 
        
    /* Utilisation de la configuration */
    
    int dernier_PA=-1;
    int configuration=config_desc->bConfigurationValue;
    
    //printf("valeur config %d\n", configuration); // Affichage valeur de la configuration
    status=libusb_set_configuration(handle,configuration);
    if(status!=0){ perror("libusb_set_configuration"); exit(-1); }
        
        
    /* Appropriation de toutes les interfaces */
    int num_intf;
    for(int indice_intf=0;indice_intf < config_desc->bNumInterfaces ;indice_intf++){
    	num_intf = config_desc->interface[indice_intf].altsetting[0].bInterfaceNumber;
    	
    	status=libusb_claim_interface(handle,indice_intf);   
    	if(status!=0){ perror("libusb_claim_interface"); exit(-1); }
    	printf("\tInterface d'indice %d et de numéro %d réclamée\n",indice_intf,num_intf);
    


		/* Parcours des points d'accès (Endpoint) pour chaque interface*/
		
		for(int num_PA=0;num_PA<config_desc->interface[indice_intf].altsetting[0].bNumEndpoints ;num_PA++){
			
			int type_PA = config_desc->interface[indice_intf].altsetting[0].endpoint[num_PA].bmAttributes;
			uint8_t adresse_PA = config_desc->interface[indice_intf].altsetting[0].endpoint[num_PA].bEndpointAddress;
            printf("\t\tPoint d'accès trouvé. Adresse : %d\n",adresse_PA);
            
        	/* Regarde si le point d'accès est de type interruption.
        	Si oui, on le sauvegarde. */
        	if((type_PA & 0b11)==LIBUSB_TRANSFER_TYPE_INTERRUPT){
				//struct EP ep1;
            	dernier_PA++;
            	tab_PA[dernier_PA] = adresse_PA;
            	printf("\t\tPoint d'accès numéro %d sauvegardé. Adresse : %d\n", dernier_PA, adresse_PA);
          	}
        
        }	
    }
    //printf("dernier indice tab_PA : %d\n", dernier_PA);
}


/* Libération des interfaces de la configuration active */
void liberer_interfaces(){ 
    
    /* Récupération de la configuration active */
    
    struct libusb_config_descriptor* config_desc;
    int status =libusb_get_active_config_descriptor(found,&config_desc); 
    if(status!=0){perror("libusb_get_active_config_descriptor");exit(-1);}
    
    /* On parcourt toutes les interfaces*/
    int num_intf;
    for(int indice_intf=0;indice_intf < config_desc->bNumInterfaces ;indice_intf++){
    	num_intf=config_desc->interface[indice_intf].altsetting[0].bInterfaceNumber;
    	
    	/* Libération de l'interface num_intf*/
        status=libusb_release_interface(handle,num_intf);
        if(status!=0){ perror("libusb_release_interface"); exit(-1); }  
        
        printf("\tL'interface numéro %d, d'indice %d a été libérée.\n", num_intf,indice_intf);          
	}
}


/* Envoie un caractère (de commande de LED) sur le port USB */
void send_data(unsigned char tab_PA[TAB_PA_SIZE], unsigned char data){

	unsigned char PA = tab_PA[0]; //LEDs sur le premier point d'accès
	printf("adresse PA %d\n", tab_PA[0]);
	int transferred = 1; 	//nombre d'octets transférés
	unsigned int timeout = 100;	//temps avant un timeout
	int status = libusb_interrupt_transfer(handle, PA, &data, sizeof(data), &transferred, timeout);
	if(status!=0){perror("libusb_interrupt_transfer");exit(-1);}
}


/* Lis le contenu des points d'accès de l'interface IN (boutons et joystick) */
void receive_data(unsigned char tab_PA[TAB_PA_SIZE], unsigned char *boutons, unsigned char *joystick_x, unsigned char *joystick_y){
	
	unsigned char PA;
	int length, transferred;
	unsigned int timeout;	
	
	/* Lecture du point d'accès des boutons */
	PA = tab_PA[1]; //LEDs sur le premier point d'accès
	
	//printf("adresse PA %d\n", tab_PA[1]);
	//printf("handle %d\n",found);
	//printf("boutons %p\n", boutons);
	
	length = 1;
	timeout = 100;	//temps avant un timeout
	
	int status = libusb_interrupt_transfer(handle, PA, boutons, length, &transferred, timeout);
	if(status!=0){perror("libusb_interrupt_transfer");exit(-1);}
	
	/* Lecture du point d'accès du joystick */
	unsigned char joystick_xy[2]; //stocke la donnée du point d'accès (1 octet pour chaque axe)

	PA = tab_PA[2]; //LEDs sur le premier point d'accès
	//printf("adresse PA %d\n", tab_PA[2]);
	length = 2;
	timeout = 100;	//temps avant un timeout
	
	status = libusb_interrupt_transfer(handle, PA, joystick_xy, length, &transferred, timeout);
	if(status!=0){perror("libusb_interrupt_transfer");exit(-1);}
	

	if (joystick_xy !=NULL){
		*joystick_x = joystick_xy[0];	//On sépare la data de chaque axe
		*joystick_y = joystick_xy[1];
	}
}



int main(){
    //init_tableau(); // initialisation tableau point d'accès endpoint
    
    /* Initialisation de la bibliothèque libusb-1.0 */
    
    int status=libusb_init(&context);
    if(status!=0) {perror("libusb_init"); exit(-1);}
    //fin inititialisation
    
    /* Enumération des périphériques USB */
    enum_periph();
    
    /* Ouverture de notre périphérique 'found' */
    int status_ouv=libusb_open(found,&handle);
    if(status_ouv!=0){ perror("libusb_open"); exit(-1); }
    
    /* Configuration du périphérique et sauvegarde des points d'accès */
    unsigned char tab_PA[TAB_PA_SIZE];
    config_periph(tab_PA);
    
    unsigned char boutons, boutons_anc=0xff;
    unsigned char joystick_x, joystick_x_anc=0xff;
    unsigned char joystick_y, joystick_y_anc=0xff;
    //unsigned char caractere;
    
    
    //TODO boucle while(pas d'arrêt), envoi et rcpt
    //TODO "pas d'arrêt" = appui sur 's' par exemple
    
    while(1){
    	/*// si un caractère intéressant est tapé au clavier, l'envoie à la carte.
		if (((c=getchar())>='a' && (c=getchar())<='f') || ((c=getchar())>='A' && (c=getchar())<='F')) { 
    		//récupération caractère
    		send_data(tab_PA, c);	//Envoi de la commande des leds
    	}
    	else {
    	*/
    		
    		receive_data(tab_PA, &boutons, &joystick_x, &joystick_y); //Réception des boutons et joystick
    		
    		
    		printf("Boutons : %02x, Joystick_x : %02x, Joystick_y :%02x\n", boutons, joystick_x, joystick_y);	//Affichage si changement
    		if ((boutons != boutons_anc) || (joystick_x != joystick_x_anc) || (joystick_y != joystick_y_anc)) printf("Boutons : %02x, Joystick_x : %02x, Joystick_y :%c\n", boutons, joystick_x, joystick_y);	//Affichage si changement
    		
    		boutons_anc = boutons;
    		joystick_x_anc = joystick_x;
    		joystick_y_anc = joystick_y;
    	//}
    
    }
    //*/
    
    /* Libération des interfaces*/
    liberer_interfaces();
    
	/* Fermeture du périphérique */
    libusb_close(handle);
    
    

    
    libusb_exit(context); //fermeture de la bibliothèque
    return 0;
}
