package com.company;

import com.company.protos.HashTableOperationProto;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.Arrays;

public class ClientConnector {
    private DistributedRemoteStringMap map;
    private int port;
    private DatagramSocket mySock;

    public ClientConnector(DistributedRemoteStringMap map,int port) throws Exception
    {
        this.map=map;
        this.port=port;
        mySock= new DatagramSocket(port);
    }




    private HashTableOperationProto.HashTableOperation   parseOp(HashTableOperationProto.HashTableOperation operation)
    {
        HashTableOperationProto.HashTableOperation outOperation;
        if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.PUT))
        {
            map.put(operation.getKey(),operation.getValue());
            outOperation=HashTableOperationProto.HashTableOperation.newBuilder()
                    .setKey(operation.getKey())
                    .setType(HashTableOperationProto.HashTableOperation.OperationType.ACK)
                    .setValue(0)
                    .build();
        }
        else if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.GET))
        {
            Integer val=map.get(operation.getKey());
            int ival=0;
            HashTableOperationProto.HashTableOperation.OperationType type;
            if (val ==null)
            {
                type=HashTableOperationProto.HashTableOperation.OperationType.NACK;
            }
            else
            {
                type=HashTableOperationProto.HashTableOperation.OperationType.ACK;
                ival=val;
            }
            outOperation=HashTableOperationProto.HashTableOperation.newBuilder()
                    .setKey(operation.getKey())
                    .setType(type)
                    .setValue(ival)
                    .build();
        }
        else if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.CONTAINS))
        {
            boolean bVal=map.containsKey(operation.getKey());
            int val;
            if(bVal)
                val=1;
            else
                val=0;
            outOperation=HashTableOperationProto.HashTableOperation.newBuilder()
                    .setKey(operation.getKey())
                    .setType(HashTableOperationProto.HashTableOperation.OperationType.ACK)
                    .setValue(val)
                    .build();
        }

        else if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.REMOVE))
        {
            Integer val=map.remove(operation.getKey());
            int ival=0;
            HashTableOperationProto.HashTableOperation.OperationType type;
            if (val ==null)
            {
                type=HashTableOperationProto.HashTableOperation.OperationType.NACK;
            }
            else
            {
                type=HashTableOperationProto.HashTableOperation.OperationType.ACK;
                ival=val;
            }
            outOperation=HashTableOperationProto.HashTableOperation.newBuilder()
                    .setKey(operation.getKey())
                    .setType(type)
                    .setValue(ival)
                    .build();
        }
        else
            outOperation=null;

        return outOperation;
    }

    public void listen() throws Exception
    {
        while(true)
        {


            HashTableOperationProto.HashTableOperation operation;


            byte[] buff= new byte[Main.BUFF_SIZE];
            DatagramPacket packet= new DatagramPacket(buff,buff.length);
            mySock.receive(packet);
            byte[] buff2= new byte[packet.getLength()];
            buff2=packet.getData();
            byte[] rcvbuff= Arrays.copyOf(packet.getData(),packet.getLength());
            System.out.println("got packet of length: "+packet.getLength()+" and return array of size:"
                    +packet.getData().length +"rcvbuff is of size:" + rcvbuff.length );
            operation=HashTableOperationProto.HashTableOperation.parseFrom(rcvbuff);
            Main.printOperation(operation);
            HashTableOperationProto.HashTableOperation outoperation=parseOp(operation);
            packet.setData(outoperation.toByteArray());
            mySock.send(packet);
        }
    }

}
