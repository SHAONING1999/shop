/**
  ******************************************************************************
  * @file    IAP/IAP_Main/Src/ymodem.c 
  * @author  MCD Application Team
  * @brief   This file provides all the software functions related to the ymodem 
  *          protocol.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/** @addtogroup STM32F4xx_IAP_Main
  * @{
  */ 
  
/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "common.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include "SerialCOM.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* 激活CRC16 */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t flashdestination;
/* @note 注意！ - 请保留这个变量为 32bit 对齐 */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];//接收缓存

/* Private function prototypes -----------------------------------------------*/
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief 从发送方接收一个数据包
  * @param  data
  * @param  length
  *     0: 发送完毕
  *     2: 发送方中止
  *    >0: 包大小
  * @param  timeout
  * @retval HAL_OK: 正常返回
  *         HAL_BUSY: 发送方中止
  */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
  uint32_t crc;
  uint32_t packet_size = 0;
  HAL_StatusTypeDef status;
  uint8_t char1;

  *p_length = 0;
  status = HAL_UART_Receive(&UartHandle, &char1, 1, timeout);
 /* 判断接收的数据头是哪一种包类型 */
  if (status == HAL_OK)
  {
    switch (char1)
    {
		//起始的128字节数据包
      case SOH:
        packet_size = PACKET_SIZE;
        break;
	  //起始的1024字节数据包
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
	  //传输结束
      case EOT:
        break;
	  //其中两个相继夭折
      case CA:
        if ((HAL_UART_Receive(&UartHandle, &char1, 1, timeout) == HAL_OK) && (char1 == CA))
        {
			/* PC主动停止传输 */
          packet_size = 2;
        }
        else
        {
          status = HAL_ERROR;
        }
        break;
		//用户中止"A"
      case ABORT1:
		//用户中止"a"
      case ABORT2:
        status = HAL_BUSY;
        break;
      default:
        status = HAL_ERROR;
        break;
    }
    *p_data = char1;
/* 如果是正常的数据包 */
    if (packet_size >= PACKET_SIZE )
    {
      status = HAL_UART_Receive(&UartHandle, &p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);

      /* 简单的数据包完整性检查 */
      if (status == HAL_OK )
      {
        if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
        {
          packet_size = 0;
          status = HAL_ERROR;
        }
        else
        {
          /* 检查 CRC */
          crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
          crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
          if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
          {
            packet_size = 0;
            status = HAL_ERROR;
          }
        }
      }
      else
      {
        packet_size = 0;
      }
    }
  }
  *p_length = packet_size;
  return status;
}

/**
  * @brief  准备第一个数据包块
  * @param  p_data:  输出缓冲区
  * @param  p_file_name: 要发送的文件的名称
  * @param  length: 要发送的文件长度，以字节为单位
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
  uint32_t i, j = 0;
  uint8_t astring[10];

  /* 前3个字节是常量 */
  p_data[PACKET_START_INDEX] = SOH;
  p_data[PACKET_NUMBER_INDEX] = 0x00;
  p_data[PACKET_CNUMBER_INDEX] = 0xff;

  /* 写文件名 */
  for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
  }

  p_data[i + PACKET_DATA_INDEX] = 0x00;

  /* 写文件大小 */
  Int2Str (astring, length);
  i = i + PACKET_DATA_INDEX + 1;
  while (astring[j] != '\0')
  {
    p_data[i++] = astring[j++];
  }

  /* 用0填充数据位 */
  for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
  {
    p_data[j] = 0;
  }
}

/**
  * @brief  准备数据包
  * @param  p_source: 指向要发送数据的指针
  * @param  p_packet: 指向输出缓冲区的指针
  * @param  pkt_nr: 数据包编号
  * @param  size_blk: 要发送的块的长度，以字节为单位
  * @retval None
  */
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
  uint8_t *p_record;
  uint32_t i, size, packet_size;

  /* 制作前三包 */
  packet_size = size_blk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
  size = size_blk < packet_size ? size_blk : packet_size;
  if (packet_size == PACKET_1K_SIZE)
  {
    p_packet[PACKET_START_INDEX] = STX;
  }
  else
  {
    p_packet[PACKET_START_INDEX] = SOH;
  }
  p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
  p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
  p_record = p_source;

  /* 文件名数据包有效的数据 */
  for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX;i++)
  {
    p_packet[i] = *p_record++;
  }
  if ( size  <= packet_size)
  {
    for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
    {
      p_packet[i] = 0x1A; /* EOF (0x1A) or 0x00 */
    }
  }
}

/**
  * @brief  更新CRC16的输入字节
  * @param  输入值的CRC
  * @param  输入字节
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }
  
  while(!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16用于YModem数据包
  * @param  指向数据
  * @param  长度
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = p_data+size;

  while(p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);
 
  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc&0xffffu;
}

/**
  * @brief  计算YModem数据包的校验和
  * @param  p_data 指向输入数据的指针
  * @param  size  length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end )
  {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  使用带有CRC16的ymodem协议接收文件
  * @param  p_size 文件的大小.
  * @retval COM_StatusTypeDef 接收结果/编制程序
  */
COM_StatusTypeDef Ymodem_Receive ( uint32_t *p_size )
{//              包长度 
  uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
  uint32_t flashdestination;//flash目的地址
  uint32_t ramsource, filesize=0, packets_received;//记录文件大小的变量filesize清零
  uint32_t alreadyfilesize=0;	//新建变量alreadyfilesize用于记录已接收到的数据大小
  uint8_t *file_ptr;//文件指针
  uint8_t file_size[FILE_SIZE_LENGTH], tmp;
  COM_StatusTypeDef result = COM_OK;

  /* APP的起始地址，即从该地址开始写入APP固件，APPLICATION_ADDRESS在flash_if.h文件中定义 */
  flashdestination = APPLICATION_ADDRESS;//
/* 等待文件完成或串口的状态 COM_OK*/
  while ((session_done == 0) && (result == COM_OK))
  {
    packets_received = 0;
    file_done = 0;
	 /* 等待文件完成或串口的状态 */
    while ((file_done == 0) && (result == COM_OK))
    {
		/* ReceivePacket主要接收串口的数据，并验证是否符合Ymodern协议的数据包 */
      switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
      {
        case HAL_OK:
          errors = 0;
          switch (packet_length)
          {
            case 2:
              /* PC传输中断 */
              Serial_PutByte(ACK);//返回应答
              result = COM_ABORT;
              break;
            case 0:
               /* 传输结束 */
              Serial_PutByte(ACK);
              file_done = 1;
              break;
            default:
              /* 判断是否接收正确的数据包数，不成功则回应NAK，此时PC会重发数据包 */
              if (aPacketData[PACKET_NUMBER_INDEX] != (uint8_t)packets_received)
              {
                Serial_PutByte(NAK);
              }
              else
              {
				  /* 第一包数据包和传输完成后传送的结束包，都是以0数据包传送 */
                if ((packets_received == 0)&&(alreadyfilesize >= filesize))
                {
                   /* 第一包数据传输的是文件名和文件大小 */
                  if (aPacketData[PACKET_DATA_INDEX] != 0)
                  {
                    /* 文件名称提取 */
                    i = 0;
                    file_ptr = aPacketData + PACKET_DATA_INDEX;
                    while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
                    {
                      aFileName[i++] = *file_ptr++;
                    }

                    /* 文件大小提取 */
                    aFileName[i++] = '\0';
                    i = 0;
                    file_ptr ++;
                    while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
                    {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';
                    Str2Int(file_size, &filesize);

                    /* 测试要发送的文件的大小 */
                    /* 文件的大小大于Flash大小 */
					/* 验证文件大小是否超过允许的APP固件大小，USER_FLASH_SIZE 在flash_if.h文件中定义 */
					if (filesize > (USER_FLASH_SIZE + 1))// 改为这个
//                    if (*p_size > (USER_FLASH_SIZE + 1))
                    {
                      /* 结束会话 */
                      tmp = CA;
                      HAL_UART_Transmit(&UartHandle, &tmp, 1, NAK_TIMEOUT);
                      HAL_UART_Transmit(&UartHandle, &tmp, 1, NAK_TIMEOUT);
                      result = COM_LIMIT;
                    }
					/* 写入flash前需擦除APP Flash区域 */
                    FLASH_If_Erase(APPLICATION_ADDRESS);
                    *p_size = filesize;

                    Serial_PutByte(ACK);
                    Serial_PutByte(CRC16);
                  }
                  /* 文件名为空，说明是结束包 */
                  else
                  {
                    Serial_PutByte(ACK);
                    file_done = 1;
                    session_done = 1;
                    break;
                  }
                }
                else /* Data packet */
                {
                  ramsource = (uint32_t) & aPacketData[PACKET_DATA_INDEX];
                   /* 写入flash */
                  if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, packet_length/4) == FLASHIF_OK)
                  {
					flashdestination += packet_length;
                    /* 记录已接收数据大小 */
					alreadyfilesize += packet_length;
                    Serial_PutByte(ACK);
                  }
                  else /* 写入闪存时发生错误 */
                  {
                    /* End session */
                    Serial_PutByte(CA);
                    Serial_PutByte(CA);
                    result = COM_DATA;
                  }
                }
                packets_received ++;
                session_begin = 1;
              }
              break;
          }
          break;
        case HAL_BUSY: /* Abort actually */
          Serial_PutByte(CA);
          Serial_PutByte(CA);
          result = COM_ABORT;
          break;
        default:
          if (session_begin > 0)
          {
            errors ++;
          }
          if (errors > MAX_ERRORS)
          {
            /* Abort communication */
            Serial_PutByte(CA);
            Serial_PutByte(CA);
          }
          else
          {
			/* 若函数ReceivePacket超时没有收到数据，则发送‘C’应答，通知PC启动传输 */
            Serial_PutByte(CRC16); /* Ask for a packet */
          }
          break;
      }
    }
  }
  return result;
}

/**
  * @brief  Tymodem协议传输文件
  * @param  p_buf: 首字节的地址
  * @param  p_file_name: Name of the file sent
  * @param  file_size: 发送的文件的名称
  * @retval COM_StatusTypeDef 沟通结果
  */
COM_StatusTypeDef Ymodem_Transmit (uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
  uint32_t errors = 0, ack_recpt = 0, size = 0, pkt_size;
  uint8_t *p_buf_int;
  COM_StatusTypeDef result = COM_OK;
  uint32_t blk_number = 1;
  uint8_t a_rx_ctrl[2];
  uint8_t i;
#ifdef CRC16_F    
  uint32_t temp_crc;
#else /* CRC16_F */   
  uint8_t temp_chksum;
#endif /* CRC16_F */  

  /* 准备第一个 包头 */
  PrepareIntialPacket(aPacketData, p_file_name, file_size);

  while (( !ack_recpt ) && ( result == COM_OK ))
  {
	  /* 发送包 */
    HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);

    /* 基于crc16f发送CRC或校验和 */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
    if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if ((HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }
    if (errors >= MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  p_buf_int = p_buf;
  size = file_size;

  /* 这里使用1024字节长度来发送数据包 */
  while ((size) && (result == COM_OK ))
  {
    /* 准备下一个数据包 */
    PreparePacket(p_buf_int, aPacketData, blk_number, size);
    ack_recpt = 0;
    a_rx_ctrl[0] = 0;
    errors = 0;

    /* 如果是NAK，则重发数据包几次 */
    while (( !ack_recpt ) && ( result == COM_OK ))
    {
		/* 发送 下一个数据包 */
      if (size >= PACKET_1K_SIZE)
      {
        pkt_size = PACKET_1K_SIZE;
      }
      else
      {
        pkt_size = PACKET_SIZE;
      }

      HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE, NAK_TIMEOUT);
      
      /* 基于crc16f发送CRC或校验 */
#ifdef CRC16_F    
      temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_crc >> 8);
      Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
      temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_chksum);
#endif /* CRC16_F */
      
      /* 等待应答 */
      if ((HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == ACK))
      {
        ack_recpt = 1;
        if (size > pkt_size)
        {
          p_buf_int += pkt_size;
          size -= pkt_size;
          if (blk_number == (USER_FLASH_SIZE / PACKET_1K_SIZE))
          {
            result = COM_LIMIT; /* boundary error */
          }
          else
          {
            blk_number++;
          }
        }
        else
        {
          p_buf_int += pkt_size;
          size = 0;
        }
      }
      else
      {
        errors++;
      }

      /* 如果NAK为10，则重发数据包，否则通信结束 */
      if (errors >= MAX_ERRORS)
      {
        result = COM_ERROR;
      }
    }
  }

  /* Sending End Of Transmission char传输字符的发送端 */
  ack_recpt = 0;
  a_rx_ctrl[0] = 0x00;
  errors = 0;
  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    Serial_PutByte(EOT);
    /* Wait for Ack */
    if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if ((HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }

    if (errors >=  MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  /* 空包发送---一些终端模拟器需要这个来关闭会话 */
  if ( result == COM_OK )
  {
    /* 准备一个空包 */
    aPacketData[PACKET_START_INDEX] = SOH;
    aPacketData[PACKET_NUMBER_INDEX] = 0;
    aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
    for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
    {
      aPacketData [i] = 0x00;
    }

    /* 发送空包 */
    HAL_UART_Transmit(&UartHandle, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);

    /* 基于crc16f发送CRC或校验 */
#ifdef CRC16_F    
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */   
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* 等待应答 and 'C' */
    if (HAL_UART_Receive(&UartHandle, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == CA)
      {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
          result = COM_ABORT;
      }
    }
  }

  return result; /* 文件传输成功 */
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
