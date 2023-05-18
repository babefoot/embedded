#include "../../include/RFID/rfid_sensor.h"

/*
Renvoie l'Id de la carte scannée en Hexadecimal
*/
char * readIdCard() {

	double set_val = 0;
 
    if (geteuid() != 0)
    {
        p_printf(RED,"Must be run as root.\n");
        exit(1);
    }

	set_signals();
    if (get_config_file()) exit(1);
    if (HW_init(spi_speed,gpio)) close_out(1);
    
	InitRc522();

    printf("Scan de la carte en cours \n");
    char * retourCard = disp_card_details_idCard(); 
    printf("ID reçu : <%s>\n", retourCard);
    close_out2(); 

    return retourCard;
}