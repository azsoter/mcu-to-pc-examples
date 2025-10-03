/*
* ethernet.c
*
*  Created on: Oct 10, 2020
*      Author: Andras Zsoter
*
*      Copyright (c) Andras Zsoter 2020-2025.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include <string.h>
#include <eth.h>
#include <network.h>
#if defined(NETWORK_ETHERNET_SUPPORTED)
#include <ethernet.h>
#include <network_interfaces.h>
#include <formatted_printing.h>
#include <os_sys.h>
#include <printing.h>
#if defined(NETWORK_IPV4_SUPPORTED)
#include <apipa.h>
#include <dhcp.h>
#endif
#if defined(NETWORK_DNS_SUPPORT)
#include <dns_internal.h>
#endif
#include <net_interface.h>

network_interface_t Ethernet0;

#if defined(NETWORK_IPV4_SUPPORTED) && defined(NETWORK_IPV4_DHCP_SUPPORTED)
dhcp_info_t DHCP_Info_Ethernet0;
#endif

#define TX_CALLBACK /* Seems to be fixed. */

#define ERROR_CALLBACK /* Not sure??? */

ETH_DMADescTypeDef  EthernetDMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4))) __attribute__((section(".ethernet_descriptor")));
ETH_DMADescTypeDef  EthernetDMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4))) __attribute__((section(".ethernet_descriptor")));

uint8_t EthernetRxBuff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]  __attribute__ ((aligned (4))) __attribute__((section(".ethernet_buffer")));
uint8_t EthernetTxBuff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]  __attribute__ ((aligned (4))) __attribute__((section(".ethernet_buffer")));

#if defined(TX_CALLBACK)
OS_SYS_Semaphore ethernetTxDone;
#endif

OS_SYS_Semaphore ethernetRecvIncoming;

OS_SYS_Semaphore ethernetTransmitterLock;

uint8_t EthUserXmitBuffer[ETH_TX_BUF_SIZE];

uint32_t ethernet_rx_buffer_count(const network_interface_t *net)
{
	return ETH_RXBUFNB;
}

int EthTransmitDataPacket_unlocked(uint8_t *data, uint16_t length, OS_SYS_Time timeout)
{
	int res = 0;
	int hal_res;
	int i;
	uint32_t tmp;

	volatile ETH_DMADescTypeDef *DmaTxDesc = heth.TxDesc;
	uint8_t *output_buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);

#if 0
	tmp = (EthernetDMATxDscrTab[0].Status | EthernetDMATxDscrTab[1].Status | EthernetDMATxDscrTab[2].Status | EthernetDMATxDscrTab[3].Status);
	if (0 != (tmp & ETH_DMATXDESC_OWN))
	{
		PRINTF("%s() %08x, %08x %08x, %08x\r\n", __FUNCTION__,
				EthernetDMATxDscrTab[0].Status, EthernetDMATxDscrTab[1].Status, EthernetDMATxDscrTab[2].Status, EthernetDMATxDscrTab[3].Status);
	}
#endif
	//PRINTF("Ethernet Transmitter[b]: DMASR=0x%08X\r\n", heth.Instance->DMASR);

	if (0 == length)
	{
		return ETHERNET_RESULT_OK;
	}

	if (length > (ETH_TX_BUF_SIZE))
	{
		return ETHERNET_RESULT_FAILED;
	}

#if defined(TX_CALLBACK)
	OS_SYS_RESETSEMAPHORE(ethernetTxDone);
	if (0 != (DmaTxDesc->Status & ETH_DMATXDESC_OWN))
	{
    	// OS_SYS_GETSEMAPHORE(ethernetTxDone, 8);
    	OS_SYS_GETSEMAPHORE(ethernetTxDone, timeout);
	}
#endif

	if (0 == (DmaTxDesc->Status & ETH_DMATXDESC_OWN))
    {
    	memcpy(output_buffer, data, length);
#if defined(TX_CALLBACK)
    	DmaTxDesc->Status |= ETH_DMATXDESC_IC;
#endif
    	hal_res = HAL_ETH_TransmitFrame(&heth, length);
    	//
    	// Check if there is a buffer that is owned by the DMA.
    	// If so check if we are even trying to transmit, and if not, make sure that we do.
    	// I have no idea why the check fails in HAL_ETH_TransmitFrame(), but apparently it sometimes does.
    	//

#if 0
		tmp = (EthernetDMATxDscrTab[0].Status | EthernetDMATxDscrTab[1].Status | EthernetDMATxDscrTab[2].Status | EthernetDMATxDscrTab[3].Status);
#else
    	tmp = 0;
    	for (i = 0; i < (ETH_TXBUFNB); i++)
    	{
    		tmp |= EthernetDMATxDscrTab[i].Status;
    	}
#endif
		if (0 != (tmp & ETH_DMATXDESC_OWN))
		{
			// The section below is already in HAL_ETH_TransmitFrame().
			// For some reason it does not always work the first time there, so we repeat it here.
			//
			if (0 != ((heth.Instance)->DMASR & ETH_DMASR_TBUS))
			{
			    (heth.Instance)->DMASR = ETH_DMASR_TBUS; // Clear TBUS ETHERNET DMA flag.
			    (heth.Instance)->DMATPDR = 0; 			 // Resume DMA transmission.
			}

		}

    	//PRINTF("Ethernet Transmitter[a]: DMASR=0x%08X\r\n", heth.Instance->DMASR);

    	if (HAL_OK != hal_res)
    	{
    		PRINTF("ERROR: Transmitting buffer %d.\r\n", hal_res);
    		res = ETHERNET_RESULT_FAILED;
    	}
    }
    else
    {
    	PRINTF("ERROR: Transmit buffer was not free. DMASR = 0x%08x\r\n", (heth.Instance)->DMASR);
    	res = ETHERNET_RESULT_NO_BUFFER;
    }

    // This test was in all the examples shipped with MxCube.
    //
    // When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission.
	//
    if (0 != (heth.Instance->DMASR & ETH_DMASR_TUS))
    {
    	PUTS("TRANSMIT UNDERFLOW\r\n");
    	heth.Instance->DMASR = ETH_DMASR_TUS;     //Clear TUS ETHERNET DMA flag.
    	heth.Instance->DMATPDR = 0;				  // Resume DMA transmission.
    }

	return res;
}

int EthTransmitDataPacket_Nucleo144(const network_interface_t *net, uint8_t *data, uint16_t length, OS_SYS_Time timeout)
{
	int res;
	// uint32_t reg;

	OS_SYS_GETSEMAPHORE(ethernetTransmitterLock, OS_SYS_TIMEOUT_FOREVER); // Lock the transmitter.

#if 0
	PRINTF("Ethernet descriptor status (before) %08x %08x %08x %08x\r\n",
				EthernetDMATxDscrTab[0].Status,
				EthernetDMATxDscrTab[1].Status,
				EthernetDMATxDscrTab[2].Status,
				EthernetDMATxDscrTab[3].Status
				);
#endif
		res = EthTransmitDataPacket_unlocked(data, length, timeout);
		if (0 <= res)
		{
			NetStats.ethernet_tx++;
		}
#if 0
		PRINTF("Ethernet descriptor status  (after) %08x %08x %08x %08x\r\n",
				EthernetDMATxDscrTab[0].Status,
				EthernetDMATxDscrTab[1].Status,
				EthernetDMATxDscrTab[2].Status,
				EthernetDMATxDscrTab[3].Status
				);
#endif
		//reg = heth.Instance->DMASR;
		//if  (0 == (reg & ETH_DMASR_ETS))
		//{
		//	PRINTF("Ethernet: ETH_DMASR ETS is not set, ETH_DMASR=0x%08X\r\n", reg);
		//}

	OS_SYS_POSTSEMAPHORE(ethernetTransmitterLock); // Unlock the transmitter.
	// PUTS("Transmitter unlocked....\r\n");
	return res;
}
// ----------------------------------------------------------------------------------------------------------------------------------------
int ethernet_transmit_datapacket(const network_interface_t *net, uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
	int res;

#if defined(NETWORK_VERBOSE_ETHERNET)
	if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
	{
		PRINTF("%s(%s, %p, %u, %u)\r\n", __FUNCTION__, net->net_name, data, length, timeout_ms);
		print_all_low_level_headers(net, data);
	}

	res = EthTransmitDataPacket_Nucleo144(net, data, length, OS_SYS_MS_TO_TICKS(timeout_ms));

	if (ETHERNET_RESULT_NO_BUFFER == res) // If the problem is that the transmitter does not have enough buffers it is worth to retry.
	{
#if defined(NETWORK_VERBOSE_ETHERNET)
		if (1 || (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET)))
		{
			PUTS("Ethernet TX buffer full.\r\n");
			// print_all_low_level_headers(net, data);
		}
#endif
	}

	return res;
}
#endif
int ethernet_transmit_buffer(const network_interface_t *net, buffer_header_t *mb, uint32_t timeout_ms)
{
	int res;
	OS_SYS_ASSERT(0 != mb);
	res = ethernet_transmit_datapacket(net, mb->data, mb->data_length, timeout_ms);
	ReleasePooledBuffer(mb, OS_SYS_TIMEOUT_FOREVER);
	return res;
}
// ----------------------------------------------------------------------------------------------------------------------------------------
// #define CONFIG_ETHERNET_DO_COPY_RECEIVE_BUFFER 1

int Ethernet_ReceivePacket(uint8_t *data)
{
	volatile ETH_DMADescTypeDef *DmaRxDesc;
	uint16_t length = 0;
	uint8_t *buffer;
	int i;

	// PRINTF("Ethernet Receiver: DMASR=0x%08X\r\n", heth.Instance->DMASR);

	if (HAL_OK != HAL_ETH_GetReceivedFrame_IT(&heth))
	{
		return 0;
	}

	length = heth.RxFrameInfos.length;

	if (length > (ETH_RX_BUF_SIZE))
	{
		PRINTF("ERROR: Ethernet receiver, length is longer than buffer %d\r\n", length);
	}
	else if (0 < length)
	{
		NetStats.ethernet_rx++;
		buffer = (uint8_t *)(heth.RxFrameInfos.buffer);
#if defined(NETWORK_VERBOSE_ETHERNET)
	if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
	{
		PUTS("Incoming Ethernet packet:\r\n");
		print_all_low_level_headers(&Ethernet0, buffer);
	}
#endif
#if defined(CONFIG_ETHERNET_DO_COPY_RECEIVE_BUFFER)
	memcpy(data, buffer, length);
#else
		ethernet_process_incoming(&Ethernet0, buffer, length, EthUserXmitBuffer, sizeof(EthUserXmitBuffer));
#endif
	}

	if (1 < heth.RxFrameInfos.SegCount)
	{
		PRINTF("WARNING: Ethernet receiver SegmentCount=%d\r\n", heth.RxFrameInfos.SegCount);
	}
    // This was in all the examples shipped with MxCube.
	//
	// Release descriptors to DMA.
	// Point to first descriptor.
	DmaRxDesc = heth.RxFrameInfos.FSRxDesc;

	// Set Own bit in Rx descriptors: gives the buffers back to DMA.
	for (i = 0; i < heth.RxFrameInfos.SegCount; i++)
	{
		DmaRxDesc->Status |= ETH_DMARXDESC_OWN;
		DmaRxDesc = (ETH_DMADescTypeDef *)(DmaRxDesc->Buffer2NextDescAddr);
	}

	// Clear Segment_Count.
	heth.RxFrameInfos.SegCount = 0;

	// When Rx Buffer unavailable flag is set: clear it and resume reception/
	if ((uint32_t)RESET != (heth.Instance->DMASR & ETH_DMASR_RBUS))
	{
	   heth.Instance->DMASR = ETH_DMASR_RBUS; 	// Clear RBUS ETHERNET DMA flag
	   heth.Instance->DMARPDR = 0;				// Resume DMA reception.
	}

	return length;
}
// -----------------------------------------------------------------------------------------
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	OS_SYS_POSTSEMAPHORE(ethernetRecvIncoming);
}

#if defined(TX_CALLBACK)
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
	OS_SYS_POSTSEMAPHORE(ethernetTxDone);
}
#endif

#if defined(ERROR_CALLBACK)
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
	if ((uint32_t)RESET != (heth->Instance->DMASR & ETH_DMASR_TUS))
	{
		heth->Instance->DMASR = ETH_DMASR_TUS;
	    heth->Instance->DMATPDR = 0;
	}

	if ((uint32_t)RESET != (heth->Instance->DMASR & ETH_DMASR_RBUS))
	{
		heth->Instance->DMASR = ETH_DMASR_RBUS;
		heth->Instance->DMARPDR = 0;
	}
}
#endif

#if defined(NETWORK_NOTIFY_INCOMING_LOOPBACK_PACKET)
void Network_NotifyIncomingLoopbackPacket(void)
{
	OS_SYS_POSTSEMAPHORE(ethernetRecvIncoming);
}
#endif

#if defined(CONFIG_ETHERNET_DO_COPY_RECEIVE_BUFFER)
static uint8_t ethernet_recv_buffer[ETH_RX_BUF_SIZE];
#endif

void ethernet_receiver_thread(void *p)
{
#if defined(CONFIG_ETHERNET_DO_COPY_RECEIVE_BUFFER)
	int len;
#endif
	while (1)
	{
		OS_SYS_GETSEMAPHORE(ethernetRecvIncoming, OS_SYS_TIMEOUT_FOREVER);
		if (0 != LoopbackReceiver(OS_SYS_TIMEOUT_NO_WAIT)) // No separate thread fro loopback receiver, just receive it in the Ethernet receiver thread.
		{
			// We shouldn't need a while loop here, but apparently we can loose a receive event when a packet comes in.
			// Perhaps if two packets come in too close to each other we may see only one interrupt???
			// This is just a guess, so far unconfirmed.
#if defined(CONFIG_ETHERNET_DO_COPY_RECEIVE_BUFFER)
			while(1)
			{
				len = Ethernet_ReceivePacket(ethernet_recv_buffer);
				if (0 == len)
				{
					break;
				}
				ethernet_process_incoming(&Ethernet0, ethernet_recv_buffer, len, EthUserXmitBuffer, sizeof(EthUserXmitBuffer));
			}
#else
			while(Ethernet_ReceivePacket(0));
#endif
		}
	}
}
// ---------------------------------------------------------------------------------------------------
#if 0
// This is a copy of the function in stm32f7xx_hal_eth.c because it is declared static there, so it is duplicated here.
// It was also modified to actually enable receiving 'at' this MAC address.
//
void ETH_MACAddressConfig(ETH_HandleTypeDef *heth, uint32_t MacAddr, uint8_t *Addr)
{
  uint32_t tmpreg;

  /* Check the parameters */
  assert_param(IS_ETH_MAC_ADDRESS0123(MacAddr));

  /* Calculate the selected MAC address high register */
  tmpreg = ((uint32_t)Addr[5] << 8) | (uint32_t)Addr[4];
  /* Load the selected MAC address high register */

  // dm00224583-stm32f76xxx-and-stm32f77xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf
  // Page: 1851
  tmpreg |= 0x80000000; // Enable this MAC.

  (*(__IO uint32_t *)((uint32_t)(ETH_MAC_ADDR_HBASE + MacAddr))) = tmpreg;
  /* Calculate the selected MAC address low register */
  tmpreg = ((uint32_t)Addr[3] << 24) | ((uint32_t)Addr[2] << 16) | ((uint32_t)Addr[1] << 8) | Addr[0];

  /* Load the selected MAC address low register */
  (*(__IO uint32_t *)((uint32_t)(ETH_MAC_ADDR_LBASE + MacAddr))) = tmpreg;
}
#endif
// ---------------------------------------------------------------------------------------------------
int Ethernet_IsLinkUp(void)
{
	uint32_t reg;
	HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &reg);
	// PRINTF("Ethernet_ConfigLink: PHY_BSR = 0x%08x\r\n", reg);
	return (0 != (reg & PHY_LINKED_STATUS));
}

int Ethernet_ConfigLink(void)
{
	uint32_t reg;
	uint32_t cnt = 0;

    if (Ethernet_IsLinkUp())
    {
    	HAL_ETH_WritePHYRegister(&heth, PHY_BCR, PHY_AUTONEGOTIATION);

    	do
    	{
    		if (++cnt > 1000)
    		{
    			return -1; // This is a time out.
    		}
    		OS_SYS_DELAY(1);
    		HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &reg);
    	} while (((reg & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));

    	HAL_ETH_ReadPHYRegister(&heth, PHY_SR, &reg);
#if defined(NETWORK_VERBOSE)
		if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
		{
			PRINTF("Ethernet_ConfigLink: PHY_SR = 0x%08x\r\n", reg);
		}
#endif
    	if (0 != (reg & PHY_DUPLEX_STATUS))
    	{
#if defined(NETWORK_VERBOSE)
    		if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
    		{
    			PUTS("ETH: Full Duplex\r\n");
    		}
#endif
    		heth.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    	}
    	else
    	{
#if defined(NETWORK_VERBOSE)
    		if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
    		{
    			PUTS("ETH: Half Duplex\r\n");
    		}
#endif
    	    heth.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
    	}

    	if (0 != (reg & PHY_SPEED_STATUS))
    	{
#if defined(NETWORK_VERBOSE)
    		if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
    		{
    			PUTS("ETH: Speed 10M\r\n");
    		}
#endif
    	    heth.Init.Speed = ETH_SPEED_10M;
    	}
    	else
    	{
#if defined(NETWORK_VERBOSE)
    		if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
    		{
    			PUTS("ETH: Speed 100M\r\n");
    		}
#endif
    		heth.Init.Speed = ETH_SPEED_100M;
    	}

    	if (HAL_OK == HAL_ETH_ConfigMAC(&heth, (ETH_MACInitTypeDef *)0))
    	{
    		if (ETH_MODE_FULLDUPLEX == heth.Init.DuplexMode)
    		{
    			PRINTF("Ethernet MACFCR = 0x%08x\r\n", heth.Instance->MACFCR);
    			//heth.Instance->MACFCR	|= 0x0E;
    			//heth.Instance->MACFCR	= 0x0020000E;
    			heth.Instance->MACFCR = 0x0200000E; // Enable various handshakes.
    			PRINTF("Ethernet MACFCR = 0x%08x\r\n", heth.Instance->MACFCR);
    		}
    		return 0;
    	}

    }
    return -1;
}

void ethernet_periodic(void)
{
	static int saved_linkup = 0;
	static int negotiation_done = 0;
	int linkup = Ethernet_IsLinkUp();

	if (0 == linkup)
	{
		Ethernet0.net_is_link_up = 0;
		Ethernet0.net_is_ready = 0;
#if defined(NETWORK_IPV6_SUPPORTED)
		Ethernet0.net_is_ipv6_ready = 0;
		Ethernet0.net_ipv6_link_local_address_ready = 0;
		Ethernet0.net_ipv6_link_local_address_is_duplicate = 0;
#endif
	}


	if ((0 != linkup) && (0 == saved_linkup))
	{
		PUTS("Ethernet cable was plugged in.\r\n");
		saved_linkup = 1;
		if (0 == Ethernet_ConfigLink())
		{
#if defined(NETWORK_IPV6_SUPPORTED)
			while((!Ethernet0.net_is_ipv6_ready) && (!Ethernet0.net_ipv6_link_local_address_is_duplicate))
			{
				ipv6_periodic(&Ethernet0);
				OS_SYS_DELAY(1);
			}
#endif
#if defined(NETWORK_IPV4_SUPPORTED)
#if defined(USE_STATIC_IP)
			Ethernet0.net_is_ready = 1;
			Ethernet0.net_is_configured = 1;
			Ethernet0.net_is_ready = 1;
#else
#if defined(NETWORK_IPV4_DHCP_SUPPORTED)
			memset(&DHCP_Info_Ethernet0, 0, sizeof(DHCP_Info_Ethernet0));
			Ethernet0.DHCP_Info = &DHCP_Info_Ethernet0;
			dhcp_negotiation(&Ethernet0);
#else
			auto_config_link_local_ipv4(&Ethernet0);
#endif // defined(NETWORK_IPV4_DHCP_SUPPORTED)
#endif // defined(USE_STATIC_IP)
#else /* defined(NETWORK_IPV4_SUPPORTED) */
			PRINTF("net_is_ipv6_ready = %d net_ipv6_address_is_duplicate = %d\r\n", Ethernet0.net_is_ipv6_ready, Ethernet0.net_ipv6_link_local_address_is_duplicate);
			Ethernet0.net_is_ready = Ethernet0.net_is_ipv6_ready;
#endif // defined(NETWORK_IPV4_SUPPORTED)
		}
	}
	else if ((0 == linkup) && (0 != saved_linkup))
	{
		PUTS("Ethernet cable has been pulled out.\r\n");
		negotiation_done = 0;

#if defined(NETWORK_IPV4_SUPPORTED) && defined(NETWORK_DNS_SUPPORT)
		dns_remove_dependents(&Ethernet0, 0);
#endif

#if defined(NETWORK_IPV6_SUPPORTED) && defined(NETWORK_DNS_SUPPORT)
		dns_remove_dependents(&Ethernet0, 1);
#endif

#if defined(NETWORK_IPV6_SUPPORTED)
		ipv6_router_reset(&Ethernet0.net_ipv6_gateway);
#endif

#if defined(NETWORK_MDNS_SUPPORT)
		mdns_network_interface_down(&Ethernet0);
#endif
		saved_linkup = 0;
	}
	else if (0 != linkup)
	{
#if defined(NETWORK_IPV4_SUPPORTED) && defined(NETWORK_IPV4_DHCP_SUPPORTED)
		dhcp_periodic(&Ethernet0);
#endif

#if defined(NETWORK_IPV6_SUPPORTED)
		ipv6_periodic(&Ethernet0);
#endif
		if (NetInterface_NegotiationFinished(&Ethernet0))
		{
			if (0 == negotiation_done)
			{
				negotiation_done = 1;
#if defined(NETWORK_MDNS_SUPPORT)
				if (NetInterface_IsUp(&Ethernet0))
				{
					mdns_network_interface_event(&Ethernet0);
				}
#endif
			}
		}
		else
		{
			negotiation_done = 0;
		}
	}
}

int ethernet_hardware_init(network_interface_t *net)
{
	// MX_ETH_Init();
	HAL_ETH_DMATxDescListInit(&heth, EthernetDMATxDscrTab, EthernetTxBuff[0], ETH_TXBUFNB);
	HAL_ETH_DMARxDescListInit(&heth, EthernetDMARxDscrTab, EthernetRxBuff[0], ETH_RXBUFNB);

#if defined(TX_CALLBACK)
	heth.Instance->DMAIER |= ETH_DMA_IT_T;
#endif

#if defined(NETWORK_IPV6_SUPPORTED) || defined(NETWORK_MDNS_SUPPORT)
	heth.Instance->MACFFR |= ETH_MACFFR_PAM; // Receive all multicasts.
#endif

	HAL_ETH_Start(&heth);
	return 0;
}

network_interface_t *ethernet_init(void)
{
	int i;

	OS_SYS_CREATESEMAPHORE(ethernetRecvIncoming, 0);

#if defined(TX_CALLBACK)
	OS_SYS_CREATESEMAPHORE(ethernetTxDone, 0);
#endif

	OS_SYS_CREATESEMAPHORE(ethernetTransmitterLock, 1);

	MX_ETH_Init();

	for (i = 0; i < ETHERNET_MAC_ADDRESS_LENGTH; i++)
	{
		Ethernet0.net_my_hw_addr.bytes[i] = heth.Init.MACAddr[i];
		//my_mac_address.bytes[i] = heth.Init.MACAddr[i];
	}

	print_mac_address(Ethernet0.net_my_hw_addr.bytes, "My Ethernet");

#if defined(USE_STATIC_IP)
    Ethernet0.net_my_ip_addr = STATIC_IP_IP_ADDRESS;
    Ethernet0.net_netmask = STATIC_IP_NETMASK;
    Ethernet0.net_gateway = STATIC_IP_GATEWAY;
    print_ip4_address(Ethernet0.net_my_ip_addr, "Using static");
#endif

#if defined(NETWORK_VERBOSE)
    if (0 != (network_verbose & NETWORK_VERBOSE_ETHERNET))
    {
    	PRINTF("Ethernet ETH_DMAOMR = 0x%08X\r\n", heth.Instance->DMAOMR);
    }
#endif

    Ethernet0.device = &heth;
    Ethernet0.net_name[0] = 'e';
    Ethernet0.net_name[1] = 't';
    Ethernet0.net_name[2] = 'h';
    Ethernet0.net_name[3] = '0';
    Ethernet0.net_name[4] = '\0';
    Ethernet0.net_hw_header_size = sizeof(ethernet_header_t);

    Ethernet0.net_mtu = NETWORK_DEFAULT_MTU;

    Ethernet0.net_ip4_header_size = sizeof(ip4_header_t); // Just the IP4 header, no options.
    Ethernet0.net_udp_header_size = sizeof(udp_header_t);

#if defined(NETWORK_IPV6_SUPPORTED)
    Ethernet0.net_ip6_header_size = sizeof(ip6_header_t);
#endif

    Ethernet0.net_offload_checksum = 0xFF; // Technically this is a bitmap, we should really set the individual bits.

    Ethernet0.interface_type = NetIF_Ethernet;

#if defined(NETWORK_IPV4_SUPPORTED)
	ipv4_init_interface(&Ethernet0);
#endif

#if defined(NETWORK_IPV6_SUPPORTED)
	ipv6_init_interface(&Ethernet0);
#endif

	return &Ethernet0;
}
#endif // defined(NETWORK_ETHERNET_SUPPORTED)
