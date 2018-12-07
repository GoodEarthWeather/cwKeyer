
#include "main.h"

// define state machine
uint8_t state;  // holds current state

#define STATE_LISTEN_FOR_WAKEUP 1
#define STATE_LISTEN_FOR_RESPONSE 2
#define STATE_SEND_COMMAND 3
#define STATE_SEND_RESPONSE_DATA 4

#define SYNC_RADIO_CHANNEL 0
#define MAX_RETRIES 5
#define MAX_RESPONSEBUFFER_SIZE 20

uint8_t radioCommandBuffer[64];  // to hold command from radio
uint8_t radioResponseBuffer[MAX_RESPONSEBUFFER_SIZE][64];  // to hold response data from radio
volatile uint8_t validRadioCommand = 0;  // true when command assembly is complete
static uint8_t responseCount = 0;
struct systemStatus sysState;


void main(void)
{
    extern volatile uint8_t timeOut;
    uint8_t ucmd;
    uint8_t status;
    uint8_t reTry;
    extern uint8_t dataChannel;

    WDT_A_hold(WDT_A_BASE);

    sysState.upTimer = 0; // reset up time counter on power up or reset
    initGPIO();
    initClocks();
    initUART();
    initRTC();

    //getRFID();  // set node and hub addresses

    initRadio();

    // now go to sleep and wait for 1 minute flag to wake up
    state = STATE_LISTEN_FOR_WAKEUP;

    while (1)
    {
        switch (state)
        {
        case STATE_LISTEN_FOR_WAKEUP :  // this state is used to synchronize the hub to the node
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0); // LED1 on
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN3); // LED2 off
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN1); // LED3 off

            radioCommandBuffer[2] = 0;  // set to CMD_NULL
            validRadioCommand = 0;
            timeOut = 0; // no timeout function for wakeup, so reset timeOut flag in case it got previously set
            status = rxRecvPacket(radioCommandBuffer,SYNC_RADIO_CHANNEL);  // radio mode receive packet
            if ((radioCommandBuffer[2] == CMD_NODE_READY) && validRadioCommand)  // synchronized, so now send commands
            {
                dataChannel = radioCommandBuffer[4];  // get data channel to use
                responseBufferAdd(radioCommandBuffer); // add CMD_NODE_READY to response buffer (to get the wind data)
                reTry = 0;
                state = STATE_SEND_COMMAND;
            }
            else
            {
                state = STATE_LISTEN_FOR_WAKEUP;  // go back and wait for query command
            }
            break;
        case STATE_LISTEN_FOR_RESPONSE :
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0); // LED1 off
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN3); // LED2 on
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN1); // LED3 off
            radioCommandBuffer[2] = 0;  // set to CMD_NULL
            validRadioCommand = 0;
            startTimeOut();
            status = rxRecvPacket(radioCommandBuffer, dataChannel);  // radio mode receive packet
            stopTimeOut();
            // should check for errors
            if ( status == RX_OK )
            {
                responseBufferAdd(radioCommandBuffer);
                reTry = 0;
                state = STATE_SEND_COMMAND;  // send any additional commands
            }
            else if ( reTry < MAX_RETRIES )  // packet error - retry
            {
                reTry++;
                sysState.reTryCount++;
                state = STATE_SEND_COMMAND;
            }
            else  // packet error - but too many re-tries, so give up
            {
                clearCommandList();
                reTry = 0;
                state = STATE_LISTEN_FOR_WAKEUP;
            }
            break;
        case STATE_SEND_COMMAND :
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0); // LED1 off
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN3); // LED2 on
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN1); // LED3 on
            if ( !reTry )  // don't get next command if trying to re-send existing command
            {
                ucmd = getUARTCommand();
            }
            if (ucmd)
            {
                buildPacket(ucmd);
                sendPacket(dataChannel);  // send uart command to radio
                state = STATE_LISTEN_FOR_RESPONSE;
            }
            else // no commands to send, so tell radio to go to sleep
            {
                getRSSI();  // first get RSSI
                buildPacket(CMD_SET_SLEEP);
                sendPacket(dataChannel);
                state = STATE_SEND_RESPONSE_DATA;  // now send response data to console
            }
            break;
        case STATE_SEND_RESPONSE_DATA :   // this will send radio response data to console
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0); // LED1 off
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN3); // LED2 off
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN1); // LED3 on
            responseBufferSend();
            state = STATE_LISTEN_FOR_WAKEUP;
            break;
        default :
            break;
        }
    }
}

// Routine to add to responseBuffer
void responseBufferAdd(uint8_t *data)
{
    uint8_t i;
    for ( i = 0; i <= data[0]; i++ )
    {
        radioResponseBuffer[responseCount][i] = data[i];
    }
    // increase response counter, but don't increase if at limit of buffer size
    // this will cause any more response buffer adds to over write the last entry
    if ( ++responseCount == MAX_RESPONSEBUFFER_SIZE )
        responseCount--;
}

// Routine to send responseBuffer contents to console
void responseBufferSend(void)
{
    uint8_t i, j;
    extern uint8_t nodeID;

    j = 0;
    while (j < responseCount)
    {
        // go through radioResponseBuffer and send data
        for ( i = 0; i <= radioResponseBuffer[j][0]; i++ )
        {
            EUSCI_A_UART_transmitData(EUSCI_A0_BASE,radioResponseBuffer[j][i]);
        }
        j++;
    }
    // now send end response cmd
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE,LENGTH_CMD_ENDRESPONSE);
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE,nodeID);
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE,CMD_ENDRESPONSE);
    responseCount = 0;
}

