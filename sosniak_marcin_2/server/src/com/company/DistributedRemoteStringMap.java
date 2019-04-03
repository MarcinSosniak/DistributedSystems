package com.company;

import com.company.protos.HashTableOperationProto;
import org.jgroups.*;
import org.jgroups.protocols.*;
import org.jgroups.protocols.pbcast.*;
import org.jgroups.stack.ProtocolStack;
import org.jgroups.util.Util;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DistributedRemoteStringMap implements SimpleStringMap {

    private static final String DEFAULT_MULTICAST_ADDRESS="230.100.200.199";
    private String multicastAddr;
    JChannel serwerChannel;

    private Map<String,Integer>  myCopy = new HashMap<>();





    private class DistrMapReciever extends ReceiverAdapter
    {

        private class Merger extends Thread
        {

            MergeView view;
            JChannel myChannel;
            public Merger(MergeView view, JChannel channel)
            {
                this.view= view;
                this.myChannel=channel;
            }
            public void run()
            {
                List<View> subgroups=view.getSubgroups();
                View primaryCluster=subgroups.get(0);
                Address myAddr=myChannel.getAddress();
                if(primaryCluster.containsMember(myAddr))
                {
                    System.out.println("Merge occuring, in primary cluster nothis to do");
                }
                else
                {
                    System.out.println("Merge occuring, in non-primary cluster, synchronizing");
                    try {
                        myChannel.getState(null, 5000);
                    }
                    catch (Exception ex)
                    {
                        throw new RuntimeException(ex.getMessage(),ex.getCause());
                    }
                }
            }
        }


        private DistributedRemoteStringMap myDistMap=DistributedRemoteStringMap.this;
        public DistrMapReciever()
        {
            ;
        }



        public void viewAccepted(View new_view) {

            if(new_view instanceof MergeView)
            {
                Merger merg=new Merger((MergeView) new_view,myDistMap.serwerChannel);
                merg.start();
            }
            else {
                System.out.println("** view: " + new_view);
            }
        }


        public void receive(Message msg) {
            if(msg.getSrc().equals(myDistMap.serwerChannel.getAddress()))
                return;
            System.out.println(msg.getSrc() + ": ");
            HashTableOperationProto.HashTableOperation operation=(HashTableOperationProto.HashTableOperation) msg.getObject();
            Main.printOperation( operation );
            myDistMap.executeOp(operation);
        }

        public void getState(OutputStream output) throws Exception {
            synchronized(DistributedRemoteStringMap.this.myCopy) {
                Util.objectToStream(DistributedRemoteStringMap.this.myCopy, new DataOutputStream(output));
            }
        }

        @SuppressWarnings("unchecked")
        public void setState(InputStream input) throws Exception {

            Map<String, Integer> newState = (Map<String, Integer>) Util.objectFromStream(new DataInputStream(input));
            System.out.println("Aquired new State" + newState);
            synchronized (DistributedRemoteStringMap.this.myCopy) {
                DistributedRemoteStringMap.this.myCopy=newState;
            }
        }
    }



    public DistributedRemoteStringMap() throws  Exception
    {
        init(DEFAULT_MULTICAST_ADDRESS);
    }

    public DistributedRemoteStringMap(String multicastAddr) throws  Exception
    {
        init(multicastAddr);
    }

    private void executeOp(HashTableOperationProto.HashTableOperation op)
    {
        synchronized (myCopy) {
            if (op.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.PUT)) {
                myCopy.put(op.getKey(), op.getValue());
                System.out.println("added new corelation {" + op.getKey() + ":" + op.getValue() + "}");
            } else if (op.getType().equals(HashTableOperationProto.HashTableOperation.OperationType.REMOVE)) {
                myCopy.remove(op.getKey());
                System.out.println("removed corelation  from key={" + op.getKey());
            }
        }
    }

    private void init(String multicastAddr) throws Exception
    {

        serwerChannel = new JChannel(false);

        ProtocolStack stack=new ProtocolStack();
        serwerChannel.setProtocolStack(stack);
        stack.addProtocol(new UDP().setValue("mcast_group_addr", InetAddress.getByName(multicastAddr)))
                .addProtocol(new PING())
                .addProtocol(new MERGE3())
                .addProtocol(new FD_SOCK())
                .addProtocol(new FD_ALL().setValue("timeout", 12000).setValue("interval", 3000))
                .addProtocol(new VERIFY_SUSPECT())
                .addProtocol(new BARRIER())
                .addProtocol(new NAKACK2())
                .addProtocol(new UNICAST3())
                .addProtocol(new STABLE())
                .addProtocol(new GMS())
                .addProtocol(new UFC())
                .addProtocol(new MFC())
                .addProtocol(new FRAG2())
                .addProtocol(new STATE())
                .addProtocol(new SEQUENCER())
                .addProtocol(new FLUSH());
        stack.init();
        serwerChannel.setReceiver(new DistrMapReciever());
        serwerChannel.connect("MarcinSCluster",null,10000);

    }

    private void sendWithHalfSuppressedErrors(Message msg)
    {
        try {
            serwerChannel.send(msg);
        }
        catch (Exception ex)
        {
            System.out.println(ex.getStackTrace());
            throw new RuntimeException(ex.getMessage(),ex.getCause());
        }
    }



    @Override
    public boolean containsKey(String key) {
        return myCopy.containsKey(key);
    }

    @Override
    public Integer get(String key) {
        return  myCopy.get(key);
    }

    @Override
    public void put(String key, Integer value) {
        myCopy.put(key,value);
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setType(HashTableOperationProto.HashTableOperation.OperationType.PUT)
                .setKey(key)
                .setValue(value)
                .build();
        Message msg = new Message(null,null,operation);
        sendWithHalfSuppressedErrors(msg);
    }

    @Override
    public Integer remove(String key) {
        myCopy.put(key,0);
        HashTableOperationProto.HashTableOperation operation;
        operation=HashTableOperationProto.HashTableOperation.newBuilder()
                .setType(HashTableOperationProto.HashTableOperation.OperationType.REMOVE)
                .setKey(key)
                .setValue(0)
                .build();
        Message msg = new Message(null,null,operation);
        sendWithHalfSuppressedErrors(msg);
        return myCopy.remove(key);
    }


}

