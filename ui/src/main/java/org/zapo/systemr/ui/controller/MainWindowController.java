package org.zapo.systemr.ui.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import org.apache.thrift.TException;
import org.zapo.lumosmaxima.remote.thrift.T_FixtureManufacturer;
import org.zapo.systemr.core.Core;

import java.util.List;

public class MainWindowController {

    @FXML
    private void test(ActionEvent event)
    {
        System.out.println("lollolol");
        try {
            List<T_FixtureManufacturer> manufacturers = Core.Get().getAPI().getFixtureManufacturers();

            for (T_FixtureManufacturer man: manufacturers)
            {
                System.out.println(man.getName());
            }
        } catch (TException e) {
            e.printStackTrace();
        }
    }
}
