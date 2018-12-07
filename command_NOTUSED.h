/*
 * command.h
 *
 *  Created on: May 24, 2018
 *      Author: dmcneill
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#define CMD_NULL 0   // no command

#define QCMD_QUERY 1
#define QCMD_HT 2
#define RCMD_HT 3
#define QCMD_WIND 4
#define RCMD_WIND 5
#define QCMD_RAIN 6
#define RCMD_RAIN 7
#define QCMD_POWER 8
#define RCMD_POWER 9
#define QCMD_HTP 10
#define RCMD_HTP 11
#define SCMD_ETIME 12
#define QCMD_ETIME 13
#define RCMD_ETIME 14
#define SCMD_SLEEP 15
#define QCMD_PRESSURE 16
#define RCMD_PRESSURE 17
#define SCMD_RFID 18
#define QCMD_RSSI 19
#define RCMD_NODE_RSSI 20
#define RCMD_ACK 21
#define HCMD_SET_ETIME 22
#define HCMD_SET_CMDLIST 23
#define HCMD_ENDRESPONSE 24
#define HCMD_RSSI 25
#define RCMD_HUB_RSSI 26
#define QCMD_NODE_SYS 27
#define RCMD_NODE_SYS 28
#define QCMD_HUB_SYS 29
#define RCMD_HUB_SYS 30
#define SCMD_CLR_SYS 31
#define HCMD_CLR_SYS 32
#define SCMD_WAKEUP_TIME 33


// lengths of each command packet
#define LENGTH_QCMD_QUERY 24
#define LENGTH_QCMD_HT 2
#define LENGTH_RCMD_HT 10
#define LENGTH_QCMD_WIND 2
#define LENGTH_RCMD_WIND 22
#define LENGTH_QCMD_RAIN 2
#define LENGTH_RCMD_RAIN 7
#define LENGTH_QCMD_POWER 2
#define LENGTH_RCMD_POWER 12
#define LENGTH_QCMD_HTP 2
#define LENGTH_RCMD_HTP 18
#define LENGTH_SCMD_ETIME 6
#define LENGTH_QCMD_ETIME 2
#define LENGTH_RCMD_ETIME 6
#define LENGTH_SCMD_SLEEP 2
#define LENGTH_QCMD_PRESSURE 2
#define LENGTH_RCMD_PRESSURE 10
#define LENGTH_SCMD_RFID 4
#define LENGTH_QCMD_RSSI 2
#define LENGTH_RCMD_NODE_RSSI 4
#define LENGTH_RCMD_ACK 2
#define LENGTH_HCMD_SET_ETIME 6
#define LENGTH_HCMD_ENDRESPONSE 2
#define LENGTH_HCMD_RSSI 2
#define LENGTH_RCMD_HUB_RSSI 4
#define LENGTH_RCMD_NODE_SYS 20
#define LENGTH_QCMD_NODE_SYS 2
#define LENGTH_RCMD_HUB_SYS 20
#define LENGTH_QCMD_HUB_SYS 2
#define LENGTH_SCMD_CLR_SYS 2
#define LENGTH_HCMD_CLR_SYS 2
#define LENGTH_SCMD_WAKEUP_TIME 3


#endif /* COMMAND_H_ */
