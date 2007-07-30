/** @file

Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Mtftp4Support.c

Abstract:

  Support routines for Mtftp


**/

#include "Mtftp4Impl.h"


/**
  Allocate a MTFTP4 block range, then init it to the
  range of [Start, End]

  @param  Start                 The start block number
  @param  End                   The last block number in the range

  @return NULL if failed to allocate memory, otherwise the created block range.

**/
STATIC
MTFTP4_BLOCK_RANGE *
Mtftp4AllocateRange (
  IN UINT16                 Start,
  IN UINT16                 End
  )
{
  MTFTP4_BLOCK_RANGE        *Range;

  Range = NetAllocatePool (sizeof (MTFTP4_BLOCK_RANGE));

  if (Range == NULL) {
    return NULL;
  }

  NetListInit (&Range->Link);
  Range->Start  = Start;
  Range->End    = End;

  return Range;
}


/**
  Initialize the block range for either RRQ or WRQ. RRQ and WRQ have
  different requirements for Start and End. For example, during start
  up, WRQ initializes its whole valid block range to [0, 0xffff]. This
  is bacause the server will send us a ACK0 to inform us to start the
  upload. When the client received ACK0, it will remove 0 from the range,
  get the next block number, which is 1, then upload the BLOCK1. For RRQ
  without option negotiation, the server will directly send us the BLOCK1
  in response to the client's RRQ. When received BLOCK1, the client will
  remove it from the block range and send an ACK. It also works if there
  is option negotiation.

  @param  Head                  The block range head to initialize
  @param  Start                 The Start block number.
  @param  End                   The last block number.

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for initial block range
  @retval EFI_SUCCESS           The initial block range is created.

**/
EFI_STATUS
Mtftp4InitBlockRange (
  IN NET_LIST_ENTRY         *Head,
  IN UINT16                 Start,
  IN UINT16                 End
  )
{
  MTFTP4_BLOCK_RANGE        *Range;

  Range = Mtftp4AllocateRange (Start, End);

  if (Range == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NetListInsertTail (Head, &Range->Link);
  return EFI_SUCCESS;
}


/**
  Get the first valid block number on the range list.

  @param  Head                  The block range head

  @return -1: if the block range is empty. Otherwise the first valid block number.

**/
INTN
Mtftp4GetNextBlockNum (
  IN NET_LIST_ENTRY         *Head
  )
{
  MTFTP4_BLOCK_RANGE  *Range;

  if (NetListIsEmpty (Head)) {
    return -1;
  }

  Range = NET_LIST_HEAD (Head, MTFTP4_BLOCK_RANGE, Link);
  return Range->Start;
}


/**
  Set the last block number of the block range list. It will
  remove all the blocks after the Last. MTFTP initialize the
  block range to the maximum possible range, such as [0, 0xffff]
  for WRQ. When it gets the last block number, it will call
  this function to set the last block number.

  @param  Head                  The block range list
  @param  Last                  The last block number

  @return None

**/
VOID
Mtftp4SetLastBlockNum (
  IN NET_LIST_ENTRY         *Head,
  IN UINT16                 Last
  )
{
  MTFTP4_BLOCK_RANGE        *Range;

  //
  // Iterate from the tail to head to remove the block number
  // after the last.
  //
  while (!NetListIsEmpty (Head)) {
    Range = NET_LIST_TAIL (Head, MTFTP4_BLOCK_RANGE, Link);

    if (Range->Start > Last) {
      NetListRemoveEntry (&Range->Link);
      NetFreePool (Range);
      continue;
    }

    if (Range->End > Last) {
      Range->End = Last;
    }

    return ;
  }
}


/**
  Remove the block number from the block range list.

  @param  Head                  The block range list to remove from
  @param  Num                   The block number to remove

  @retval EFI_NOT_FOUND         The block number isn't in the block range list
  @retval EFI_SUCCESS           The block number has been removed from the list
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource

**/
EFI_STATUS
Mtftp4RemoveBlockNum (
  IN NET_LIST_ENTRY         *Head,
  IN UINT16                 Num
  )
{
  MTFTP4_BLOCK_RANGE        *Range;
  MTFTP4_BLOCK_RANGE        *NewRange;
  NET_LIST_ENTRY            *Entry;

  NET_LIST_FOR_EACH (Entry, Head) {

    //
    // Each block represents a hole [Start, End] in the file,
    // skip to the first range with End >= Num
    //
    Range = NET_LIST_USER_STRUCT (Entry, MTFTP4_BLOCK_RANGE, Link);

    if (Range->End < Num) {
      continue;
    }

    //
    // There are three different cases for Start
    // 1. (Start > Num) && (End >= Num):
    //    because all the holes before this one has the condition of
    //    End < Num, so this block number has been removed.
    //
    // 2. (Start == Num) && (End >= Num):
    //    Need to increase the Start by one, and if End == Num, this
    //    hole has been removed completely, remove it.
    //
    // 3. (Start < Num) && (End >= Num):
    //    if End == Num, only need to decrease the End by one because
    //    we have (Start < Num) && (Num == End), so (Start <= End - 1).
    //    if (End > Num), the hold is splited into two holes, with
    //    [Start, Num - 1] and [Num + 1, End].
    //
    if (Range->Start > Num) {
      return EFI_NOT_FOUND;

    } else if (Range->Start == Num) {
      Range->Start++;

      if (Range->Start > Range->End) {
        NetListRemoveEntry (&Range->Link);
        NetFreePool (Range);
      }

      return EFI_SUCCESS;

    } else {
      if (Range->End == Num) {
        Range->End--;
      } else {
        NewRange = Mtftp4AllocateRange (Num + 1, (UINT16) Range->End);

        if (NewRange == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        Range->End = Num - 1;
        NetListInsertAfter (&Range->Link, &NewRange->Link);
      }

      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}


/**
  Build then transmit the request packet for the MTFTP session.

  @param  Instance              The Mtftp session

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for the request
  @retval EFI_SUCCESS           The request is built and sent
  @retval Others                Failed to transmit the packet.

**/
EFI_STATUS
Mtftp4SendRequest (
  IN MTFTP4_PROTOCOL        *Instance
  )
{
  EFI_MTFTP4_PACKET         *Packet;
  EFI_MTFTP4_OPTION         *Options;
  EFI_MTFTP4_TOKEN          *Token;
  NET_BUF                   *Nbuf;
  UINT8                     *Mode;
  UINT8                     *Cur;
  UINT32                    Len;
  UINTN                     Index;

  Token   = Instance->Token;
  Options = Token->OptionList;
  Mode    = Instance->Token->ModeStr;

  if (Mode == NULL) {
    Mode = "octet";
  }

  //
  // Compute the packet length
  //
  Len = (UINT32) (AsciiStrLen (Token->Filename) + AsciiStrLen (Mode) + 4);

  for (Index = 0; Index < Token->OptionCount; Index++) {
    Len += (UINT32) (AsciiStrLen (Options[Index].OptionStr) +
                     AsciiStrLen (Options[Index].ValueStr) + 2);
  }

  //
  // Allocate a packet then copy the data over
  //
  if ((Nbuf = NetbufAlloc (Len)) == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Packet         = (EFI_MTFTP4_PACKET *) NetbufAllocSpace (Nbuf, Len, FALSE);
  Packet->OpCode = HTONS (Instance->Operation);
  Cur            = Packet->Rrq.Filename;
  Cur            = AsciiStrCpy (Cur, Token->Filename);
  Cur            = AsciiStrCpy (Cur, Mode);

  for (Index = 0; Index < Token->OptionCount; ++Index) {
    Cur = AsciiStrCpy (Cur, Options[Index].OptionStr);
    Cur = AsciiStrCpy (Cur, Options[Index].ValueStr);
  }

  return Mtftp4SendPacket (Instance, Nbuf);
}


/**
  Build then send an error message

  @param  Instance              The MTFTP session
  @param  ErrInfo               The error code and message

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for the error packet
  @retval EFI_SUCCESS           The error packet is transmitted.
  @retval Others                Failed to transmit the packet.

**/
EFI_STATUS
Mtftp4SendError (
  IN MTFTP4_PROTOCOL        *Instance,
  IN UINT16                 ErrCode,
  IN UINT8*                 ErrInfo
  )
{
  NET_BUF                   *Packet;
  EFI_MTFTP4_PACKET         *TftpError;
  UINT32                    Len;

  Len     = (UINT32) (AsciiStrLen (ErrInfo) + sizeof (EFI_MTFTP4_ERROR_HEADER));
  Packet  = NetbufAlloc (Len);

  if (Packet == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  TftpError         = (EFI_MTFTP4_PACKET *) NetbufAllocSpace (Packet, Len, FALSE);
  TftpError->OpCode = HTONS (EFI_MTFTP4_OPCODE_ERROR);
  TftpError->Error.ErrorCode = HTONS (ErrCode);

  AsciiStrCpy (TftpError->Error.ErrorMessage, ErrInfo);

  return Mtftp4SendPacket (Instance, Packet);
}


/**
  The callback function called when the packet is transmitted.
  It simply frees the packet.

  @param  Packet                The transmitted (or failed to) packet
  @param  Points                The local and remote UDP access point
  @param  IoStatus              The result of the transmission
  @param  Context               Opaque parameter to the callback

  @return None

**/
STATIC
VOID
Mtftp4OnPacketSent (
  NET_BUF                   *Packet,
  UDP_POINTS                *Points,
  EFI_STATUS                IoStatus,
  VOID                      *Context
  )
{
  NetbufFree (Packet);
}


/**
  Set the timeout for the instance. User a longer time for
  passive instances.

  @param  Instance              The Mtftp session to set time out

  @return None

**/
VOID
Mtftp4SetTimeout (
  IN MTFTP4_PROTOCOL        *Instance
  )
{
  if (Instance->Master) {
    Instance->PacketToLive = Instance->Timeout;
  } else {
    Instance->PacketToLive = Instance->Timeout * 2;
  }
}


/**
  Send the packet for the instance. It will first save a reference to
  the packet for later retransmission. then determine the destination
  port, listen port for requests, and connected port for others. At last,
  send the packet out.

  @param  Instance              The Mtftp instance
  @param  Packet                The packet to send

  @retval EFI_SUCCESS           The packet is sent out
  @retval Others                Failed to transmit the packet.

**/
EFI_STATUS
Mtftp4SendPacket (
  IN MTFTP4_PROTOCOL        *Instance,
  IN NET_BUF                *Packet
  )
{
  UDP_POINTS                UdpPoint;
  EFI_STATUS                Status;
  UINT16                    OpCode;

  //
  // Save the packet for retransmission
  //
  if (Instance->LastPacket != NULL) {
    NetbufFree (Instance->LastPacket);
  }

  Instance->LastPacket  = Packet;

  Instance->CurRetry    = 0;
  Mtftp4SetTimeout (Instance);

  UdpPoint.LocalAddr    = 0;
  UdpPoint.LocalPort    = 0;
  UdpPoint.RemoteAddr   = Instance->ServerIp;

  //
  // Send the requests to the listening port, other packets
  // to the connected port
  //
  OpCode = NTOHS (*((UINT16 *) NetbufGetByte (Packet, 0, NULL)));

  if ((OpCode == EFI_MTFTP4_OPCODE_RRQ) || (OpCode == EFI_MTFTP4_OPCODE_DIR) ||
      (OpCode == EFI_MTFTP4_OPCODE_WRQ)) {
    UdpPoint.RemotePort = Instance->ListeningPort;
  } else {
    UdpPoint.RemotePort = Instance->ConnectedPort;
  }

  NET_GET_REF (Packet);

  Status = UdpIoSendDatagram (
             Instance->UnicastPort,
             Packet,
             &UdpPoint,
             Instance->Gateway,
             Mtftp4OnPacketSent,
             Instance
             );

  if (EFI_ERROR (Status)) {
    NET_PUT_REF (Packet);
  }

  return Status;
}


/**
  Retransmit the last packet for the instance

  @param  Instance              The Mtftp instance

  @retval EFI_SUCCESS           The last packet is retransmitted.
  @retval Others                Failed to retransmit.

**/
EFI_STATUS
Mtftp4Retransmit (
  IN MTFTP4_PROTOCOL        *Instance
  )
{
  UDP_POINTS                UdpPoint;
  EFI_STATUS                Status;
  UINT16                    OpCode;

  ASSERT (Instance->LastPacket != NULL);

  UdpPoint.LocalAddr  = 0;
  UdpPoint.LocalPort  = 0;
  UdpPoint.RemoteAddr = Instance->ServerIp;

  //
  // Set the requests to the listening port, other packets to the connected port
  //
  OpCode = NTOHS (*(UINT16 *) NetbufGetByte (Instance->LastPacket, 0, NULL));

  if ((OpCode == EFI_MTFTP4_OPCODE_RRQ) || (OpCode == EFI_MTFTP4_OPCODE_DIR) ||
      (OpCode == EFI_MTFTP4_OPCODE_WRQ)) {
    UdpPoint.RemotePort = Instance->ListeningPort;
  } else {
    UdpPoint.RemotePort = Instance->ConnectedPort;
  }

  NET_GET_REF (Instance->LastPacket);

  Status = UdpIoSendDatagram (
             Instance->UnicastPort,
             Instance->LastPacket,
             &UdpPoint,
             Instance->Gateway,
             Mtftp4OnPacketSent,
             Instance
             );

  if (EFI_ERROR (Status)) {
    NET_PUT_REF (Instance->LastPacket);
  }

  return Status;
}


/**
  The timer ticking function for the Mtftp service instance.

  @param  Event                 The ticking event
  @param  Context               The Mtftp service instance

  @return None

**/
VOID
EFIAPI
Mtftp4OnTimerTick (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
{
  MTFTP4_SERVICE            *MtftpSb;
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  MTFTP4_PROTOCOL           *Instance;
  EFI_MTFTP4_TOKEN          *Token;

  MtftpSb = (MTFTP4_SERVICE *) Context;

  //
  // Iterate through all the children of the Mtftp service instance. Time
  // out the packet. If maximum retries reached, clean the session up.
  //
  NET_LIST_FOR_EACH_SAFE (Entry, Next, &MtftpSb->Children) {
    Instance = NET_LIST_USER_STRUCT (Entry, MTFTP4_PROTOCOL, Link);

    if ((Instance->PacketToLive == 0) || (--Instance->PacketToLive > 0)) {
      continue;
    }

    //
    // Call the user's time out handler
    //
    Token = Instance->Token;

    if ((Token->TimeoutCallback != NULL) &&
        EFI_ERROR (Token->TimeoutCallback (&Instance->Mtftp4, Token))) {

      Mtftp4SendError (
         Instance,
         EFI_MTFTP4_ERRORCODE_REQUEST_DENIED,
         "User aborted the transfer in time out"
         );

      Mtftp4CleanOperation (Instance, EFI_ABORTED);
      continue;
    }

    //
    // Retransmit the packet if haven't reach the maxmium retry count,
    // otherwise exit the transfer.
    //
    if (++Instance->CurRetry < Instance->MaxRetry) {
      Mtftp4Retransmit (Instance);
      Mtftp4SetTimeout (Instance);
    } else {
      Mtftp4CleanOperation (Instance, EFI_TIMEOUT);
      continue;
    }
  }
}
