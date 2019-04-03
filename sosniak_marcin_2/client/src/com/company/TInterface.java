package com.company;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class TInterface {

    private SimpleStringMap map;
    private BufferedReader reader =
            new BufferedReader(new InputStreamReader(System.in));

    public TInterface(SimpleStringMap map)
    {
        this.map=map;
    }

    public void launchTI ()throws Exception
    {

        while(true)
        {
            System.out.println("instert Command (PUT/CONTAINSKEY/GET/REMOVE)");
            String command=reader.readLine();
            if( !parseCommand( command))
            {
                System.out.println("you failed to issue command, try again");
            }
        }
    }


    private boolean parseCommand(String command) throws Exception
    {
        if(! (command.toUpperCase().equals("PUT") || command.toUpperCase().equals("CONTAINSKEY") || command.toUpperCase().equals("GET") || command.toUpperCase().equals("REMOVE") ))
            return false;
        try {
            System.out.println("Key?)");
            String key = reader.readLine();
            if (command.toUpperCase().equals("PUT")) {
                System.out.println("Value?)");
                String strVal = reader.readLine();
                int val = Integer.parseInt(strVal);
                map.put(key, val);
                System.out.println("Put operation succesfull: key='" + key + "' val=" + val);
            } else if (command.toUpperCase().equals("CONTAINSKEY")) {
                if (map.containsKey(key)) {
                    System.out.println("Contains given key('" + key + "')");
                } else {
                    System.out.println("Doesn't contain given key('" + key + "')");
                }

            } else if (command.toUpperCase().equals("GET")) {
                Integer val = map.get(key);
                if(val==null)
                    System.out.println("Got null back, from key='"+key+"' meaning there was nothing bound to this key");
                else
                    System.out.println("Got Val=" + val + " from key='" + key + "'");

            } else if (command.toUpperCase().equals("REMOVE")) {
                Integer val = map.remove(key);
                if(val==null)
                    System.out.println("Got null back, from key='"+key+"' meaning there was nothing bound to this key adn thus nothing removed");
                else
                    System.out.println("Got Val=" + val + " from key='" + key + "', successfully removed");
            } else
                return false;
            return true;
        }
        catch (RuntimeException ex)
        {
            if(ex.getMessage().equals(DistributedMap.TIMEOUT_MESSAGE))
            {
                System.out.println("Reply timeout occured");
                return true;
            }
            else
                throw ex;
        }

    }

}
