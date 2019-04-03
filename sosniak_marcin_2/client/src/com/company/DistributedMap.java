package com.company;

import com.company.protos.HashTableOperationProto;

import java.io.IOException;
import java.net.*;
import java.util.Arrays;

public class DistributedMap implements SimpleStringMap {

    public static final String TIMEOUT_MESSAGE="Request failed, time out occured";
    private static final int TIME_OUT=2000; // ms

    private InetAddress address;
    private int port;
    private DatagramSocket mySock;
    
    public DistributedMap (int port) throws Exception
    {
        this.port=port;
        address=InetAddress.getLocalHost();
        mySock= new DatagramSocket();
        mySock.setSoTimeout(TIME_OUT);
    }

    public DistributedMap (InetAddress address,int port) throws Exception
    {
        this.port=port;
        this.address=address;
        mySock= new DatagramSocket();
        mySock.setSoTimeout(TIME_OUT);
    }




    private HashTableOperationProto.HashTableOperation sendAndAwait(HashTableOperationProto.HashTableOperation operation)
    {
        try
        {
            DatagramPacket packet= new DatagramPacket(operation.toByteArray(),operation.toByteArray().length, address,port);
            mySock.send(packet);
            DatagramPacket recvPacket=new DatagramPacket(new byte[Main.BUFF_SIZE],Main.BUFF_SIZE);
            mySock.receive(recvPacket);
            byte[] rcvbuff= Arrays.copyOf(recvPacket.getData(),recvPacket.getLength());
            return HashTableOperationProto.HashTableOperation.parseFrom(rcvbuff);
        }
        catch (SocketTimeoutException ex)
        {
            throw new RuntimeException(TIMEOUT_MESSAGE);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new RuntimeException(ex.getMessage(),ex.getCause());
        }
//        DatagramPacket packet= new DatagramPacket(operation.toByteArray(),operation.toByteArray().length, address,port);
//        mySock.send(packet);
//        DatagramPacket recvPacket=new DatagramPacket(new byte[Main.BUFF_SIZE],Main.BUFF_SIZE);
//        mySock.receive(recvPacket);
//        byte[] rcvbuff= Arrays.copyOf(recvPacket.getData(),recvPacket.getLength());
//        return HashTableOperationProto.HashTableOperation.parseFrom(rcvbuff);

    }


    @Override
    public boolean containsKey(String key){
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setKey(key)
                .setType(HashTableOperationProto.HashTableOperation.OperationType.CONTAINS)
                .setValue(0).build();

            operation=sendAndAwait(operation);

        if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.ACK)) {
            if(operation.getValue()!=0)
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }

    }

    @Override
    public Integer get(String key) {
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setKey(key)
                .setType(HashTableOperationProto.HashTableOperation.OperationType.GET)
                .setValue(0).build();

        operation=sendAndAwait(operation);

        if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.ACK)) {
            return operation.getValue();
        }
        else
        {
            return null;
        }
    }

    @Override
    public void put(String key, Integer value) {
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setKey(key)
                .setType(HashTableOperationProto.HashTableOperation.OperationType.PUT)
                .setValue(value).build();

        operation=sendAndAwait(operation);
    }

    @Override
    public Integer remove(String key) {
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setKey(key)
                .setType(HashTableOperationProto.HashTableOperation.OperationType.REMOVE)
                .setValue(0).build();
        operation=sendAndAwait(operation);

        if(operation.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.ACK)) {
            return operation.getValue();
        }
        else
        {
            return null;
        }
    }
}
