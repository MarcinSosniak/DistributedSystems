package com.company;

import java.io.Serializable;
import java.util.Map;

public class DistMapState implements Serializable {
    private boolean fRemote=false;
    private Map<String,Integer> map;

    public boolean fReadyToRecieve()
    {
        return fRemote;
    }

    public boolean booleanfReadyToRecieveClear()
    {
        boolean out=fRemote;
        fRemote=false;
        return out;
    }

    public Map getMap()
    {
        return map;
    }

    public void setMap(Map<String,Integer> map)
    {
        this.map=map;
    }

    public void setRemote()
    {
        fRemote=true;
    }
    public void clearRemote()
    {
        fRemote=false;
    }
}

