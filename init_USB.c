#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>

libusb_device_handle *handle=NULL;
libusb_device *device=NULL;
libusb_device *found=NULL; //notre périphérique

struct TABLEAU{
    struct EP *tableau[3]; //tableau des points d'accès en interruption
    int dernier;
};

struct EP {
    uint8_t ep; 				//adresse d'accès de l'endpoint
    unsigned char io;   //décrit si c'est une entrée ou une sortie
};

//init structure tableau
struct TABLEAU tab_pa;

void init_tableau(){
    for (int i=0;i<3;i++){        
    tab_pa.tableau[i]=NULL;
    }
    tab_pa.dernier=0;
}

/*énumération des périphériques USB*/
void enum_periph(libusb_context *context){
    libusb_device **list;
    ssize_t count=libusb_get_device_list(context,&list);
    if(count<0) {perror("libusb_get_device_list"); exit(-1);}
    ssize_t i=0;
    for(i=0;i<count;i++){
        libusb_device *device=list[i];      //enregistre l'appareil i dans la liste list
        struct libusb_device_descriptor desc;
        int status=libusb_get_device_descriptor(device,&desc);      //enregistre le descripteur d'appareil
        if(status!=0) continue;
        uint8_t bus=libusb_get_bus_number(device);              //num bus
        uint8_t address=libusb_get_device_address(device);      //num adresse
        printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
        printf("Vendor ID 0x0%x\n",desc.idVendor);
        printf("Product ID 0x0%x\n",desc.idProduct);
        
        /* PEUT VARIER D'UNE CARTE A L'AUTRE ?*/ 
        if(desc.idVendor == 0x413c && desc.idProduct == 0x2003) found = device;      //recherche de notre périphérique et sauvegarde dans found
        //printf("device :%d \n handle : %d\n", device, found);     //test si le "found" reste le meme
    }
    libusb_free_device_list(list,1);
}


void config_periph(){
    struct libusb_config_descriptor *config=NULL;
    libusb_get_active_config_descriptor(found,&config);   //on récupère la structure config_descriptor config dans found
    
    for(int num_intf=0;num_intf < config->bNumInterfaces ;num_intf++){
        
        //Si le méchant noyau est passé avant nous
        int interface; //on récupère tous les numéros des alt_settings numéro 0 => for et détach
        
        interface=config->interface[num_intf].altsetting[0].bInterfaceNumber;

        // on detache cette interface
        if(libusb_kernel_driver_active(handle,interface)){
        int status=libusb_detach_kernel_driver(handle,interface);
        if(status!=0){ perror("libusb_detach_kernel_driver"); exit(-1); } //si status!=0 il y a une erreur
        }
        printf("indice intf trouvée %d\n",interface);
    }
        
        
    //Utilisation d'une configuration du périphérique
    int configuration=config->bConfigurationValue;
    int status=libusb_set_configuration(handle,configuration);
    if(status!=0){ perror("libusb_set_configuration"); exit(-1); }
        
    //Appropriation d'une interface (fonctionnalité USB)
    status=libusb_claim_interface(handle,interface);   
    if(status!=0){ perror("libusb_claim_interface"); exit(-1); }  //si status!=0 il y a une erreur
        
    printf("indice interface claim %d",interface);
       

    //Sauvegarde des points d'accès
    for(int num_intf=0;num_intf < config->bNumInterfaces ;num_intf++){
        for(num_ep=0;num_ep<config->interface[num_intf].altsetting[0].bNumEndpoints ;num_ep++){
            int eptype = config->interface[num_intf].altsetting[0].endpoint.bDescriptorType;
            if(eptype & 0b11==LIBUSB_TRANSFER_TYPE_INTERRUPT){ //sauvegarde dans tab_pa du point d'accès qui est une interruption
                struct EP ep1;
                    
                //LAISSER TOMBER INPUT OUTPUT
                ep1.ep=config->interface[num_intf].altsetting[0].endpoint.bEndpointAddress;
                if (num_intf==0){
                     if (eptype & 0b10000000 == LIBUSB_ENDPOINT_OUT){ 
                         tab_pa.tableau[num_intf].ep=ep1.ep;
                         tab_pa.tableau[num_intf].io=1;
                         tab_pa.dernier++;
                         break;
                    }      //sortie => io=1
                }
                else{
                    if (eptype & 0b10000000 == LIBUSB_ENDPOINT_IN) {
                        tab_pa.tableau[num_intf].ep=ep1.ep;
                        tab_pa.tableau[num_intf].io=0;
                        tab_pa.dernier++;
                        if (num_ep >2)break;//entrée => io=0
                    }
                }   
            }
        }
    if (num_intf = 3) break;
    }
        
    //release des interfaces  
    for(int num_intf=0;num_intf < config->bNumInterfaces ;num_intf++){
        status=libusb_release_interface(handle,interface);
        if(status!=0){ perror("libusb_release_interface"); exit(-1); }
            
    }
}

int main(){
    init_tableau(); // initialisation tableau point d'accès endpoint
    //initialisation de la bibliothèque libusb-1.0
    libusb_context *context;
    int status=libusb_init(&context);
    if(status!=0) {perror("libusb_init"); exit(-1);}
    //fin inititialisation
    
    enum_periph(context); //énumération périphériques USB
    
    //ouverture du périphérique
    //libusb_device_handle *handle;
    int status_ouv=libusb_open(found,&handle);
    if(status_ouv!=0){ perror("libusb_open"); exit(-1); }  //status_ouv!=0 => erreur d'ouverture
    

    
    
    /*fermeture du périphérique*/
    libusb_close(handle);
    
    libusb_exit(context); //fermeture de la bibliothèque
    return 0;
}
