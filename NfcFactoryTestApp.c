/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

#include <stdio.h>
#include <unistd.h>
#include <tml.h>

#define print_buf(x,y,z)  {int loop; printf(x); for(loop=0;loop<z;loop++) printf("0x%.2x ", y[loop]); printf("\n");}

static char gNfcController_generation = 0;

static void RfOn (int handle)
{
    char NCIStartDiscovery[] = {0x21, 0x03, 0x09, 0x04, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x06, 0x01};
    char NCIRfOn[] = {0x2F, 0x3D, 0x02, 0x20, 0x01};
    char Answer[256];

    if (gNfcController_generation == 1) {
        printf("Continuous RF ON test - please tap a card ...\n");
        tml_transceive(handle, NCIStartDiscovery, sizeof(NCIStartDiscovery), Answer, sizeof(Answer));
        if((Answer[0] != 0x41) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) {
            printf("Cannot start discovery loop\n");
            return;
        }
        do {
            tml_receive(handle,  Answer, sizeof(Answer));
        } while ((Answer[0] != 0x61) || ((Answer[1] != 0x05) && (Answer[1] != 0x03)));
    }
    else {
        printf("Continuous RF ON test\n");
        tml_transceive(handle, NCIRfOn, sizeof(NCIRfOn), Answer, sizeof(Answer));
    }
    fgets(Answer, sizeof(Answer), stdin);
    printf("NFC Controller is now in continuous RF ON mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

static void Functional (int handle)
{
    char NCIStartDiscovery[] = {0x21, 0x03, 0x09, 0x04, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x06, 0x01};
    char NCIRestartDiscovery[] = {0x21, 0x06, 0x01, 0x03};
    char Answer[256];

    printf("Functional test mode, starting discovery loop ...\n");
    tml_transceive(handle, NCIStartDiscovery, sizeof(NCIStartDiscovery), Answer, sizeof(Answer));
    if((Answer[0] != 0x41) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) {
        printf("Cannot start discovery loop\n");
        return;
    }
    printf("NFC Controller is now in functional mode - Press Crtl^Z to stop\n");
    while(1) {
    	do {
            tml_receive(handle,  Answer, sizeof(Answer));
    	} while ((Answer[0] != 0x61) || ((Answer[1] != 0x05) && (Answer[1] != 0x03)));
	printf(" - tag discovered, restarting discovery loop ...\n"); tml_transceive(handle, NCIRestartDiscovery, sizeof(NCIRestartDiscovery), Answer, sizeof(Answer));
    }
}

static void Dump (int handle)
{
    int i, j, l;
    char get_rf[] = {0x2F, 0x14, 0x02, 0x00, 0x00};
    char Answer[255];

    printf("Dumping all RF settings:\n");
    for (i=0; i<0x9B; i++)
    {
        get_rf[3] = i;
	for (j=0; j<255; j++)
	{
	    if((j > 0x8F) && (j < 0xFE)) continue;
	    get_rf[4] = j;
	    tml_transceive(handle, get_rf, sizeof(get_rf), Answer, sizeof(Answer));
	    if ((Answer[0] != 0x4F) || (Answer[1] != 0x14) || (Answer[3] != 0x00)) continue;
	    printf(" transition 0x%02X, register 0x%02X = ", i, j); for(l=0; l<Answer[2]-1; l++) printf("%02X ", Answer[4+l]); printf("\n");
	}
    }
}

static void SetRF (int handle)
{
    char set_rf[] = {0x20, 0x02, 0x00, 0x01, 0xA0, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char Answer[255];
    unsigned int temp[4];

    printf("Set RF settings menu (enter 0 during transition ID selection step to leave):\n");

    while (1)
    {
	printf("\n");

	printf("- enter transition ID in (hexadecimal): ");
	scanf("%x", &temp[0]);
	set_rf[7] = (char) temp[0];
	if (set_rf[7] == 0)
	{
	    printf("Leaving RF setting menu\n");
	    return;
	}

	printf("- enter register ID in (hexadecimal): ");
	scanf("%x", &temp[0]);
	set_rf[8] = (char) temp[0];
	if (set_rf[8] == 0)
	{
	    printf("Wrong register ID\n");
	    continue;
	}

	printf("- enter value length (1, 2 or 4): ");
	scanf("%x", &temp[0]);
	if ((temp[0] != 1) && (temp[0] != 2) && (temp[0] != 4))
	{
	    printf("Wrong value length\n");
	    continue;
	}
	set_rf[2] = 6 + temp[0];
	set_rf[6] = 2 + temp[0];

	printf("- enter value (in hexadecimal): ");
	switch(temp[0])
	{
	case 1:
	    scanf("%x", &temp[0]);
	    set_rf[9] = (char) temp[0];
	    break;
	case 2:
	    scanf("%x %x", &temp[0], &temp[1]);
	    set_rf[9]  = (char) temp[0];
	    set_rf[10] = (char) temp[1];
	    break;
	default:
	    scanf("%x %x %x %x", &temp[0], &temp[1], &temp[2], &temp[3]);
	    set_rf[9]  = (char) temp[0];
	    set_rf[10] = (char) temp[1];
	    set_rf[11] = (char) temp[2];
	    set_rf[12] = (char) temp[3];
	    break;
	}

	tml_transceive(handle, set_rf, set_rf[2] + 3, Answer, sizeof(Answer));
	if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00))
	    printf("Error, cannot write the RF setting\n");
	else
	    printf("RF setting successfully written\n");
    }
}

static void GetNciParam (int handle)
{
    char get[] = {0x20, 0x03, 0x02, 0x01, 0x00};
    char Answer[255];
    unsigned int temp;

    printf("Get NCI parameter value menu (enter 'ff' during tag ID selection step to leave):\n");

    while (1)
    {
	printf("\n");

	printf("- enter NCI parameter tag ID in (hexadecimal): ");
	scanf("%x", &temp);

	if(temp == 0xff) break;

	get[4] = (char) temp;

	tml_transceive(handle, get, sizeof(get), Answer, sizeof(Answer));

	if ((Answer[0] != 0x40) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) {
	    printf("Error, cannot get parameter value\n");
        }
	else {
            int loop;
            printf("Parameter %.2X = ", Answer[5]);
            for(loop=0; loop<Answer[6]; loop++) printf("%.2X ", Answer[7+loop]);
            printf("\n");
        }
    }
}

static void SetNciParam (int handle)
{
    char set[255] = {0x20, 0x02, 0x00, 0x01, 0x00, 0x00};
    char Answer[255];
    unsigned int temp;
    int loop;

    printf("Set NCI parameter value menu (enter ff during tag ID selection step to leave):\n");

    while (1)
    {
	printf("\n");

	printf("- enter NCI parameter tag ID in (hexadecimal): ");
	scanf("%x", &temp);

	if(temp == 0xff) break;

	set[4] = (char) temp;

	printf("- enter length: ");
	scanf("%x", &temp);
	if (temp == 0) {
	    printf("Wrong length\n");
	    continue;
	}
	set[5] = temp;
	set[2] = 3 + temp;

	printf("- enter value (in hexadecimal) LSB first: ");
        for(loop=0; loop<set[5]; loop++) {
	    scanf("%x", &temp);
	    set[6+loop] = (char) temp;
        }

	tml_transceive(handle, set, set[2] + 3, Answer, sizeof(Answer));

	if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00))
	    printf("Error, cannot set the NCI parameter\n");
	else
	    printf("NCI parameter successfully set\n");
    }
}

static void GetPropParam (int handle)
{
    char get[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x00};
    char Answer[255];
    unsigned int temp;

    printf("Get proprietary parameter (starting with 0xA0) value menu (enter 'ff' during tag ID selection step to leave):\n");

    while (1)
    {
	printf("\n");

	printf("- enter NCI parameter tag ID in (hexadecimal): A0");
	scanf("%x", &temp);

	if(temp == 0xff) break;

	get[5] = (char) temp;

	tml_transceive(handle, get, sizeof(get), Answer, sizeof(Answer));

	if ((Answer[0] != 0x40) || (Answer[1] != 0x03) || (Answer[3] != 0x00)) {
	    printf("Error, cannot read parameter value\n");
        }
	else {
            int loop;
            printf("Parameter A0%.2X = ", Answer[6]);
            for(loop=0; loop<Answer[7]; loop++) printf("%.2X ", Answer[8+loop]);
            printf("\n");
        }
    }
}

static void SetPropParam (int handle)
{
    char set[255] = {0x20, 0x02, 0x00, 0x01, 0xA0, 0x00, 0x00};
    char Answer[255];
    unsigned int temp;
    int loop;

    printf("Set proprietary parameter value menu (enter ff during tag ID selection step to leave):\n");

    while (1)
    {
	printf("\n");

	printf("- enter proprietary parameter tag ID in (hexadecimal): A0");
	scanf("%x", &temp);

	if(temp == 0xff) break;

	set[5] = (char) temp;

	printf("- enter length: ");
	scanf("%x", &temp);
	if (temp == 0) {
	    printf("Wrong length\n");
	    continue;
	}
	set[6] = temp;
	set[2] = 4 + temp;

	printf("- enter value (in hexadecimal) LSB first: ");
        for(loop=0; loop<set[6]; loop++) {
	    scanf("%x", &temp);
	    set[7+loop] = (char) temp;
        }

	tml_transceive(handle, set, set[2] + 3, Answer, sizeof(Answer));

	if ((Answer[0] != 0x40) || (Answer[1] != 0x02) || (Answer[3] != 0x00))
	    printf("Error, cannot set the proprietary parameter\n");
	else
	    printf("Proprietary parameter successfully set\n");
    }
}

static void Prbs (int handle)
{
    char NCIPrbsPN7120[] = {0x2F, 0x30, 0x04, 0x00, 0x00, 0x01, 0x01};
    char NCIPrbsPN71X0[] = {0x2F, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF}; /* On PN7160, the only allowed value for "PRBS series length" (last two bytes) is 0x01FF when Firmware generated PRBS is selected*/
    char Answer[256];
    int tech, bitrate;

    printf("PRBS test:\n");
    printf(" Select technology (A=0, B=1, F=2): ");
    scanf("%d", &tech);
    printf(" Select bitrate (106=0, 212=1, 424=2, 848=3): ");
    scanf("%d", &bitrate);

    if (gNfcController_generation == 1) {
        NCIPrbsPN7120[3] = tech;
        NCIPrbsPN7120[4] = bitrate;
        tml_transceive(handle, NCIPrbsPN7120, sizeof(NCIPrbsPN7120), Answer, sizeof(Answer));
    }
    else {
        NCIPrbsPN71X0[5] = tech;
        NCIPrbsPN71X0[6] = bitrate;
        tml_transceive(handle, NCIPrbsPN71X0, sizeof(NCIPrbsPN71X0), Answer, sizeof(Answer));
    }
    fgets(Answer, sizeof(Answer), stdin);
    printf("NFC Controller is now in PRBS mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

static void Standby (int handle)
{
    char NCIEnableStandby[] = {0x2F, 0x00, 0x01, 0x01};
    char Answer[256];

    tml_transceive(handle, NCIEnableStandby, sizeof(NCIEnableStandby), Answer, sizeof(Answer));
    if((Answer[0] != 0x4F) || (Answer[1] != 0x00) || (Answer[3] != 0x00)) {
        printf("Cannot set the NFC Controller in standby mode\n");
        return;
    }

    /* Wait to allow PN71xx entering the standby mode */
    usleep(500 * 1000);

    fgets(Answer, sizeof(Answer), stdin);
    printf("NFC Controller is now in standby mode - Press enter to stop\n");
    fgets(Answer, sizeof(Answer), stdin);
}

static int reset_controller(int handle)
{
    char NCICoreReset[] = {0x20, 0x00, 0x01, 0x01};
    char NCICoreInit1_0[] = {0x20, 0x01, 0x00};
    char NCICoreInit2_0[] = {0x20, 0x01, 0x02, 0x00, 0x00};
    char Answer[256];
    int NbBytes = 0;

    tml_reset(handle);
    tml_transceive(handle, NCICoreReset, sizeof(NCICoreReset), Answer, sizeof(Answer));

    /* Catch potential notification */
    usleep(100*1000);
    NbBytes = tml_receive(handle,  Answer, sizeof(Answer));

    tml_transceive(handle, NCICoreReset, sizeof(NCICoreReset), Answer, sizeof(Answer));

    /* Catch potential notification */
    usleep(100*1000);
    NbBytes = tml_receive(handle,  Answer, sizeof(Answer));

    if((NbBytes == 12) && (Answer[0] == 0x60) && (Answer[1] == 0x00) && (Answer[3] == 0x02))
    {
	NbBytes = tml_transceive(handle, NCICoreInit2_0, sizeof(NCICoreInit2_0), Answer, sizeof(Answer));
        if((NbBytes < 19) || (Answer[0] != 0x40) || (Answer[1] != 0x01) || (Answer[3] != 0x00))    {
            printf("Error communicating with NFC Controller\n");
            return -1;
        }
        gNfcController_generation = 3;
    }
    else
    {
	NbBytes = tml_transceive(handle, NCICoreInit1_0, sizeof(NCICoreInit1_0), Answer, sizeof(Answer));
        if((NbBytes < 19) || (Answer[0] != 0x40) || (Answer[1] != 0x01) || (Answer[3] != 0x00))    {
            printf("Error communicating with PN71xx NFC Controller\n");
            return -1;
        }

        /* Retrieve NXP-NCI NFC Controller generation */
        if (Answer[17+Answer[8]] == 0x08) {
            gNfcController_generation = 1;
        }
        else if (Answer[17+Answer[8]] == 0x10) {
            gNfcController_generation = 2;
        }
        else {
            return -1;
        }
    }
    return 0;
}

int main()
{
    int nHandle;
    char NCIDisableStandby[] = {0x2F, 0x00, 0x01, 0x00};
    char Answer[256];
    int choice = 0;

    printf("\n----------------------------\n");
    printf("NFC Factory Test Application\n");
    printf("----------------------------\n");

    if(tml_open(&nHandle) != 0) {
        printf("Cannot connect to NFC controller\n");
        return -1;
    }

    if(reset_controller(nHandle) != 0) {
        printf("Error communicating with the NFC Controller\n");
        return -1;
    }

    switch(gNfcController_generation) {
    case 1: printf("PN7120 NFC controller detected\n"); break;
    case 2: printf("PN7150 NFC controller detected\n"); break;
    case 3: printf("PN7160 NFC controller detected\n"); break;
    default: printf("Wrong NFC controller detected\n"); break;
    }

    /* Disable standby mode */
    tml_transceive(nHandle, NCIDisableStandby, sizeof(NCIDisableStandby), Answer, sizeof(Answer));

    do {
        printf("Select the test to run:\n");
        printf("\t 1. Continuous RF ON mode\n");
        printf("\t 2. Functional mode\n");
        printf("\t 3. PRBS mode\n");
        printf("\t 4. Standby mode\n");
        printf("\t 5. Dump RF settings\n");
        printf("\t 6. Set RF setting\n");
        printf("\t 7. Get NCI parameter value\n");
        printf("\t 8. Set NCI parameter value\n");
        printf("\t 9. Get proprietary parameter value\n");
        printf("\t10. Set proprietary parameter value\n");
        printf("enter 0 to leave the application\n");
        printf("Your choice: ");
        scanf("%d", &choice);

        switch(choice) {
	    case 0: break;
	    case 1: RfOn(nHandle);
		reset_controller(nHandle);
		break;
            case 2: Functional(nHandle); break;
            case 3: Prbs(nHandle);
		reset_controller(nHandle);
		break;
            case 4: Standby(nHandle);
		reset_controller(nHandle);
                tml_transceive(nHandle, NCIDisableStandby, sizeof(NCIDisableStandby), Answer, sizeof(Answer));
		break;
            case 5: Dump(nHandle); break;
            case 6: SetRF(nHandle); break;
            case 7: GetNciParam(nHandle); break;
            case 8: SetNciParam(nHandle); break;
            case 9: GetPropParam(nHandle); break;
            case 10: SetPropParam(nHandle); break;
            default: printf("Wrong choice\n"); break;
        }
    } while(choice != 0);

    tml_reset(nHandle);
    tml_close(nHandle);

    return 0;
}
