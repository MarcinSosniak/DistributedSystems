package com.company;

import com.company.protos.HashTableOperationProto;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.Arrays;

public class Main {


    public static int BUFF_SIZE=1024;

    public static void printOperation(HashTableOperationProto.HashTableOperation operation)
    {
        System.out.print("Prining operation:\nType="+operation.getType()+"\nintVal="+
                operation.getValue()+"\nkey='"+operation.getKey()+"'\n");
    }


    public static int port=5678;
    public static void main(String[] args) throws Exception {
        System.setProperty("java.net.preferIPv4Stack","true");
        DistributedRemoteStringMap map;
        if(args.length > 0)
        {
            System.out.println("running as secondary thread");
            port=Integer.parseInt(args[0]);
        }
        if(args.length > 1)
        {
            map =new DistributedRemoteStringMap(args[1]);
        }
        else
        {
            map = new DistributedRemoteStringMap();
        }

//        DatagramSocket mySock= new DatagramSocket(5678);
//
//        HashTableOperationProto.HashTableOperation operation;
//
//
//        byte[] buff= new byte[BUFF_SIZE];
//        DatagramPacket packet= new DatagramPacket(buff,buff.  length);
//        mySock.receive(packet);
//        byte[] buff2= new byte[packet.getLength()];
//        buff2=packet.getData();
//        byte[] rcvbuff= Arrays.copyOf(packet.getData(),packet.getLength());
//        System.out.println("got packet of length: "+packet.getLength()+" and return array of size:"
//                +packet.getData().length +"rcvbuff is of size:" + rcvbuff.length );
//        printOperation(HashTableOperationProto.HashTableOperation.parseFrom(rcvbuff));

        ClientConnector connector = new ClientConnector(map,port);
        connector.listen();



    }
}
