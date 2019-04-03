package com.company;

import com.company.protos.HashTableOperationProto;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class Main {

    public static int TARGET_PORT=5678;
    public static int BUFF_SIZE=1024;


    // thiis 4567 127.0.0.1
    public static void main(String[] args) throws Exception {

        int port=TARGET_PORT;
        DistributedMap map;
        if(args.length>0)
        {
            port=Integer.parseInt(args[0]);
        }
        if(args.length>1)
        {
            map = new DistributedMap(InetAddress.getByName(args[1]),port);
        }
        else
        {
            map= new DistributedMap(port);
        }


//        DatagramSocket mySock=new DatagramSocket();
//        byte[] buff= new byte[BUFF_SIZE];
//
//        HashTableOperationProto.HashTableOperation operation;
//        operation=HashTableOperationProto.HashTableOperation.newBuilder()
//                .setKey("test1")
//                .setType(HashTableOperationProto.HashTableOperation.OperationType.PUT)
//                .setValue(1).build();
//        buff=operation.toByteArray();
//        System.out.println("the size of out structure is:"+operation.toByteArray().length);
//        DatagramPacket packet= new DatagramPacket(buff,operation.toByteArray().length, InetAddress.getLocalHost(),TARGET_PORT);
//        mySock.send(packet);
        TInterface face= new TInterface(map);
        face.launchTI();


        // write your code here
    }
}
