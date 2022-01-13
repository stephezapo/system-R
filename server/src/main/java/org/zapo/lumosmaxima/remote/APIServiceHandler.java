package org.zapo.lumosmaxima.remote;

import core.Core;
import org.apache.thrift.TException;
import org.zapo.lumosmaxima.remote.thrift.API;
import org.zapo.lumosmaxima.remote.thrift.T_FixtureManufacturer;
import org.zapo.lumosmaxima.remote.thrift.T_FixtureModel;
import org.zapo.lumosmaxima.remote.thrift.T_FixtureModels;

import java.util.List;

public class APIServiceHandler implements API.Iface {
    @Override
    public int login(String username, String password) throws TException {
        System.out.println("User '" + username + "' logged in.");
        return 0;
    }

    @Override
    public void logout() throws TException {

    }

    @Override
    public List<T_FixtureManufacturer> getFixtureManufacturers() throws TException {
        return Core.INSTANCE.getFixtureLibrary().getFixtureManufacturers();
    }

    @Override
    public T_FixtureModels getFixtureModels(String manufacturer) throws TException {
        return Core.INSTANCE.getFixtureLibrary().getFixtureModels(manufacturer);
    }
}
