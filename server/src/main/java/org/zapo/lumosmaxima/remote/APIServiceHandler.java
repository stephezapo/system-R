package org.zapo.lumosmaxima.remote;

import org.apache.thrift.TException;
import org.zapo.lumosmaxima.remote.thrift.API;

public class APIServiceHandler implements API.Iface {
    @Override
    public int login(String username, String password) throws TException {
        System.out.println("User '" + username + "' logged in.");
        return 0;
    }

    @Override
    public void logout() throws TException {

    }
}
